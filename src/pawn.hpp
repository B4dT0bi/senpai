
#ifndef PAWN_HPP
#define PAWN_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

class Pos;

namespace pawn {

// functions

void init ();

Bit  weak    (const Pos & pos, Side sd);
Bit  strong  (const Pos & pos, Side sd);
Bit  blocked (const Pos & pos, Side sd);

bool is_passed    (const Pos & pos, Square sq, Side sd);
bool is_duo       (const Pos & pos, Square sq, Side sd);
bool is_protected (const Pos & pos, Square sq, Side sd);
bool is_ram       (const Pos & pos, Square sq, Side sd);

bool is_open   (const Pos & pos, Square sq, Side sd);
bool is_strong (const Pos & pos, Square sq, Side sd);

Bit  file   (Square sq);
Bit  rank   (Square sq);

Bit  fronts (Square sq, Side sd);
Bit  rears  (Square sq, Side sd);

}

#endif // !defined PAWN_HPP

