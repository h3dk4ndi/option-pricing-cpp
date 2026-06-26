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

// ---- your normal helpers ----
double norm_pdf(double x) { return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x); }
double norm_cdf(double x) { return 0.5 * std::erfc(-x / std::sqrt(2.0)); }

// ---- your EuropeanOption struct (unchanged) ----
struct EuropeanOption {
    double S, K, r, sigma, T, P;
    int N;
    double d1() { return (std::log(S/K) + (r + 0.5*sigma*sigma)*T) / (sigma*std::sqrt(T)); }
    double d2() { return d1() - sigma*std::sqrt(T); }
    double black_scholes_call() { return S*norm_cdf(d1()) - K*std::exp(-r*T)*norm_cdf(d2()); }
    double black_scholes_put()  { return K*std::exp(-r*T)*norm_cdf(-d2()) - S*norm_cdf(-d1()); }
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
    req.append("securities", "AAPL US Equity");
    req.append("fields", "PX_LAST");          // → S (spot price)
    req.append("fields", "VOLATILITY_90D");   // → sigma  ** VERIFY THIS FIELD NAME IN FLDS<GO> **

    std::cout << "Sending Request: " << req << "\n";
    session.sendRequest(req);

    // variables we'll fill from Bloomberg
    double spot = 0.0;
    double vol = 0.0;
    bool gotSpot = false, gotVol = false;

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
                    std::cerr << "[SECURITY ERROR] " << sec << "\n";
                    continue;
                }

                // report any field problems (e.g. wrong field name)
                if (secData.hasElement("fieldExceptions")) {
                    Element fex = secData.getElement("fieldExceptions");
                    for (size_t j = 0; j < fex.numValues(); ++j) {
                        Element ex = fex.getValueAsElement(j);
                        std::cerr << "[FIELD EXC] " << ex.getElementAsString("fieldId")
                                  << ": " << ex.getElement("errorInfo").getElementAsString("message") << "\n";
                    }
                }

                Element fdata = secData.getElement("fieldData");

                // pull spot
                if (fdata.hasElement("PX_LAST")) {
                    spot = fdata.getElementAsFloat64("PX_LAST");
                    gotSpot = true;
                }
                // pull volatility (VERIFY field name + units!)
                if (fdata.hasElement("VOLATILITY_90D")) {
                    vol = fdata.getElementAsFloat64("VOLATILITY_90D") / 100.0;  // /100 if field is in %
                    gotVol = true;
                }
            }
        }
        if (ev.eventType() == Event::RESPONSE) break;
    }

    // ---- feed the pricer ----
    if (gotSpot && gotVol) {
        EuropeanOption opt;
        opt.S = spot;        // from Bloomberg
        opt.sigma = vol;     // from Bloomberg
        opt.K = spot;        // hardcoded: at-the-money strike = spot (your choice)
        opt.r = 0.05;        // hardcoded risk-free rate
        opt.T = 1.0;         // hardcoded: 1 year
        opt.N = 500;

        std::cout << "\n--- Pulled from Bloomberg ---\n";
        std::cout << "Spot (S):       " << opt.S << "\n";
        std::cout << "Volatility (s): " << opt.sigma << "\n";
        std::cout << "--- Hardcoded ---\n";
        std::cout << "Strike (K):     " << opt.K << " (ATM)\n";
        std::cout << "Rate (r):       " << opt.r << "\n";
        std::cout << "Maturity (T):   " << opt.T << "\n";
        std::cout << "\n--- Pricer output ---\n";
        std::cout << "Call price: " << opt.black_scholes_call() << "\n";
        std::cout << "Put price:  " << opt.black_scholes_put() << "\n";
        std::cout << "Delta:      " << opt.delta_call() << "\n";
        std::cout << "Gamma:      " << opt.gamma() << "\n";
        std::cout << "Vega:       " << opt.vega() << "\n";
    } else {
        std::cerr << "\nDidn't get both spot and vol — check field names in FLDS<GO>.\n";
        if (!gotSpot) std::cerr << "  Missing: PX_LAST\n";
        if (!gotVol)  std::cerr << "  Missing: VOLATILITY_90D (wrong field name?)\n";
    }

    return 0;
}