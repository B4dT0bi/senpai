
// includes

#include <cmath>

#include "libmy.hpp"
#include "math.hpp"

namespace math {

// constants

const int Table_Size = 256;

// variables

static float Sqrt [Table_Size];
static float Log_2[Table_Size];

// functions

void init() {

   for (int i = 0; i < Table_Size; i++) {

      double x = double(i);

      Sqrt[i]  = std::sqrt(x);
      Log_2[i] = std::log2(x);
   }
}

double sqrt(int n) {
   assert(n >= 0 && n < Table_Size);
   return Sqrt[n];
}

double log_2(int n) {
   assert(n > 0 && n < Table_Size);
   return Log_2[n];
}

}

