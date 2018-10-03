
#ifndef LIBMY_HPP
#define LIBMY_HPP

// includes

#include <cstdint>
#include <iostream>
#include <string>

// constants

#undef FALSE
#define FALSE 0

#undef TRUE
#define TRUE 1

#ifdef DEBUG
#  undef DEBUG
#  define DEBUG TRUE
#else
#  define DEBUG FALSE
#endif

#ifdef BMI
#  undef BMI
#  define BMI TRUE
#else
#  define BMI FALSE
#endif

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#  if BMI
#include <immintrin.h>
#pragma intrinsic(_pext_u64)
#pragma intrinsic(_pdep_u64)
#  endif
#endif

// macros

#if DEBUG
#  undef NDEBUG
#else
#  define NDEBUG
#endif

#include <cassert> // needs NDEBUG

// types

typedef std::int8_t  int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

typedef std::uint8_t  uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

// classes

namespace ml {

   // array

   template <class T, int Size> class Array {

   private:

      int p_size;
      T p_item[Size];

      void copy(const Array<T, Size> & array) {

         int size = array.p_size;

         p_size = size;

         for (int pos = 0; pos < size; pos++) {
            p_item[pos] = array.p_item[pos];
         }
      }

   public:

      Array ()                             { clear(); }
      Array (const Array<T, Size> & array) { copy(array); }

      void operator= (const Array<T, Size> & array) { copy(array); }

      void clear   ()               { p_size = 0; }
      void add     (T item)         { assert(!full()); p_item[p_size++] = item; }
      void add_ref (const T & item) { assert(!full()); p_item[p_size++] = item; }

      T    remove   ()         { assert(!empty()); return p_item[--p_size]; }
      void set_size (int size) { assert(size <= Size); p_size = size; }

      bool empty () const { return p_size == 0; }
      bool full  () const { return p_size == Size; }
      int  size  () const { return p_size; }

      const T & operator[] (int pos) const { assert(pos < p_size); return p_item[pos]; }
      T &       operator[] (int pos)       { assert(pos < p_size); return p_item[pos]; } // direct access!
   };
}

// functions

namespace ml {

   // math

   uint64 rand_int_64 ();

   int  round (double x);

   int  div       (int a, int b);
   int  div_round (int a, int b);

   bool is_power_2 (int64 n);
   int  log_2      (int64 n);

   inline uint64 bit       (int n) { return uint64(1) << n; }
   inline uint64 bit_mask  (int n) { return bit(n) - 1; }

   inline void bit_set   (uint64 & b, int n) { b |=  bit(n); }
   inline void bit_clear (uint64 & b, int n) { b &= ~bit(n); }
   inline bool bit_has   (uint64   b, int n) { return (b & bit(n)) != 0; }

#ifdef _MSC_VER

   inline int bit_first (uint64 b) { assert(b != 0); unsigned long i; _BitScanForward64(&i, b); return i; }
   inline int bit_count (uint64 b) { return int(__popcnt64(b)); }
#if BMI
   inline uint64 pext (uint64 a, uint64 b) { return _pext_u64(a, b); }
   inline uint64 pdep (uint64 a, uint64 b) { return _pdep_u64(a, b); }
#endif

#else // assume GCC/Clang

   inline int bit_first (uint64 b) { assert(b != 0); return __builtin_ctzll(b); }
   inline int bit_count (uint64 b) { return __builtin_popcountll(b); }
#if BMI
   inline uint64 pext (uint64 a, uint64 b) { return __builtin_ia32_pext_di(a, b); }
   inline uint64 pdep (uint64 a, uint64 b) { return __builtin_ia32_pdep_di(a, b); }
#endif

#endif
}

#endif // !defined LIBMY_HPP

