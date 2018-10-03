
#ifndef VAR_HPP
#define VAR_HPP

// includes

#include <string>

#include "libmy.hpp"

namespace var {

// variables

extern bool Ponder;
extern bool SMP;
extern int  Threads;
extern int  Hash;
extern bool Chess_960;

// functions

void init   ();
void update ();

std::string get (const std::string & name);
void        set (const std::string & name, const std::string & value);

bool get_bool (const std::string & name);
int  get_int  (const std::string & name);

}

#endif // !defined VAR_HPP

