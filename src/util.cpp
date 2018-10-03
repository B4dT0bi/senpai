
// includes

#include <string>

#include "libmy.hpp"
#include "util.hpp"

// functions

int find(char c, const std::string & s) {

   auto i = s.find(c);
   if (i == std::string::npos) throw Bad_Input();

   return int(i);
}

