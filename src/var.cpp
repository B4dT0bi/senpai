
// includes

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "libmy.hpp"
#include "var.hpp"

namespace var {

// variables

bool Ponder;
bool SMP;
int  Threads;
int  Hash;
bool Chess_960;

static std::map<std::string, std::string> Var;

// functions

void init() {

   set("Ponder", "false");
   set("Threads", "1");
   set("Hash", "64");
   set("UCI_Chess960", "false");

   update();
}

void update() {

   Ponder    = get_bool("Ponder");
   Threads   = get_int("Threads");
   SMP       = Threads > 1;
   Hash      = 1 << ml::log_2(get_int("Hash"));
   Chess_960 = get_bool("UCI_Chess960");
}

std::string get(const std::string & name) {

   if (Var.find(name) == Var.end()) {
      std::cerr << "unknown variable: \"" << name << "\"" << std::endl;
      std::exit(EXIT_FAILURE);
   }

   return Var[name];
}

void set(const std::string & name, const std::string & value) {
   Var[name] = value;
}

bool get_bool(const std::string & name) {

   std::string value = get(name);

   if (value == "true") {
      return true;
   } else if (value == "false") {
      return false;
   } else {
      std::cerr << "not a boolean: variable " << name << " = \"" << value << "\"" << std::endl;
      std::exit(EXIT_FAILURE);
      return false;
   }
}

int get_int(const std::string & name) {
   return std::stoi(get(name));
}

}

