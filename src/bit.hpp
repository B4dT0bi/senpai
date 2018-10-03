
#ifndef BIT_HPP
#define BIT_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

namespace bit {

// variables

extern Bit Pawn_Squares;
extern Bit Promotion_Squares;
extern Bit Colour_Squares[2];

// functions

void init ();

Bit  bit (Square sq);

bool has (Bit b, Square sq);
int  bit (Bit b, Square sq);

bool is_single (Bit b);
bool is_incl   (Bit b0, Bit b1);

void set   (Bit & b, Square sq);
void clear (Bit & b, Square sq);
void flip  (Bit & b, Square sq);

Bit  add    (Bit b, Square sq);
Bit  remove (Bit b, Square sq);

Square first (Bit b);
Bit    rest  (Bit b);
int    count (Bit b);

Bit  file (File fl);
Bit  rank (Rank rk);
Bit  rank (Rank rk, Side sd);

Bit  rect (int left, int bottom, int right, int top);

Bit  ray     (Square from, Square to);
Bit  beyond  (Square from, Square to);
Bit  between (Square from, Square to);
Bit  line    (Square from, Square to);

Bit  pawn_moves   (Side sd, Bit froms);
Bit  pawn_attacks (Side sd, Bit froms);

Bit  pawn_moves_to   (Side sd, Bit tos);
Bit  pawn_attacks_to (Side sd, Bit tos);

Bit  pawn_moves      (Side sd, Square from);
Bit  pawn_attacks    (Side sd, Square from);
Bit  pawn_attacks_to (Side sd, Square to);

Bit  piece_attacks    (Piece pc, Square from);
Bit  piece_attacks_to (Piece pc, Square to);

Bit  piece_attacks    (Piece pc, Side sd, Square from);
Bit  piece_attacks_to (Piece pc, Side sd, Square to);

Bit  piece_attacks    (Piece pc, Square from, Bit pieces);
Bit  piece_attacks_to (Piece pc, Square to,   Bit pieces);

bool piece_attack (Piece pc, Side sd, Square from, Square to);
bool piece_attack (Piece pc, Side sd, Square from, Square to, Bit pieces);

Bit  knight_attacks (Square from);
Bit  bishop_attacks (Square from, Bit pieces);
Bit  rook_attacks   (Square from, Bit pieces);
Bit  queen_attacks  (Square from, Bit pieces);
Bit  king_attacks   (Square from);

bool line_is_empty (Square from, Square to, Bit pieces);

}

#endif // !defined BIT_HPP

