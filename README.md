# option-pricing-cpp
A small project that uses Black-Scholes closed-form and Cox-Ross-Rubinstein binomial tree call-option pricing model that was built from scratch in C++. 
We observe a convergence in results of both models with a small discrepancy in numerical terms.

Example:
| S | K | r | $\sigma$ | T |
|:---|:---:|:---:|:---:|---:|
| 100 | 100 | 0.05 | 0.2 | 1 |

First toy-project C++ with implementation of financial models.

## Build and Run

```
g++ option_pricing.cpp -o option_pricing
./option_pricing
```
