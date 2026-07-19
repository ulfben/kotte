#pragma once

#include <engines/romuduojr.hpp>
#include <random.hpp>

namespace kotte
{
    /*
    A fast, deterministic random-number generator for game simulation.

    Important: bounded ranges are half-open. The upper bound is excluded.

        Random random{1234}; // Explicit seed gives a reproducible sequence.

        int die = random.between(1, 7);          // 1, 2, 3, 4, 5, or 6
        float unit = random.normalized<float>(); // [0.0f, 1.0f)
        bool hit = random.coin_flip(0.75f);      // 75% chance to be true
        auto& item = random.element(items);      // Reference to a random element
        auto variant = random.next<3, std::uint8_t>(); // 0, 1, or 2, returned as an 8-bit unsigned int.

    Common operations:

        next()                Raw random unsigned integer in [min(), max()) (exclusive)
        next(bound)           Integer in [0, bound)
        next<N, T>()          Integer in [0, N), returned as T
        between(lo, hi)       Integer or float in [lo, hi)
        normalized<F>()       Floating-point value in [0, 1)
        signed_norm<F>()      Floating-point value in [-1, 1)
        coin_flip()           Fair boolean result
        coin_flip(p)          True with probability p
        index(range)          Random valid index
        iterator(range)       Iterator to a random element
        element(range)        Reference to a random element
        gaussian(mean, dev)   Approximate normally distributed value

    The generator also works with standard-library facilities such as
    std::shuffle and std::sample.

    Source, benchmarks and complete documentation:
    https://github.com/ulfben/cpp_prngs/

    This generator is intended for game simulation, not cryptography.
    */
    using Random = rnd::Random<RomuDuoJr>;
}
