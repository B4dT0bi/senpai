
// includes

#include <cctype>
#include <string>

#include "attack.hpp"
#include "bit.hpp"
#include "common.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "util.hpp"
#include "var.hpp"

namespace move {

// functions

Move make(Square from, Square to, Piece prom) {
   return Move((prom << 12) | (from << 6) | (to << 0));
}

Square from(Move mv) {
   assert(mv != None);
   assert(mv != Null);
   return Square((int(mv) >> 6) & 077);
}

Square to(Move mv) {
   assert(mv != None);
   assert(mv != Null);
   return Square((int(mv) >> 0) & 077);
}

Piece prom(Move mv) {
   assert(mv != None);
   assert(mv != Null);
   return Piece((int(mv) >> 12) & 7);
}

bool is_promotion(Move mv) {
   Piece prom = move::prom(mv);
   return prom >= Knight && prom <= Queen;
}

bool is_underpromotion(Move mv) {
   Piece prom = move::prom(mv);
   return prom >= Knight && prom <= Rook;
}

bool is_castling(Move mv) {
   return prom(mv) == King;
}

bool is_en_passant(Move mv) {
   return prom(mv) == Pawn;
}

bool is_tactical(Move mv, const Pos & pos) {
   return is_capture(mv, pos) || is_promotion(mv);
}

bool is_capture(Move mv, const Pos & pos) {
   return (!pos.is_empty(to(mv)) && !is_castling(mv))
       || is_en_passant(mv);
}

bool is_recapture(Move mv, const Pos & pos) {
   return to(mv) == pos.cap_sq() && move_is_win(mv, pos);
}

bool is_conversion(Move mv, const Pos & pos) {
   return pos.is_piece(from(mv), Pawn) || is_capture(mv, pos);
}

Square castling_king_to(Move mv) {

   assert(is_castling(mv));

   Square from = move::from(mv);
   Square to   = move::to(mv);

   return (square_file(to) > square_file(from))
        ? square_make(File_G, square_rank(to))
        : square_make(File_C, square_rank(to));
}

Square castling_rook_to(Move mv) {

   assert(is_castling(mv));

   Square from = move::from(mv);
   Square to   = move::to(mv);

   return (square_file(to) > square_file(from))
        ? square_make(File_F, square_rank(to))
        : square_make(File_D, square_rank(to));
}

Piece piece(Move mv, const Pos & pos) {
   assert(mv != None);
   assert(mv != Null);
   return pos.piece(from(mv));
}

Piece capture(Move mv, const Pos & pos) {

   assert(mv != None);
   assert(mv != Null);

   if (is_castling(mv))   return Piece_None;
   if (is_en_passant(mv)) return Pawn;

   return pos.piece(to(mv));
}

Side side(Move mv, const Pos & pos) {
   assert(mv != None);
   assert(mv != Null);
   return pos.side(from(mv));
}

Move_Index index(Move mv, const Pos & pos) {

   assert(mv != None);
   assert(mv != Null);

   Piece pc = piece(mv, pos);
   Side  sd = pos.turn();

   return Move_Index((sd << 9) | (pc << 6) | (to(mv) << 0));
}

Move_Index index_last_move(const Pos & pos) {

   Move mv = pos.last_move();
   if (mv == move::None || mv == move::Null) return Move_Index_None;

   Piece pc = move::is_castling(mv) ? King : pos.piece(to(mv));
   Side  sd = side_opp(pos.turn());

   return Move_Index((sd << 9) | (pc << 6) | (to(mv) << 0));
}

bool pseudo_is_legal(Move mv, const Pos & pos) {

   assert(mv != None);
   assert(mv != Null);

   Square from = move::from(mv);
   Square to   = move::to(mv);

   Piece pc = pos.piece(from);
   Side  sd = pos.side(from);

   Side xd = side_opp(sd);

   Square king = pos.king(sd);

   Bit pieces = pos.pieces();

   if (is_castling(mv)) {
      bit::clear(pieces, to); // remove rook
      to = castling_king_to(mv);
   }

   bit::clear(pieces, from);
   bit::set(pieces, to);

   // king move?

   if (pc == King) return !has_attack(pos, xd, to, pieces);

   // pinned piece?

   Bit beyond = bit::beyond(king, from);

   if (is_en_passant(mv)) {

      Square sq = square_rear(to, sd);

      bit::clear(pieces, sq);
      beyond |= bit::beyond(king, sq);
   }

   for (Bit b = pos.sliders(xd) & beyond; b != 0; b = bit::rest(b)) {

      Square ds = bit::first(b);
      Piece  dp = pos.piece(ds);

      if (bit::piece_attack(dp, xd, ds, king, pieces) && ds != to) return false;
   }

   return true;
}

bool is_check(Move mv, const Pos & pos) {

   assert(mv != None);
   assert(mv != Null);

   Square from = move::from(mv);
   Square to   = move::to(mv);

   Bit pieces = pos.pieces();

   if (is_castling(mv)) {

      bit::clear(pieces, from); // remove king

      from = to;
      to = castling_rook_to(mv);
   }

   Piece pc = pos.piece(from);
   Side  sd = pos.side(from);

   Square king = pos.king(side_opp(sd));

   bit::clear(pieces, from);
   bit::set(pieces, to);

   // direct check?

   if (is_promotion(mv)) pc = prom(mv);
   if (bit::piece_attack(pc, sd, to, king, pieces)) return true;

   // discovered check?

   Bit beyond = bit::beyond(king, from);

   if (is_en_passant(mv)) {

      Square sq = square_rear(to, sd);

      bit::clear(pieces, sq);
      beyond |= bit::beyond(king, sq);
   }

   for (Bit b = pos.sliders(sd) & beyond; b != 0; b = bit::rest(b)) {

      Square ds = bit::first(b);
      Piece  dp = pos.piece(ds);

      if (bit::piece_attack(dp, sd, ds, king, pieces)) return true;
   }

   return false;
}

std::string to_uci(Move mv, const Pos & /* pos */) {

   if (mv == None) return "0000";
   if (mv == Null) return "0000";

   Square from = move::from(mv);
   Square to   = !var::Chess_960 && is_castling(mv)
               ? castling_king_to(mv)
               : move::to(mv);

   std::string s;
   s += square_to_string(from);
   s += square_to_string(to);
   if (is_promotion(mv)) s += std::tolower(piece_to_char(prom(mv)));

   return s;
}

Move from_uci(const std::string & s, const Pos & pos) {

   if (s == "0000") return Null;

   if (s.size() < 4 || s.size() > 5) throw Bad_Input();

   Square from = square_from_string(s.substr(0, 2));
   Square to   = square_from_string(s.substr(2, 2));

   Piece prom = Piece_None;
   if (s.size() == 5) prom = piece_from_char(std::toupper(s[4]));

   if (!var::Chess_960 && pos.is_piece(from, King) && square_dist(from, to) > 1) {
      if (square_file(to) == File_G) to = square_make(File_H, square_rank(to));
      if (square_file(to) == File_C) to = square_make(File_A, square_rank(to));
      prom = King;
   } else if (pos.is_side(to, pos.turn())) {
      assert(pos.is_piece(from, King));
      assert(pos.is_piece(to,   Rook));
      prom = King;
   } else if (pos.is_piece(from, Pawn) && to == pos.ep_sq()) {
      prom = Pawn;
   }

   return make(from, to, prom);
}

}

