
#ifndef EVAL_HPP
#define EVAL_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

class Pos;

// functions

void clear_pawn_table ();

Score eval (const Pos & pos, Side sd);

Score piece_mat (Piece pc);

#endif // !defined EVAL_HPP

