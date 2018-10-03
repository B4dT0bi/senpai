
// libmy.cpp

// includes

#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "libmy.hpp"

namespace ml {

// functions

// math

uint64 rand_int_64() {
   static std::mt19937_64 gen;
   return gen();
}

int round(double x) {
   return int(floor(x + 0.5));
}

int div(int a, int b) {

   assert(b > 0);

   if (b <= 0) {
      std::cerr << "ml::div(): divide error" << std::endl;
      std::exit(EXIT_FAILURE);
   }

   int div = a / b;
   if (a < 0 && a != b * div) div--; // fix buggy C semantics

   return div;
}

int div_round(int a, int b) {
   assert(b > 0);
   return div(a + b / 2, b);
}

bool is_power_2(int64 n) {
   assert(n >= 0);
   return (n & (n - 1)) == 0 && n != 0;
}

int log_2(int64 n) {

   assert(n > 0);

   int ln = -1;

   for (; n != 0; n >>= 1) {
      ln++;
   }

   assert(ln >= 0);
   return ln;
}

}

