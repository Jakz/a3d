#pragma once

#include <cassert>
#include <string>

#define LOGD(x, ...) printf(x "\n", __VA_ARGS__)
#define LOGDD(x) printf(x "\n")

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;

template<typename T>
struct bit_mask
{
  using utype = typename std::underlying_type<T>::type;
  utype value;

  inline void clear() { value = 0; }

  inline bool isSet(T flag) const { return value & static_cast<utype>(flag); }
  inline void set(T flag) { value |= static_cast<utype>(flag); }
  inline void reset(T flag) { value &= ~static_cast<utype>(flag); }
  inline void set(T flag, bool value) { if (value) set(flag); else reset(flag); }

  inline bit_mask<T> operator~() const
  {
    return bit_mask<T>(~value);
  }

  inline bit_mask<T> operator&(T flag) const
  {
    return bit_mask<T>(value & static_cast<utype>(flag));

  }

  inline bit_mask<T> operator|(T flag) const
  {
    return bit_mask<T>(value | static_cast<utype>(flag));
  }

  inline bit_mask<T> operator&(const bit_mask<T>& other) const
  {
    return bit_mask<T>(value & other.value);
  }

  bit_mask<T>() : value(0) { }

private:
  bit_mask<T>(utype value) : value(value) { }
};

using coord_t = int32_t;

struct point_t
{
  coord_t x, y;

  bool operator==(const point_t& o) const
  {
    return x == o.x && y == o.y;
  }

  struct hash
  {
    size_t operator()(const point_t& p) const { return p.y << 16 | p.x; }
  };
};

struct size2d_t
{
  coord_t w, h;
};

struct color_t
{
  u8 r, g, b, a;
};

using path = std::string;

