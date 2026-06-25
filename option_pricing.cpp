#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <math.h>
# define M_PI           3.14159265358979323846  /* pi */





// PDF
double norm_pdf(double x) {
    return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x);
}

// CDF
double norm_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}



struct EuropeanOption {
    double S, K, r, sigma, T, P;
    int N;
    
    double d1() {
        return (std::log(S/K) + (r + 0.5*sigma*sigma)*T) / (sigma*std::sqrt(T));
    }

    double d2() {
        return d1() - sigma*std::sqrt(T);
    }

    // Call Option
    // Black-Scholes 
    double black_scholes_call() {
        return S * norm_cdf(d1()) - K * std::exp(-r * T) * norm_cdf(d2());
    }

    // Cox-Ross-Rubinstein 
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

    // Put Option 
    double black_scholes_put() {
        return K * std::exp(-r * T) * norm_cdf(-d2()) - S * norm_cdf(-d1());
    }

    double put_call_parity() {
        double C = black_scholes_call();
        double lhs = C - P;
        double rhs = S - K * std::exp(-r * T);
        double tolerance = 1e-9;   // "close enough" threshold

        if (std::abs(lhs - rhs) < tolerance) {       // ← compare the DIFFERENCE to a tiny number
            std::cout << "Put-Call Parity established." << std::endl;
        } else {
            std::cout << "Put-Call Parity is NOT established." << std::endl;
        }
        std::cout << "C - P = " << lhs << std::endl;
        std::cout << "S - K*exp(-rT) = " << rhs << std::endl;
        return 0;
    }

    double delta_call() {
         return norm_cdf(d1());
    }

    double gamma() {
        return (norm_pdf(d1()) / (S * sigma * std::sqrt(T)));
    }

    double vega() {
        return (S * norm_pdf(d1()) * std::sqrt(T));
    }

    double theta_call() {
        return (- (S * norm_pdf(d1()) * sigma) / (2 * std::sqrt(T)) - r * K * std::exp(-r * T) * norm_cdf(d2()));
        //              ^^^^^^^^ pdf (was cdf)                                                  ^^^^^^^^ cdf (was pdf)
    }

    double rho_call() {
        return (K * T * std::exp(-r * T) * norm_cdf(d2()));
    }
};




int main() {
    EuropeanOption opt;   // create ONE option object

    // read inputs straight into the object's members
    std::cout << "Enter the current stock price (underlying asset): " << std::endl;
    std::cin >> opt.S;

    std::cout << "Enter the strike price: " << std::endl;
    std::cin >> opt.K;

    std::cout << "Enter the risk-free interest rate: " << std::endl;
    std::cin >> opt.r;

    std::cout << "Enter the volatility: " << std::endl;
    std::cin >> opt.sigma;

    std::cout << "Enter the time to maturity: " << std::endl;
    std::cin >> opt.T;

    opt.N = 500;   // set the CRR steps (it's a member now)

    // now call the METHODS on the object — no parameters needed
    double call_price = opt.black_scholes_call();
    std::cout << "Black-Scholes call price: " << call_price << std::endl;

    double crr_price = opt.crr_call();
    std::cout << "CRR binomial call price (N=" << opt.N << "): " << crr_price << std::endl;

    double put_price = opt.black_scholes_put();
    std::cout << "Black-Scholes put price: " << put_price << std::endl;

    opt.P = put_price;          // store the put price IN the object before parity uses it
    opt.put_call_parity();      // now parity can read opt.P

    std::cout << "Delta: " << opt.delta_call() << std::endl;
    std::cout << "Gamma: " << opt.gamma() << std::endl;
    std::cout << "Vega:  " << opt.vega() << std::endl;
    std::cout << "Theta: " << opt.theta_call() << std::endl;
    std::cout << "Rho:   " << opt.rho_call() << std::endl;
    return 0;
}
