
#ifndef MATH_HPP
#define MATH_HPP

// includes

#include "libmy.hpp"

namespace math {

// functions

void init ();

template <typename T>
inline T clamp(T x, T min, T max) {

   if (x < min) {
      return min;
   } else if (x > max) {
      return max;
   } else {
      return x;
   }
}

double sqrt  (int n);
double log_2 (int n);

}

#endif // !defined MATH_HPP

