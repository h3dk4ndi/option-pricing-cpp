#include <blpapi_session.h>
#include <blpapi_service.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#define M_PI 3.14159265358979323846

using namespace BloombergLP;
using namespace blpapi;

double norm_pdf(double x) {
    return (1.0 / std::sqrt(2 * M_PI)) * std::exp(-0.5 * x * x);
}

double norm_cdf(double x) {
    return (0.5 * std::erfc(-x / std::sqrt(2.0)));
}

struct EuropeanOption {
    double S, K, r, sigma, T, P;
    int N;
    double d1() { return (std::log(S/K) + (r + 0.5*sigma*sigma)*T) / (sigma*std::sqrt(T)); }
    double d2() { return d1() - sigma*std::sqrt(T); }
    double black_scholes_call() { return S*norm_cdf(d1()) - K*std::exp(-r*T)*norm_cdf(d2()); }
    double black_scholes_put()  { return K*std::exp(-r*T)*norm_cdf(-d2()) - S*norm_cdf(-d1()); }

    double crr_call() {
        double dt = T / N;
        double u = std::exp(sigma * std::sqrt(dt));
        double d = 1.0 / u;
        double p = (std::exp(r * dt) - d) / (u - d);
        double disc = std::exp(-r * dt);       

        std::vector<double> V(N + 1);      
        for (int j = 0; j <= N; j++) {
            double S_j = S * std::pow(u, j) * std::pow(d, N - j);
            V[j] = std::max(S_j - K, 0.0); 
        }

        for (int i = N - 1; i >= 0; i--) {      // backward through layers
            for (int j = 0; j <= i; j++) {      // nodes in layer 
                V[j] = disc * (p * V[j + 1] + (1 - p) * V[j]);

            }
        }
        return V[0];
    }

    double crr_put() {
        double dt = T / N;
        double u = std::exp(sigma * std::sqrt(dt));
        double d = 1.0 / u;
        double p = (std::exp(r * dt) - d) / (u - d);
        double disc = std::exp(-r * dt);       

        std::vector<double> V(N + 1);      
        for (int j = 0; j <= N; j++) {
            double S_j = S * std::pow(u, j) * std::pow(d, N - j);
            V[j] = std::max(K - S_j, 0.0); 
        }

        for (int i = N - 1; i >= 0; i--) {      // backward through layers
            for (int j = 0; j <= i; j++) {      // nodes in layer 
                V[j] = disc * (p * V[j + 1] + (1 - p) * V[j]);

            }
        }
        return V[0];
    }

    void put_call_parity() {
        double C = black_scholes_call();
        double P_val = black_scholes_put();   // compute put here too
        double lhs = C - P_val;
        double rhs = S - K * std::exp(-r * T);
        double tolerance = 1e-9;

        std::cout << "C - P = " << lhs << "\n";
        std::cout << "S - K*exp(-rT) = " << rhs << "\n";
        if (std::abs(lhs - rhs) < tolerance)
            std::cout << "Put-Call Parity established.\n";
        else
            std::cout << "Put-Call Parity is NOT established.\n";
    }


    double delta_call() { return norm_cdf(d1()); }
    double gamma()      { return norm_pdf(d1()) / (S*sigma*std::sqrt(T)); }
    double vega()       { return S*norm_pdf(d1())*std::sqrt(T); }
    double theta_call() { return -(S*norm_pdf(d1())*sigma)/(2*std::sqrt(T)) - r*K*std::exp(-r*T)*norm_cdf(d2()); }
    double rho_call()   { return K*T*std::exp(-r*T)*norm_cdf(d2()); }
};

int main() {
    SessionOptions opts; 
    opts.setServerHost("localhost");
    opts.setServerPort(8194);

    Session session(opts); 
    if (!session.start() || !session.openService("//blp/refdata")) {
        std::cerr << "Failed to start/open //blp/refdata\n";
        return 1;
    }

    Service ref = session.getService("//blp/refdata");
    Request req = ref.createRequest("ReferenceDataRequest");
    req.append("securities", "EZJ LN 12 C640 Equity"); 
    req.append("fields", "OPT_UNDL_PX");                    // Underlying Value -- S
    req.append("fields", "OPT_STRIKE_PX");                  // Strike Price -- K 
    req.append("fields", "IVOL_ASK");                       // Implied Volatility Using Ask Price -- sigma (!!!)
    req.append("fields", "OPT_EXPIRE_DT");                  // Expiration Date -- T
    req.append("fields", "EOD_RISK_FREE_RATE_ASK");         // End of Day Risk Free Rate Ask -- r

    std::cout << "Sending Request: " << req << std::endl;
    session.sendRequest(req);

    while (true) {
        Event ev = session.nextEvent();
        MessageIterator it(ev); 
        while (it.next()) {
            Message msg = it.message(); 
            if (msg.messageType() != Name("ReferenceDataResponse")) continue;

            Element secDataArr = msg.getElement("securityData");
            for (size_t i = 0; i < secDataArr.numValues(); ++i) {
                Element secData = secDataArr.getValueAsElement(i); 
                std::string sec = secData.getElementAsString("security");

                if (secData.hasElement("securityError")) {
                    Element err = secData.getElement("securityError");
                    std::cerr << "[SECURITY ERROR] " << sec << ":" << err.getElementAsString("message") << "\n";
                    continue;
                }

                if (secData.hasElement("fieldExceptions")) {
                    Element fex = secData.getElement("fieldExceptions"); 
                    for (size_t j = 0; j < fex.numValues(); ++j) {
                        Element ex = fex.getValueAsElement(j);
                        std::string fld = ex.getElementAsString("fieldId");
                        std::string reason = ex.getElement("errorInfo").getElementAsString("message");
                        std::cerr << "[FIELD EXC] " << sec << " - " << fld << ": " << reason << "\n";
                    }
                }

                // --- draw the values from fieldData ---
                Element fdata = secData.getElement("fieldData");

                EuropeanOption opt;   // make the option object
                opt.S = 0; opt.K = 0; opt.sigma = 0; opt.r = 0; opt.T = 1; opt.N = 500;
                // pull each field AS A DOUBLE, with a safety check
                if (fdata.hasElement("OPT_UNDL_PX"))   opt.S = fdata.getElementAsFloat64("OPT_UNDL_PX");
                if (fdata.hasElement("OPT_STRIKE_PX")) opt.K = fdata.getElementAsFloat64("OPT_STRIKE_PX");
                if (fdata.hasElement("IVOL_ASK"))      opt.sigma = fdata.getElementAsFloat64("IVOL_ASK") / 100.0;  // /100 if % 
                //if (fdata.hasElement("EOD_RISK_FREE_RATE_ASK")) opt.r = fdata.getElementAsFloat64("EOD_RISK_FREE_RATE_ASK") / 100.0;
                std::cout << "Enter risk-free rate (e.g. 0.05): ";
                std::cin >> opt.r;
                std::cout << "Enter time to maturity in years (e.g. 0.5): ";
                std::cin >> opt.T;

                // --- price it ---
                std::cout << "\n--- " << sec << " ---\n";
                std::cout << "S: " << opt.S << "  K: " << opt.K 
                          << "  sigma: " << opt.sigma << "  r: " << opt.r << "\n";
                std::cout << "Call (Black-Scholes): " << opt.black_scholes_call() << "\n";
                std::cout << "Put (Black-Scholes):  " << opt.black_scholes_put() << "\n";
                std::cout << "Call (Cox-Ross-Rubinstein): " << opt.crr_call() << "\n";
                std::cout << "Put (Cox-Ross-Rubinstein):  " << opt.crr_put() << "\n";

                std::cout << "Delta (Call): " << opt.delta_call() << "\n";
                std::cout << "Gamma: " << opt.gamma() << "\n";
                std::cout << "Vega: " << opt.vega() << "\n";
                std::cout << "Theta (Call): " << opt.theta_call() << "\n";
                std::cout << "Rho (Call): " << opt.rho_call() << "\n";

                opt.put_call_parity(); 
            }
        }
        if (ev.eventType() == Event::RESPONSE) break;
    }
    return 0; 
}