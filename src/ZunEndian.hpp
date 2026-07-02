#pragma once

// Header originally created by Zero318
//   Any bad parts were tacked on by me

#include "inttypes.hpp"
#include <SDL2/SDL_endian.h>
#include <cstring>
#include <type_traits>

static_assert(SDL_BYTEORDER == SDL_LIL_ENDIAN || SDL_BYTEORDER == SDL_BIG_ENDIAN,
              "System endian must be either big or little!");

static_assert(SDL_FLOATWORDORDER == SDL_LIL_ENDIAN || SDL_FLOATWORDORDER == SDL_BIG_ENDIAN,
              "Float endian must be either big or little!");

template <typename T>
using UIForSize = typename std::conditional<
    sizeof(T) == sizeof(u8), u8,
    typename std::conditional<sizeof(T) == sizeof(u16), u16,
                              typename std::conditional<sizeof(T) == sizeof(u32), u32,
                                                        typename std::conditional<sizeof(T) == sizeof(u64), u64,
                                                                                  void>::type>::type>::type>::type;

// GCC-ARM without aligned access and GCC-SuperH both fail to inline a fixed-size unaligned memcpy,
//    giving horrid codegen, but on just about every other platform, memcpy gets inlined and
//    produces equal or better codegen than a manual implementation. So this check exists
//    Is this hyperspecific? Yes, but I spent way too much time looking at godbolt for this
//    header in general and I'll be damned if it's going to have suboptimal codegen
#if (defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)) &&                                        \
    ((defined(__arm__) && !defined(__ARM_FEATURE_UNALIGNED)) || (defined(__sh__)))
#define DO_MANUAL_MEMCPY 1
#else
#define DO_MANUAL_MEMCPY 0
#endif

#if !DO_MANUAL_MEMCPY
template <typename T> static inline constexpr UIForSize<T> read_to_ui_unaligned(void *value)
{
    UIForSize<T> ret;
    std::memcpy(&ret, value, sizeof(T));
    return ret;
}

template <typename T> static inline constexpr void write_from_ui_unaligned(void *dst, const T &src)
{
    std::memcpy(dst, &src, sizeof(T));
}
#else

// Since we have to go byte-by-byte anyway we do the accesses here as little endian.
//   Aside from dodging the byteswap, this is also necessary because if GCC thinks we're
//   doing a memcpy it'll "helpfully" outline it to the libc's memcpy :|

template <typename T> static inline constexpr UIForSize<T> read_to_ui_unaligned(void *value)
{
    UIForSize<T> ret = 0;

    for (i32 i = sizeof(T) - 1; i >= 0; i--)
    {
        ret <<= 8;
        ret |= ((char *)value)[i];
    }

    return ret;
}

template <typename T> static inline constexpr void write_from_ui_unaligned(void *dst, const T &src)
{
    T n = src;

    for (size_t i = 0; i < sizeof(T); i++)
    {
        ((char *)dst)[i] = n & 0xFF;
        n >>= 8;
    }
}
#endif

template <typename T> static inline constexpr UIForSize<T> bit_cast_from_size(const T &a)
{
    UIForSize<T> ret;
    std::memcpy(&ret, &a, sizeof(T));
    return ret;
}

template <typename T> static inline constexpr T bit_cast_to_size(UIForSize<T> value)
{
    T ret;
    std::memcpy(&ret, &value, sizeof(value));
    return ret;
}

static constexpr inline u8 ZunByteswap(u8 in)
{
    return in;
}
static constexpr inline u16 ZunByteswap(u16 in)
{
    return SDL_Swap16(in);
}
static constexpr inline u32 ZunByteswap(u32 in)
{
    return SDL_Swap32(in);
}
static constexpr inline u64 ZunByteswap(u64 in)
{
    return SDL_Swap64(in);
}

template <typename T> struct LE
{
    T raw;

    // Prevent any situation where a struct containing LEs of some type is copied, the compiler assumes
    //   an alignment, and the resulting copy segfaults. Instead we throw a compile-time error
    LE(const LE<T> &a) = delete;

    inline constexpr operator T() const
    {
        UIForSize<T> ui = read_to_ui_unaligned<T>((void *)&raw);

        if constexpr ((std::is_floating_point<T>::value ? SDL_FLOATWORDORDER : SDL_BYTEORDER) == SDL_BIG_ENDIAN &&
                      !DO_MANUAL_MEMCPY)
        {
            ui = ZunByteswap(ui);
        }

        return bit_cast_to_size<T>(ui);
    }

    inline constexpr LE &operator=(const T &a)
    {
        UIForSize<T> ui = bit_cast_from_size<T>(a);

        if constexpr ((std::is_floating_point<T>::value ? SDL_FLOATWORDORDER : SDL_BYTEORDER) == SDL_BIG_ENDIAN &&
                      !DO_MANUAL_MEMCPY)
        {
            ui = ZunByteswap(ui);
        }

        write_from_ui_unaligned((void *)&raw, ui);
        return *this;
    }
};

#undef DO_MANUAL_MEMCPY
