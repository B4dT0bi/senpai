
// includes

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

#include "attack.hpp"
#include "bit.hpp"
#include "common.hpp"
#include "fen.hpp"
#include "hash.hpp"
#include "libmy.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "var.hpp"

// functions

Pos::Pos() {
}

Pos::Pos(Side turn, Bit piece_side[], Bit castling_rooks) {

   clear();

   // set up position

   for (int p = 0; p < Piece_Side_Size; p++) {

      Piece_Side ps = piece_side_make(p);

      Piece pc = piece_side_piece(ps);
      Side  sd = piece_side_side(ps);

      for (Bit b = piece_side[ps]; b != 0; b = bit::rest(b)) {
         Square sq = bit::first(b);
         add_piece(pc, sd, sq);
      }
   }

   p_castling_rooks = castling_rooks;

   if (turn != p_turn) switch_turn();

   update();
}

void Pos::update() {

   p_all = p_side[White] | p_side[Black];

   p_key_full = p_key_piece;

   p_key_full ^= hash::key_castling(White, castling_rooks(White));
   p_key_full ^= hash::key_castling(Black, castling_rooks(Black));

   if (p_ep_sq != Square_None) p_key_full ^= hash::key_en_passant(square_file(p_ep_sq));
}

void Pos::clear() {

   p_parent = nullptr;

   for (int pc = 0; pc < Piece_Size; pc++) {
      p_piece[pc] = Bit(0);
   }

   for (int sd = 0; sd < Side_Size; sd++) {
      p_side[sd] = Bit(0);
   }

   p_all = Bit(0);

   p_turn = White;

   p_castling_rooks = Bit(0);
   p_ep_sq = Square_None;
   p_ply = 0;
   p_rep = 0;

   for (int sq = 0; sq < Square_Size; sq++) {
      p_pc[sq] = Piece_None;
   }

   p_last_move = move::None;
   p_cap_sq = Square_None;
   p_key_piece = hash::key_turn(p_turn);
   p_key_pawn = Key(0);
}

Pos Pos::succ(Move mv) const {

   if (mv == move::Null) return null(); // moves in a PV can be "null"
   if (move::is_castling(mv)) return castle(mv);

   Square from = move::from(mv);
   Square to   = move::to(mv);

   Side sd = p_turn;
   Side xd = side_opp(sd);

   assert( is_side(from, sd));
   assert(!is_side(to,   sd));

   Pos pos = *this;

   pos.p_parent = this;

   pos.p_ep_sq = Square_None;
   pos.p_ply = move::is_conversion(mv, *this) ? 0 : p_ply + 1;
   pos.p_rep = pos.p_ply;

   pos.p_last_move = mv;
   pos.p_cap_sq = Square_None;

   Piece pc = piece_make(p_pc[from]);
   Piece cp = Piece(p_pc[to]); // can be Piece_None

   if (cp != Piece_None) { // capture

      assert(cp != King);

      pos.remove_piece(cp, xd, to);
      pos.p_cap_sq = to;

   } else if (move::is_en_passant(mv)) {

      pos.remove_piece(Pawn, xd, square_rear(to, sd));
      pos.p_cap_sq = to;
   }

   if (move::is_promotion(mv)) {

      pos.remove_piece(pc, sd, from);
      pos.add_piece(move::prom(mv), sd, to);

      pos.p_cap_sq = to;

   } else {

      pos.move_piece(pc, sd, from, to);
   }

   // special moves

   if (pc == Pawn
    && square_rank(from, sd) == Rank_2
    && square_rank(to,   sd) == Rank_4) {

      Square sq = square_make((from + to) / 2);
      if ((pos.pawns(xd) & bit::pawn_attacks_to(xd, sq)) != 0) pos.p_ep_sq = sq;

   } else if (pc == King) {

      pos.p_castling_rooks &= ~bit::rank(Rank_1, sd);
   }

   pos.switch_turn();

   pos.update();
   return pos;
}

Pos Pos::castle(Move mv) const {

   assert(move::is_castling(mv));

   Side sd = p_turn;

   Square kf = move::from(mv);
   Square rf = move::to(mv);

   Square kt, rt;

   Rank rk = rank_side(Rank_1, sd);

   if (square_file(rf) > square_file(kf)) {
      kt = square_make(File_G, rk);
      rt = square_make(File_F, rk);
   } else {
      kt = square_make(File_C, rk);
      rt = square_make(File_D, rk);
   }

   Pos pos = *this;

   pos.p_parent = this;

   pos.p_ep_sq = Square_None;
   pos.p_ply = move::is_conversion(mv, *this) ? 0 : p_ply + 1;
   pos.p_rep = pos.p_ply;

   pos.p_last_move = mv;
   pos.p_cap_sq = Square_None;

   pos.remove_piece(Rook, sd, rf);
   pos.move_piece  (King, sd, kf, kt);
   pos.add_piece   (Rook, sd, rt);

   pos.p_castling_rooks &= ~bit::rank(Rank_1, sd);

   pos.switch_turn();

   pos.update();
   return pos;
}

Pos Pos::null() const {

   Pos pos = *this;

   pos.p_parent = this;

   pos.switch_turn();

   pos.p_ep_sq = Square_None;
   pos.p_ply = p_ply + 1;
   pos.p_rep = 0; // don't detect repetition across a null move

   pos.p_last_move = move::Null;
   pos.p_cap_sq = Square_None;

   pos.update();
   return pos;
}

void Pos::switch_turn() {
   p_turn = side_opp(p_turn);
   p_key_piece ^= hash::key_turn();
}

void Pos::move_piece(Piece pc, Side sd, Square from, Square to) {
   remove_piece(pc, sd, from);
   add_piece(pc, sd, to);
}

void Pos::add_piece(Piece pc, Side sd, Square sq) {

   assert(pc != Piece_None);

   assert(!bit::has(p_piece[pc], sq));
   assert(!bit::has(p_side[sd], sq));

   bit::flip(p_piece[pc], sq);
   bit::flip(p_side[sd], sq);

   assert(p_pc[sq] == Piece_None);
   p_pc[sq] = pc;

   p_key_piece ^= hash::key_piece(pc, sd, sq);
   if (pc == Pawn) p_key_pawn ^= hash::key_piece(pc, sd, sq);
}

void Pos::remove_piece(Piece pc, Side sd, Square sq) {

   assert(pc != Piece_None);

   assert(bit::has(p_piece[pc], sq));
   assert(bit::has(p_side[sd], sq));

   bit::flip(p_piece[pc], sq);
   bit::flip(p_side[sd], sq);

   bit::clear(p_castling_rooks, sq); // moved or captured

   assert(p_pc[sq] == pc);
   p_pc[sq] = Piece_None;

   p_key_piece ^= hash::key_piece(pc, sd, sq);
   if (pc == Pawn) p_key_pawn ^= hash::key_piece(pc, sd, sq);
}

bool Pos::is_draw() const {

   if (p_ply >= 100) {
      return !is_mate(*this);
   } else if (p_rep >= 4) {
      return is_rep();
   } else {
      return false;
   }
}

bool Pos::is_rep() const {

   const Pos * pos = this;

   for (int i = 0; i < p_rep / 2; i++) {
      pos = pos->p_parent->p_parent;
      if (pos->key() == key()) return true;
   }

   return false;
}

namespace pos { // ###

// variables

Pos Start;

// functions

void init() {
   Start = pos_from_fen(Start_FEN);
}

double phase(const Pos & pos) {

   double phase = double(stage(pos)) / double(Stage_Size);

   assert(phase >= 0.0 && phase <= 1.0);
   return phase;
}

int stage(const Pos & pos) {

   int stage = 24 - (force(pos, White) + force(pos, Black));
   if (stage < 0) stage = 0;

   assert(stage >= 0 && stage <= Stage_Size);
   return stage;
}

bool lone_king(const Pos & pos, Side sd) {
   return pos.non_pawns(sd) == pos.pieces(King, sd);
}

bool opposit_bishops(const Pos & pos) {

   Bit white = pos.non_king(White);
   Bit black = pos.non_king(Black);

   return white == pos.pieces(Bishop, White)
       && black == pos.pieces(Bishop, Black)
       && bit::is_single(white)
       && bit::is_single(black)
       && square_colour(bit::first(white)) != square_colour(bit::first(black));
}

int force(const Pos & pos, Side sd) {

   return pos.count(Knight, sd) * 1
        + pos.count(Bishop, sd) * 1
        + pos.count(Rook,   sd) * 2
        + pos.count(Queen,  sd) * 4;
}

}

