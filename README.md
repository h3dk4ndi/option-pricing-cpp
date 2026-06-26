# option-pricing-cpp

A from-scratch C++ implementation of European option pricing using the
Black-Scholes closed-form solution and the Cox-Ross-Rubinstein (CRR) binomial
tree model. Built as a first C++ project implementing financial models.

## Features

- Black-Scholes pricing for European calls and puts
- Cox-Ross-Rubinstein binomial tree pricing for calls and puts
- Put-call parity validation
- The five option Greeks: Delta, Gamma, Vega, Theta, Rho
- Convergence of CRR to Black-Scholes as the number of steps increases

The two pricing methods agree to within a small numerical discrepancy,
demonstrating the convergence of the binomial tree to the closed-form solution.

## Example

For the parameters below:

| S | K | r | $\sigma$ | T |
|:---:|:---:|:---:|:---:|:---:|
| 100 | 100 | 0.05 | 0.2 | 1 |

Black-Scholes call ≈ 10.4506, CRR call ≈ 10.4466 (N = 500).

## Build and Run

```
g++ option_pricing.cpp -o option_pricing
./option_pricing
```
