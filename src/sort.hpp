
#ifndef SORT_HPP
#define SORT_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

class List;
class Pos;

// functions

void sort_clear ();

void good_move (Move mv, const Pos & pos, Ply ply);
void bad_move  (Move mv, const Pos & pos, Ply ply);

void sort_mvv_lva (List & list, const Pos & pos);
void sort_all     (List & list, const Pos & pos, Move tt_move, Ply ply);
void sort_tt_move (List & list, const Pos & pos, Move tt_move);

#endif // !defined SORT_HPP

