
#ifndef HASH_HPP
#define HASH_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

class Pos;

namespace hash {

// functions

void init ();

Key  key       (const Pos & pos);
Key  key_piece (const Pos & pos);
Key  key_pawn  (const Pos & pos);
Key  key_full  (const Pos & pos);

Key  key_turn       ();
Key  key_turn       (Side sd);
Key  key_piece      (Piece pc, Side sd, Square sq);
Key  key_castling   (Side sd, Bit rooks);
Key  key_en_passant (File fl);

int    index (Key key, int mask);
uint32 lock  (Key key);

}

#endif // !defined HASH_HPP

