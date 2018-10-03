
// includes

#include <algorithm>

#include "attack.hpp"
#include "bit.hpp"
#include "common.hpp"
#include "eval.hpp" // for piece_mat
#include "gen.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "move.hpp"
#include "pos.hpp"

// prototypes

static bool can_play (const Pos & pos);

static bool in_check (const Pos & pos, Side sd);

static Score  see_rec  (const Pos & pos, Side sd, Square to, Bit pieces, Piece cp);
static Square pick_lva (const Pos & pos, Side sd, Square to, Bit pieces);

static Bit slider_attacks_to (const Pos & pos, Side sd, Square to);

// functions

bool is_mate(const Pos & pos) {
   return in_check(pos) && !can_play(pos);
}

bool is_stalemate(const Pos & pos) {
   return !in_check(pos) && !can_play(pos);
}

static bool can_play(const Pos & pos) {

   List list;
   gen_moves(list, pos);

   for (int i = 0; i < list.size(); i++) {
      Move mv = list[i];
      if (move::pseudo_is_legal(mv, pos)) return true;
   }

   return false;
}

bool is_legal(const Pos & pos) {
   return !in_check(pos, side_opp(pos.turn()));
}

bool in_check(const Pos & pos) {
   return checks(pos) != 0;
}

static bool in_check(const Pos & pos, Side sd) {
   return has_attack(pos, side_opp(sd), pos.king(sd));
}

Bit checks(const Pos & pos) {

   Bit checks = Bit(0);

   Side xd = pos.turn();
   Side sd = side_opp(xd);

   Square king = pos.king(xd);

   Bit pieces = pos.pieces();

   Move mv = pos.last_move();

   if (mv == move::None) return attacks_to(pos, sd, king, pieces);
   if (mv == move::Null) return Bit(0);

   Square from = move::from(mv);
   Square to   = move::is_castling(mv) ? move::castling_rook_to(mv) : move::to(mv);

   Piece pc = pos.piece(to);
   assert(pos.is_side(to, sd));

   // direct check?

   if (bit::piece_attack(pc, sd, to, king, pieces)) bit::set(checks, to);

   // discovered check?

   Bit beyond = bit::beyond(king, from);
   if (move::is_en_passant(mv)) beyond |= bit::beyond(king, square_rear(to, sd));

   for (Bit b = pos.sliders(sd) & beyond; b != 0; b = bit::rest(b)) {

      Square ds = bit::first(b);
      Piece  dp = pos.piece(ds);

      if (bit::piece_attack(dp, sd, ds, king, pieces)) bit::set(checks, ds);
   }

   return checks;
}

bool move_is_safe(Move mv, const Pos & pos) {

   if (move::is_underpromotion(mv)) return false;

   Piece pc = move::piece(mv, pos);

   if (pc == King) return true; // always safe when legal
   if (move::is_capture(mv, pos) && piece_mat(move::capture(mv, pos)) >= piece_mat(pc)) return true; // low x high

   return see(mv, pos) >= 0;
}

bool move_is_win(Move mv, const Pos & pos) {

   assert(move::is_tactical(mv, pos));

   if (move::is_underpromotion(mv)) return false;

   Piece pc = move::piece(mv, pos);

   if (pc == King) return true; // always a win when legal
   if (move::is_capture(mv, pos) && piece_mat(move::capture(mv, pos)) > piece_mat(pc)) return true; // low x high

   return see(mv, pos) > 0;
}

Score see(Move mv, const Pos & pos) {

   Square from = move::from(mv);
   Square to   = move::is_castling(mv) ? move::castling_king_to(mv) : move::to(mv);

   Piece pc = move::piece(mv, pos);
   Side  sd = move::side(mv, pos);

   Score sc = Score(0);

   if (move::is_capture(mv, pos)) sc += piece_mat(move::capture(mv, pos));

   if (move::is_promotion(mv)) {
      pc = move::prom(mv);
      sc += piece_mat(pc) - piece_mat(Pawn);
   }

   sc -= see_rec(pos, side_opp(sd), to, bit::remove(pos.pieces(), from), pc);

   return sc;
}

Score see_max(Move mv, const Pos & pos) {

   Score sc = Score(0);
   if (move::is_capture(mv, pos)) sc += piece_mat(move::capture(mv, pos));
   if (move::is_promotion(mv))    sc += piece_mat(move::prom(mv)) - piece_mat(Pawn);

   return sc;
}

static Score see_rec(const Pos & pos, Side sd, Square to, Bit pieces, Piece cp) {

   assert(cp != Piece_None);

   Score bs = Score(0); // stand pat

   Square from = pick_lva(pos, sd, to, pieces);

   if (from != Square_None) {

      Piece pc = pos.piece(from);

      Score sc = piece_mat(cp);
      if (cp != King) sc -= see_rec(pos, side_opp(sd), to, bit::remove(pieces, from), pc);

      if (sc > bs) bs = sc;
   }

   assert(bs >= 0);
   return bs;
}

bool has_attack(const Pos & pos, Side sd, Square to) {
   return has_attack(pos, sd, to, pos.pieces());
}

bool has_attack(const Pos & pos, Side sd, Square to, Bit pieces) {

   for (Bit b = pseudo_attacks_to(pos, sd, to); b != 0; b = bit::rest(b)) {
      Square from = bit::first(b);
      if (bit::line_is_empty(from, to, pieces)) return true;
   }

   return false;
}

Bit attacks_to(const Pos & pos, Side sd, Square to, Bit pieces) {

   Bit froms = Bit(0);

   for (Bit b = pseudo_attacks_to(pos, sd, to); b != 0; b = bit::rest(b)) {
      Square from = bit::first(b);
      if (bit::line_is_empty(from, to, pieces)) bit::set(froms, from);
   }

   return froms;
}

Bit pseudo_attacks_to(const Pos & pos, Side sd, Square to) {

   Bit froms = Bit(0);

   for (int p = 0; p < Piece_Size; p++) {
      Piece pc = piece_make(p);
      froms |= pos.pieces(pc, sd) & bit::piece_attacks_to(pc, sd, to);
   }

   return froms;
}

bool is_pinned(const Pos & pos, Square king, Square sq, Side sd) {

   Side xd = side_opp(sd);

   Bit pieces = bit::remove(pos.pieces(), sq);

   for (Bit b = pos.sliders(xd) & bit::beyond(king, sq); b != 0; b = bit::rest(b)) {

      Square ds = bit::first(b);
      Piece  dp = pos.piece(ds);

      if (bit::piece_attack(dp, xd, ds, king, pieces)) return true;
   }

   return false;
}

Square pinned_by(const Pos & pos, Square king, Square sq, Side sd) {

   Side xd = side_opp(sd);

   Bit pieces = bit::remove(pos.pieces(), sq);

   for (Bit b = pos.sliders(xd) & bit::beyond(king, sq); b != 0; b = bit::rest(b)) {

      Square ds = bit::first(b);
      Piece  dp = pos.piece(ds);

      if (bit::piece_attack(dp, xd, ds, king, pieces)) return ds;
   }

   return Square_None;
}

Bit pins(const Pos & pos, Square king) {

   Bit pins = Bit(0);

   Side sd = pos.side(king);
   Side xd = side_opp(sd);

   for (Bit b = slider_attacks_to(pos, xd, king); b != 0; b = bit::rest(b)) {

      Square ds = bit::first(b);

      Bit between = bit::between(ds, king) & pos.pieces();
      if (bit::is_single(between)) pins |= between;
   }

   return pins;
}

static Bit slider_attacks_to(const Pos & pos, Side sd, Square to) {
   return ((pos.pieces(Bishop, sd) | pos.pieces(Queen, sd)) & bit::piece_attacks_to(Bishop, sd, to))
        | ((pos.pieces(Rook,   sd) | pos.pieces(Queen, sd)) & bit::piece_attacks_to(Rook,   sd, to));
}

static Square pick_lva(const Pos & pos, Side sd, Square to, Bit pieces) {

   for (int p = 0; p < Piece_Size; p++) {

      Piece pc = piece_make(p);
      Bit froms = pos.pieces(pc, sd) & pieces & bit::piece_attacks_to(pc, sd, to);

      for (Bit b = froms; b != 0; b = bit::rest(b)) {
         Square from = bit::first(b);
         if (bit::line_is_empty(from, to, pieces)) return from;
      }
   }

   return Square_None;
}

void Attack_Info::init(const Pos & pos) {

   for (int s = 0; s < Side_Size; s++) {

      Side sd = side_make(s);

      p_attacks[sd] = Bit(0);
      p_support[sd] = Bit(0);

      Piece pc;
      Bit froms;

      // pawns

      pc    = Pawn;
      froms = pos.pieces(pc, sd);

      p_le_pieces [sd][pc] = froms;
      p_le_attacks[sd][pc] = Bit(0);

      Bit tos = bit::pawn_attacks(sd, froms);

      p_le_attacks[sd][pc] |= tos;
      p_support[sd] |= tos & p_attacks[sd];
      p_attacks[sd] |= tos;

      // knight

      pc    = Knight;
      froms = pos.pieces(pc, sd);

      p_le_pieces [sd][pc] = p_le_pieces [sd][pc - 1] | froms;
      p_le_attacks[sd][pc] = p_le_attacks[sd][pc - 1];

      for (Bit b = froms; b != 0; b = bit::rest(b)) {

         Square from = bit::first(b);
         Bit    tos  = bit::knight_attacks(from);

         p_piece_attacks[from] = tos;
         p_le_attacks[sd][pc] |= tos;
         p_support[sd] |= tos & p_attacks[sd];
         p_attacks[sd] |= tos;
      }

      // bishop

      pc    = Bishop;
      froms = pos.pieces(pc, sd);

      p_le_pieces [sd][pc] = p_le_pieces [sd][pc - 1] | froms;
      p_le_attacks[sd][pc] = p_le_attacks[sd][pc - 1];

      for (Bit b = froms; b != 0; b = bit::rest(b)) {

         Square from = bit::first(b);
         Bit    tos  = bit::bishop_attacks(from, pos.pieces());

         p_piece_attacks[from] = tos;
         p_le_attacks[sd][pc] |= tos;
         p_support[sd] |= tos & p_attacks[sd];
         p_attacks[sd] |= tos;
      }

      // rook

      pc    = Rook;
      froms = pos.pieces(pc, sd);

      p_le_pieces [sd][pc] = p_le_pieces [sd][pc - 1] | froms;
      p_le_attacks[sd][pc] = p_le_attacks[sd][pc - 1];

      for (Bit b = froms; b != 0; b = bit::rest(b)) {

         Square from = bit::first(b);
         Bit    tos  = bit::rook_attacks(from, pos.pieces());

         p_piece_attacks[from] = tos;
         p_le_attacks[sd][pc] |= tos;
         p_support[sd] |= tos & p_attacks[sd];
         p_attacks[sd] |= tos;
      }

      // queen

      pc    = Queen;
      froms = pos.pieces(pc, sd);

      p_le_pieces [sd][pc] = p_le_pieces [sd][pc - 1] | froms;
      p_le_attacks[sd][pc] = p_le_attacks[sd][pc - 1];

      for (Bit b = froms; b != 0; b = bit::rest(b)) {

         Square from = bit::first(b);
         Bit    tos  = bit::queen_attacks(from, pos.pieces());

         p_piece_attacks[from] = tos;
         p_le_attacks[sd][pc] |= tos;
         p_support[sd] |= tos & p_attacks[sd];
         p_attacks[sd] |= tos;
      }

      // king

      pc    = King;
      froms = pos.pieces(pc, sd);

      p_le_pieces [sd][pc] = p_le_pieces [sd][pc - 1] | froms;
      p_le_attacks[sd][pc] = p_le_attacks[sd][pc - 1];

      for (Bit b = froms; b != 0; b = bit::rest(b)) {

         Square from = bit::first(b);
         Bit    tos  = bit::king_attacks(from);

         p_piece_attacks[from] = tos;
         p_le_attacks[sd][pc] |= tos;
         p_support[sd] |= tos & p_attacks[sd];
         p_attacks[sd] |= tos;
      }

      // wrap up

      p_le_pieces [sd][Knight] |= p_le_pieces [sd][Bishop];
      p_le_attacks[sd][Knight] |= p_le_attacks[sd][Bishop];
   }
}

Bit Attack_Info::piece_attacks(Square from) const {
   return p_piece_attacks[from];
}

Bit Attack_Info::attacks(Side sd) const {
   return p_attacks[sd];
}

Bit Attack_Info::pawn_attacks(Side sd) const {
   return p_le_attacks[sd][Pawn];
}

Bit Attack_Info::queen_attacks(Side sd) const {
   return p_le_attacks[sd][Queen];
}

Bit Attack_Info::queen_safe(Side sd) const {

   Side xd = side_opp(sd);

   return ~p_le_attacks[xd][King]
        | (~p_le_attacks[xd][Queen] & p_support[sd]);
}

