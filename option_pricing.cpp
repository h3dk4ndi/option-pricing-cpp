#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>


double norm_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double black_scholes_call(double S, double K, double r, double sigma, double T) {
    double d1 = (std::log(S/K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
}

double crr_call(double S, double K, double r, double sigma, double T, int N) {
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

int main() {
    double und_asset;
    double strike_price;
    double risk_free_int_rate;
    double time_to_maturity;
    double volatility; 
    int N = 500;

    std::cout << "Enter the current stock price (underlying asset): " << std::endl;
    std::cin >> und_asset;

    std::cout << "Enter the strike price: " << std::endl;
    std::cin >> strike_price;

    std::cout << "Enter the risk-free interest rate: " << std::endl;
    std::cin >> risk_free_int_rate;

    std::cout << "Enter the volatility: " << std::endl;
    std::cin >> volatility;

    std::cout << "Enter the time to maturity: " << std::endl; 
    std::cin >> time_to_maturity;

    double call_price = black_scholes_call(und_asset, strike_price, risk_free_int_rate, volatility, time_to_maturity);

    std::cout << "Black-Scholes call price: " << call_price << std::endl;

    double crr_price = crr_call(und_asset, strike_price, risk_free_int_rate, volatility, time_to_maturity, N);

    std::cout << "CRR binomial call price (N=" << N << "): " << crr_price << std::endl;

    return 0;
}

