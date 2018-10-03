
#ifndef MOVE_HPP
#define MOVE_HPP

// includes

#include <string>

#include "common.hpp"
#include "libmy.hpp"

class Pos;

namespace move {

// constants

const Move None { Move(-1) };
const Move Null { Move( 0) };

// functions

void init ();

Move make (Square from, Square to, Piece prom = Piece_None);

Square from (Move mv);
Square to   (Move mv);
Piece  prom (Move mv);

bool is_promotion      (Move mv);
bool is_underpromotion (Move mv);
bool is_castling       (Move mv);
bool is_en_passant     (Move mv);

bool is_tactical   (Move mv, const Pos & pos);
bool is_capture    (Move mv, const Pos & pos);
bool is_recapture  (Move mv, const Pos & pos);
bool is_conversion (Move mv, const Pos & pos);

Square castling_king_to (Move mv);
Square castling_rook_to (Move mv);

Piece piece   (Move mv, const Pos & pos);
Piece capture (Move mv, const Pos & pos);
Side  side    (Move mv, const Pos & pos);

Move_Index index           (Move mv, const Pos & pos);
Move_Index index_last_move (const Pos & pos);

bool pseudo_is_legal (Move mv, const Pos & pos);
bool is_check        (Move mv, const Pos & pos);

std::string to_uci   (Move mv, const Pos & pos);
Move        from_uci (const std::string & s, const Pos & pos);

}

#endif // !defined MOVE_HPP

