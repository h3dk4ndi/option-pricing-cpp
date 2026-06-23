# TODO

Roadmap for improving this C++ option-pricing project from a single-file implementation into a small, testable numerical-finance library.

## 1. Project structure

* [ ] Split the current single-file implementation into a standard C++ project layout:

  * [ ] `include/`
  * [ ] `src/`
  * [ ] `tests/`
  * [ ] `examples/`
  * [ ] `docs/`
* [ ] Add a `CMakeLists.txt` build system.
* [ ] Move the interactive `main()` logic into `src/main.cpp`.
* [ ] Keep pricing logic separate from input/output logic.
* [ ] Add clear namespaces, e.g. `pricing::black_scholes`, `pricing::binomial`.

## 2. Black-Scholes model

* [ ] Implement European call pricing.
* [ ] Implement European put pricing.
* [ ] Add put-call parity validation:

$C - P = S_0 - K e^{-rT}$


* [ ] Add input validation for:

  * [ ] non-positive spot price
  * [ ] non-positive strike
  * [ ] negative volatility
  * [ ] negative time to maturity
  * [ ] invalid interest-rate assumptions

## 3. Greeks

* [ ] Implement Delta.
* [ ] Implement Gamma.
* [ ] Implement Vega.
* [ ] Implement Theta.
* [ ] Implement Rho.
* [ ] Add tests comparing analytical Greeks against finite-difference approximations.

## 4. Cox-Ross-Rubinstein binomial model

* [ ] Implement European call pricing.
* [ ] Implement European put pricing.
* [ ] Add configurable number of time steps.
* [ ] Validate risk-neutral probability:

$p = \frac{e^{r\Delta t} - d}{u - d}$

* [ ] Add checks for invalid CRR configurations where (p \notin [0,1]).
* [ ] Add convergence tests showing that CRR prices approach Black-Scholes prices as the number of steps increases.

## 5. Tests

* [ ] Add unit tests for Black-Scholes call and put prices.
* [ ] Add unit tests for put-call parity.
* [ ] Add unit tests for CRR convergence.
* [ ] Add unit tests for Greeks.
* [ ] Add edge-case tests:

  * [ ] zero time to maturity
  * [ ] very low volatility
  * [ ] deep in-the-money options
  * [ ] deep out-of-the-money options
  * [ ] high interest-rate scenarios

## 6. Command-line interface

* [ ] Replace interactive-only input with command-line arguments.
* [ ] Support model selection from the command line:

```bash
./option_pricer --model black-scholes --type call --spot 100 --strike 100 --rate 0.05 --vol 0.2 --maturity 1
```

* [ ] Add optional output formatting:

  * [ ] plain text
  * [ ] CSV-style row
  * [ ] JSON-style output

## 7. Documentation

* [ ] Improve `README.md` with:

  * [ ] project overview
  * [ ] build instructions
  * [ ] usage examples
  * [ ] model formulas
  * [ ] sample outputs
* [ ] Add `docs/formulas.md` explaining:

  * [ ] Black-Scholes formula
  * [ ] CRR tree formula
  * [ ] Greeks
  * [ ] put-call parity
* [ ] Add `docs/convergence.md` showing CRR convergence to Black-Scholes.
* [ ] Add a short note on numerical assumptions and limitations.

## 8. Benchmarking

* [ ] Add simple runtime benchmarks for:

  * [ ] Black-Scholes pricing
  * [ ] CRR pricing with different numbers of steps
  * [ ] Greeks calculation
* [ ] Compare CRR runtime as step count increases.
* [ ] Document time-complexity assumptions.

## 9. Continuous integration

* [ ] Add GitHub Actions workflow.
* [ ] Run build checks automatically on push.
* [ ] Run unit tests automatically on push.
* [ ] Add compiler warnings:

```bash
-Wall -Wextra -Wpedantic
```

* [ ] Treat warnings seriously and keep the code warning-clean.

## 10. Future extensions

* [ ] Add Monte Carlo pricing for European options.
* [ ] Add antithetic variates for variance reduction.
* [ ] Add Asian option pricing.
* [ ] Add implied volatility solver using:

  * [ ] bisection
  * [ ] Newton-Raphson
* [ ] Add dividend yield support.
* [ ] Add American option pricing using binomial trees.
* [ ] Add basic volatility-surface interpolation.
* [ ] Add Python comparison scripts for validation.

## Current development priority

The immediate priority is to make the project structurally professional before adding advanced models:

1. Add CMake.
2. Split the code into headers and source files.
3. Add put pricing.
4. Add Greeks.
5. Add tests.
6. Add CRR convergence validation.
7. Improve README documentation.

Advanced models should only be added after the basic implementation is tested and reproducible.
