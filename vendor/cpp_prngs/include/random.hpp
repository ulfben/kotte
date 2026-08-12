#pragma once
#include "concepts.hpp" //for RandomBitEngine concept
#include "detail.hpp"   //for constexpr and portable 128-bit multiplication
#include <bit> // for std::bit_cast
#include <cassert>
#include <concepts>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ranges>
#include <type_traits>

// This is an RNG interface that wraps around any engine that meets the RandomBitEngine concept.
// It provides useful functions for generating values, including integers, floating-point numbers, and colors
// as well as methods for Gaussian distribution, coin flips (with odds), picking from collections (index or element), etc.
// 
// Source: https://github.com/ulfben/cpp_prngs/
// Demo is available on Compiler Explorer: https://compiler-explorer.com/z/PrjqfrP5z
// Benchmarks: https://github.com/ulfben/cpp_prngs/#performance-benchmarks

namespace rnd {
    template <RandomBitEngine E>
    class Random final{
    public:
        using engine_type = E;
        using result_type = typename E::result_type;
        using seed_type = typename E::seed_type;
        static_assert(std::is_unsigned_v<result_type>);
        static_assert(E::min() == 0);
        static_assert(E::max() == std::numeric_limits<result_type>::max());

        constexpr Random() noexcept = default; //the engine will default initialize
        explicit constexpr Random(engine_type engine) noexcept : _engine(engine){}
        explicit constexpr Random(seed_type seed_val) noexcept : _engine(seed_val){};

        constexpr bool operator==(const Random& rhs) const noexcept = default;

        //access to the underlying engine for manual serialization, etc.
        constexpr const E& engine() const noexcept{ return _engine; }
        constexpr E& engine() noexcept{ return _engine; }

        constexpr void seed() noexcept{ _engine.seed(); } //restore default constructed seed
        constexpr void seed(seed_type v) noexcept{ _engine.seed(v); }

        //advance the random engine n steps.
        //some engines (like PCG32) can do this faster than linear time
        constexpr void discard(result_type n) noexcept{ _engine.discard(n); }

        static constexpr auto min() noexcept{ return 0; }
        static constexpr auto max() noexcept{ return E::max(); }

        // --- raw values / bits ---

        // Produce one raw engine value in [min(), max()], inclusive.
        constexpr result_type next() noexcept{ return _engine(); }
        constexpr result_type operator()() noexcept{ return next(); }                

        // Runtime bit extraction. Returns n random bits in the low end of T.
        // We take high bits from the engine because those are the bits most
        // small PRNGs are designed to make strongest. If T is wider than one engine
        // result, gather_high_bits() stitches together as many draws as are needed.
        template <class T = result_type>
        constexpr T bits(unsigned n) noexcept{
            static_assert(std::is_unsigned_v<T>, "bits<T>(n) requires an unsigned T");
            assert(n > 0);
            assert(n <= std::numeric_limits<T>::digits);
            if(n <= value_bits){
                return take_high_bits<T>(next(), n);
            }
            return gather_high_bits<T>(n);
        }

        // Bit extraction with a compile-time bit count.
        // Returns N random bits in the low end of T, with N known at compile time.
        // This can be more efficient than bits(unsigned), because the compiler can
        // specialize the code for the exact bit count and eliminate unused branches.
        template <unsigned N, class T = result_type>
        constexpr T bits() noexcept{
            static_assert(N > 0, "Need at least 1 bit");
            static_assert(std::is_unsigned_v<T>, "bits<N,T> requires an unsigned T");
            static_assert(N <= std::numeric_limits<T>::digits, "T cannot hold N bits");

            if constexpr(N <= value_bits){
                return take_high_bits<T>(next(), N);
            } else{
                // Still centralized: reuse runtime gather (the loop count is deterministic anyway).
                return gather_high_bits<T>(N);
            }
        }

        // Convenience: fill T with random bits.
        template <class T>
        constexpr T bits_as() noexcept{
            static_assert(std::is_unsigned_v<T>, "bits_as<T>() requires an unsigned T");
            return bits<std::numeric_limits<T>::digits, T>();
        }

        // Derive a child generator by consuming enough parent output to fill one seed.
        // This is handy when you need multiple generators for different purposes, or running in different threads.
        [[nodiscard]] constexpr Random split() noexcept{
            return Random{bits_as<seed_type>()};
        }


        // --- integers ---

        // Produce [0, bound) using multiply-high range reduction (often called
        // Lemire's FastRange). It is much faster than a division-based reduction,
        // perfectly unbiased for power-of-two bounds, and has only tiny bias otherwise.
        // See: https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
        constexpr result_type next(result_type bound) noexcept{
            assert(bound > 0 && "bound must be non-zero and positive");
            result_type raw_value = next(); // raw_value is [0, 2^value_bits - 1] (i.e. min()..max(), inclusive)
            if constexpr(value_bits <= 32){ // for small engines, multiply into a 64-bit product
                auto product = std::uint64_t(raw_value) * std::uint64_t(bound);	// product < bound * 2^value_bits   (since raw_value < 2^value_bits)
                auto result = result_type(product >> value_bits); // equivalent to floor(product / 2^value_bits)
                return result;                    // result is now in range [0, bound)
            } else if constexpr(value_bits <= 64){
                // same logic, but use helper for 128-bit math, since __uint128_t isn't universally available
                return detail::mul_shift_u64<value_bits>(raw_value, bound);
            } else{ // fallback for hypothetical >64-bit engines. Naive modulo (slower, more bias)
                return bound > 0 ? raw_value % bound : bound; // avoid division by zero in release builds
            }
        }
        
        constexpr result_type operator()(result_type bound) noexcept{ return next(bound); }

        // Bounded generation with a bound known at compile time and an optional result type.
        // This lets the compiler specialize for Bound: 1 needs no random draw, powers of two
        // can use exact bit extraction, and other constant bounds can be optimized aggressively.
        template <result_type Bound, std::integral T = result_type>
        constexpr T next() noexcept{
            static_assert(Bound > 0, "Bound must be positive");
            static_assert(Bound - 1 <= static_cast<result_type>(std::numeric_limits<T>::max()), "Bound is too large for return type T");

            if constexpr(Bound == 1){
                return T{0}; // The only possible result is 0, so no random draw is needed.
            } else if constexpr((Bound & (Bound - 1)) == 0){ // if Bound is a power of two, we can use a mask / bit-extract.
                constexpr unsigned bits_needed = std::countr_zero(Bound);
                static_assert(bits_needed <= value_bits, "Bound is too large for this engine's result_type");
                return bits<bits_needed, T>();
            } else{ // Otherwise just call the runtime version.
                return static_cast<T>(next(Bound)); // Bound is a compile-time constant here, so the compiler can constant-fold the multiply/shift.
            }
        }

        // integer in [lo, hi)
        template <std::integral I>
        constexpr I between(I lo, I hi) noexcept{
            if(!(lo < hi)){
                assert(false && "between(lo, hi): inverted or empty range");
                return lo;
            }
            using U = std::make_unsigned_t<I>;
            U bound = static_cast<U>(hi) - static_cast<U>(lo);
            assert(uint64_t{bound} <= uint64_t{(max) ()} && "Random::between(lo, hi): range is too large for this engine.");
            auto safe_bound = static_cast<U>(static_cast<result_type>(bound));
            return lo + static_cast<I>(next(safe_bound));
        }

        // --- floating point ---

        // real in [0.0,1.0) using the "IQ float hack"
        //   see Iñigo Quilez, "sfrand": https://iquilezles.org/articles/sfrand/
        // Fast, branchless and, now, portable.
        template <std::floating_point F = float>
        constexpr F normalized() noexcept{
            static_assert(std::numeric_limits<F>::is_iec559, "normalized() requires IEEE 754 (IEC 559) floating point types.");
            using UInt = std::conditional_t<sizeof(F) == 4, uint32_t, uint64_t>; // Pick wide enough unsigned int type for F
            constexpr int mantissa_bits = std::numeric_limits<F>::digits - 1; // Number of mantissa bits for F (e.g., 23 for float)
            static_assert(mantissa_bits <= value_bits,
                "This engine cannot generate enough mantissa bits for this floating-point type. "
                "Use a 64-bit engine or request a 32-bit float.");
            constexpr UInt base = std::bit_cast<UInt>(F(1.0)); // Bit pattern for F(1.0), i.e., exponent set, mantissa 0
            UInt mantissa = this->template bits<mantissa_bits, UInt>();      // Get random bits to fill the mantissa field
            UInt as_int = base | mantissa; // Combine base (1.0) with random mantissa bits
            return std::bit_cast<F>(as_int) - F(1.0); // Convert bits to float/double, then subtract 1.0 to get [0,1)
        }

        // real in [-1.0,1.0) using the IQ float hack.
        template <std::floating_point F = float>
        constexpr F signed_norm() noexcept{
            return F(2) * normalized<F>() - F(1); // scale to [0.0, 2.0), then shift to [-1.0, 1.0)
        }

        // real in [lo, hi)
        template <std::floating_point F = float> 
        constexpr F between(F lo, F hi) noexcept{
            return lo + (hi - lo) * normalized<F>();
        }

        // --- probability/distributions ---

        // A fair coin from the high bit of one engine result.
        constexpr bool coin_flip() noexcept{
            return bits<1, unsigned>() != 0;
        }

        // A weighted coin: true with probability in [0, 1].
        template <std::floating_point F = float>
        constexpr bool coin_flip(F probability) noexcept{
            assert(F{0} <= probability && probability <= F{1} && "Random::coin_flip(probability): probability must be in [0, 1].");
            return normalized<F>() < probability;
        }

        // This is the pleasantly simple Irwin-Hall approximation to a normal
        // distribution. The sum of twelve U(0,1) samples has mean 6 and variance
        // 1, so subtracting 6 and applying mean/stddev gives an approximate normal.
        // See: https://en.wikipedia.org/wiki/Irwin-Hall_distribution
        template <std::floating_point F = float>
        constexpr F gaussian(F mean, F stddev) noexcept{    
            assert(stddev >= F{0} && "Random::gaussian(mean, stddev): standard deviation must be non-negative.");
            F sum{};
            for(auto i = 0; i < 12; ++i){
                sum += normalized<F>();
            }
            return mean + (sum - F(6)) * stddev;
        }

        // --- collections ---
        
        // pick an index in [0, size)
        template <std::ranges::sized_range R>
        constexpr auto index(const R& collection) noexcept{
            assert(!std::ranges::empty(collection) && "Random::index(): empty collection.");
            using idx_t = std::ranges::range_size_t<R>;
            return static_cast<idx_t>(
                between<idx_t>(0, static_cast<idx_t>(std::ranges::size(collection))));
        }

        // get an iterator to a random element. Accepts const and non-const ranges
        template <std::ranges::forward_range R>
            requires std::ranges::sized_range<R>
        constexpr auto iterator(R&& collection) noexcept{
            assert(!std::ranges::empty(collection) && "Random::iterator(): empty collection");
            auto idx = index(collection);             // index accepts const&
            auto it = std::ranges::begin(collection); // picks begin or cbegin for us
            std::advance(it, idx);
            return it;
        }

        //return a reference to a random element in a collection
        //accepts both const and non-const ranges
        template <std::ranges::forward_range R>
            requires std::ranges::sized_range<R>
        constexpr auto element(R&& collection) noexcept{
            return *iterator(std::forward<R>(collection));
        }  
        
    private:
        static constexpr unsigned value_bits = std::numeric_limits<typename E::result_type>::digits;
        E _engine{}; //the underlying engine providing random bits. This class will turn those into useful values.

        template <class T>
        static constexpr T low_bits_mask(unsigned n) noexcept{
            assert(n <= std::numeric_limits<T>::digits); // n in [0, digits(T)]
            constexpr unsigned W = std::numeric_limits<T>::digits;
            if(n == 0) return T{0};
            if(n >= W) return std::numeric_limits<T>::max(); // avoid UB on (1<<W)
            return static_cast<T>((T{1} << n) - T{1});
        }

        template <class T>
        constexpr T take_high_bits(E::result_type x, unsigned n) noexcept{
            assert(n > 0 && n <= value_bits && n <= std::numeric_limits<T>::digits);// Preconditions: 1 <= n <= value_bits, and n <= digits(T)            
            const unsigned shift = value_bits - n;    // shift in [0, value_bits-1]
            return static_cast<T>(x >> shift) & low_bits_mask<T>(n);
        }

        template <class T>
        constexpr T gather_high_bits(unsigned n) noexcept{
            assert(1 <= n && n <= std::numeric_limits<T>::digits); // Preconditions: 1 <= n <= digits(T)			
            T result = 0;
            unsigned filled = 0;
            while(filled < n){
                const unsigned take = std::min<unsigned>(value_bits, n - filled);
                const T chunk = take_high_bits<T>(next(), take);
                result |= static_cast<T>(chunk << filled);             // filled < digits(T) always holds here
                filled += take;
            }
            // If n == digits(T), low_bits_mask returns all-ones, so this is cheap and safe.
            return static_cast<T>(result & low_bits_mask<T>(n));
        }
    };
} //namespace rnd
