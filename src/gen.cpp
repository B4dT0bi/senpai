
// includes

#include "attack.hpp"
#include "bit.hpp"
#include "common.hpp"
#include "gen.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "move.hpp"
#include "pos.hpp"

// constants

const Inc Inc_2N { Inc_N * 2 };

// prototypes

static void gen_pseudos (List & list, const Pos & pos);

static void add_en_passant (List & list, const Pos & pos);
static void add_castling   (List & list, const Pos & pos);

static void add_pawn_moves    (List & list, const Pos & pos, Side sd, Bit froms, Bit tos);
static void add_pawn_captures (List & list, const Pos & pos, Side sd, Bit froms, Bit tos);

static void add_piece_moves (List & list, const Pos & pos, Bit    froms, Bit tos);
static void add_piece_moves (List & list, const Pos & pos, Square from,  Bit tos);

static void add_piece_moves_rare (List & list, const Pos & pos, Bit    froms, Bit tos);
static void add_piece_moves_rare (List & list, const Pos & pos, Square from,  Bit tos);

static void add_moves (List & list, const Pos & pos, Bit froms, Square to);

static void add_moves_from (List & list, Bit froms, Inc inc);

static void add_move       (List & list, const Pos & pos, Square from, Square to);
static void add_piece_move (List & list, const Pos & pos, Square from, Square to);
static void add_pawn_move  (List & list, Square from, Square to);
static void add_promotion  (List & list, Square from, Square to);

// functions

void gen_legals(List & list, const Pos & pos) {

   List tmp;
   gen_moves(tmp, pos);

   list.clear();

   for (int i = 0; i < tmp.size(); i++) {
      Move mv = tmp[i];
      if (move::pseudo_is_legal(mv, pos)) list.add(mv);
   }
}

void gen_moves(List & list, const Pos & pos) {
   gen_moves(list, pos, checks(pos));
}

void gen_moves(List & list, const Pos & pos, Bit checks) {

   if (checks != 0) {
      gen_evasions(list, pos, checks);
   } else {
      gen_pseudos(list, pos);
   }
}

static void gen_pseudos(List & list, const Pos & pos) {

   list.clear();

   Side sd = pos.turn();
   Side xd = side_opp(sd);

   add_pawn_moves   (list, pos, sd, pos.pawns(sd), pos.empties());
   add_pawn_captures(list, pos, sd, pos.pawns(sd), pos.pieces(xd));
   add_piece_moves  (list, pos, pos.non_pawns(sd), ~pos.pieces(sd));

   add_en_passant(list, pos);
   add_castling  (list, pos);
}

void gen_evasions(List & list, const Pos & pos, Bit checks) {

   assert(checks != 0);

   list.clear();

   Side sd = pos.turn();

   Bit kings = pos.pieces(King, sd);
   Square king = bit::first(kings);

   // non-king moves

   if (bit::is_single(checks)) {

      Square check = bit::first(checks);

      // captures

      add_moves(list, pos, pos.pieces(sd) & ~kings, check); // includes pawns
      if (pos.is_piece(check, Pawn)) add_en_passant(list, pos);

      // interpositions

      Bit tos = bit::between(king, check);

      if (tos != 0) {
         add_pawn_moves (list, pos, sd, pos.pawns(sd), tos);
         add_piece_moves(list, pos, pos.non_king(sd), tos);
      }
   }

   // king moves

   add_piece_moves(list, pos, king, ~pos.pieces(sd));
}

void gen_eva_caps(List & list, const Pos & pos, Bit checks) {

   assert(checks != 0);

   list.clear();

   Side sd = pos.turn();
   Side xd = side_opp(sd);

   Bit kings = pos.pieces(King, sd);

   // non-king captures

   if (bit::is_single(checks)) {

      Square check = bit::first(checks);

      add_moves(list, pos, pos.pieces(sd) & ~kings, check); // includes pawns
      if (pos.is_piece(check, Pawn)) add_en_passant(list, pos);
   }

   // king captures

   add_piece_moves(list, pos, bit::first(kings), pos.pieces(xd));
}

void gen_captures(List & list, const Pos & pos) {
   gen_captures(list, pos, pos.turn());
}

void gen_captures(List & list, const Pos & pos, Side sd) {

   list.clear();

   Side xd = side_opp(sd);

   add_pawn_captures   (list, pos, sd, pos.pawns(sd), pos.pieces(xd));
   add_piece_moves_rare(list, pos, pos.non_pawns(sd), pos.pieces(xd));

   add_en_passant(list, pos);
}

void add_promotions(List & list, const Pos & pos) {
   add_promotions(list, pos, pos.turn());
}

void add_promotions(List & list, const Pos & pos, Side sd) {
   add_pawn_moves(list, pos, sd, pos.pawns(sd), pos.empties() & bit::Promotion_Squares);
}

void add_checks(List & list, const Pos & pos) {

   Side sd = pos.turn();
   Side xd = side_opp(sd);

   Square king = pos.king(xd);

   Bit empties = pos.empties();
   Bit pieces  = pos.pieces();
   Bit pins = Bit(0);

   Bit froms = pos.non_king(sd);

   // discovered checks

   pins = ::pins(pos, king);

   for (Bit bf = froms & pins; bf != 0; bf = bit::rest(bf)) {
      Square from = bit::first(bf);
      add_piece_moves(list, pos, from, empties);
   }

   // piece direct checks

   for (Bit bf = froms & ~pins; bf != 0; bf = bit::rest(bf)) {

      Square from = bit::first(bf);
      Piece  pc   = pos.piece(from);

      Bit tos = empties
              & bit::piece_attacks_to(pc, sd, king)
              & ~bit::pawn_attacks(xd, pos.pawns(xd)); // pawn safe

      for (Bit bt = bit::piece_attacks(pc, sd, from) & tos; bt != 0; bt = bit::rest(bt)) {
         Square to = bit::first(bt);
         if (bit::line_is_empty(to, king, pieces)) add_move(list, pos, from, to);
      }
   }
}

static void add_castling(List & list, const Pos & pos) {

   Side sd = pos.turn();
   Side xd = side_opp(sd);

   for (Bit b = pos.castling_rooks(sd); b != 0; b = bit::rest(b)) {

      Square kf = pos.king(sd);
      Square rf = bit::first(b);

      Square kt, rt;

      Rank rk = rank_side(Rank_1, sd);

      if (square_file(rf) > square_file(kf)) {
         kt = square_make(File_G, rk);
         rt = square_make(File_F, rk);
      } else {
         kt = square_make(File_C, rk);
         rt = square_make(File_D, rk);
      }

      // conditions

      Bit pieces = pos.pieces();
      bit::clear(pieces, kf);
      bit::clear(pieces, rf);

      if ((bit::line(kf, kt) & pieces) != 0) goto cont;
      if ((bit::line(rf, rt) & pieces) != 0) goto cont;

      for (Bit b = bit::between(kf, kt); b != 0; b = bit::rest(b)) { // kf and kt are checked elsewhere
         Square sq = bit::first(b);
         if (has_attack(pos, xd, sq, pieces)) goto cont;
      }

      assert(bit::line_is_empty(kf, rf, pieces));
      list.add_move(kf, rf, King); // fake promotion to king

      cont : ;
   }
}

static void add_en_passant(List & list, const Pos & pos) {

   Square to = pos.ep_sq();

   if (to != Square_None) {

      Side sd = pos.turn();

      for (Bit b = pos.pawns(sd) & bit::piece_attacks_to(Pawn, sd, to); b != 0; b = bit::rest(b)) {
         Square from = bit::first(b);
         list.add_move(from, to, Pawn); // fake promotion to pawn
      }
   }
}

static void add_pawn_moves(List & list, const Pos & pos, Side sd, Bit froms, Bit tos) {

   if (sd == White) {
      add_moves_from(list, froms & bit::rank(Rank_2, sd) & (pos.empties() >> Inc_N) & (tos >> Inc_2N), +Inc_2N);
      add_moves_from(list, froms & (tos >> Inc_N), +Inc_N);
   } else {
      add_moves_from(list, froms & bit::rank(Rank_2, sd) & (pos.empties() << Inc_N) & (tos << Inc_2N), -Inc_2N);
      add_moves_from(list, froms & (tos << Inc_N), -Inc_N);
   }
}

static void add_pawn_captures(List & list, const Pos & /* pos */, Side sd, Bit froms, Bit tos) {

   if (sd == White) {
      add_moves_from(list, froms & (tos << Inc_SW), -Inc_SW);
      add_moves_from(list, froms & (tos >> Inc_NW), +Inc_NW);
   } else {
      add_moves_from(list, froms & (tos >> Inc_SW), +Inc_SW);
      add_moves_from(list, froms & (tos << Inc_NW), -Inc_NW);
   }
}

static void add_piece_moves(List & list, const Pos & pos, Bit froms, Bit tos) {

   for (Bit b = froms; b != 0; b = bit::rest(b)) {
      Square from = bit::first(b);
      add_piece_moves(list, pos, from, tos);
   }
}

static void add_piece_moves(List & list, const Pos & pos, Square from, Bit tos) {

   Piece pc = pos.piece(from);
   assert(pc != Pawn);

   for (Bit b = bit::piece_attacks(pc, from, pos.pieces()) & tos; b != 0; b = bit::rest(b)) {
      Square to = bit::first(b);
      list.add_move(from, to);
   }
}

static void add_piece_moves_rare(List & list, const Pos & pos, Bit froms, Bit tos) {

   for (Bit b = froms; b != 0; b = bit::rest(b)) {
      Square from = bit::first(b);
      add_piece_moves_rare(list, pos, from, tos);
   }
}

static void add_piece_moves_rare(List & list, const Pos & pos, Square from, Bit tos) {

   Side sd = pos.turn();

   Piece pc = pos.piece(from);

   for (Bit b = bit::piece_attacks(pc, sd, from) & tos; b != 0; b = bit::rest(b)) {
      Square to = bit::first(b);
      add_move(list, pos, from, to);
   }
}

static void add_moves(List & list, const Pos & pos, Bit froms, Square to) {

   Side sd = pos.turn();

   for (Bit b = froms & pseudo_attacks_to(pos, sd, to); b != 0; b = bit::rest(b)) {
      Square from = bit::first(b);
      add_move(list, pos, from, to);
   }
}

static void add_moves_from(List & list, Bit froms, Inc inc) {

   for (Bit b = froms; b != 0; b = bit::rest(b)) {
      Square from = bit::first(b);
      add_pawn_move(list, from, square_make(from + inc));
   }
}

static void add_move(List & list, const Pos & pos, Square from, Square to) {

   if (pos.is_piece(from, Pawn)) {
      add_pawn_move(list, from, to);
   } else {
      add_piece_move(list, pos, from, to);
   }
}

static void add_piece_move(List & list, const Pos & pos, Square from, Square to) {

   if (bit::line_is_empty(from, to, pos.pieces())) {
      list.add_move(from, to);
   }
}

static void add_pawn_move(List & list, Square from, Square to) {

   if (square_is_promotion(to)) {
      add_promotion(list, from, to);
   } else {
      list.add_move(from, to);
   }
}

static void add_promotion(List & list, Square from, Square to) {

   assert(square_is_promotion(to));

   list.add_move(from, to, Queen);
   list.add_move(from, to, Knight);
   list.add_move(from, to, Rook);
   list.add_move(from, to, Bishop);
}

