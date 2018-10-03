
// includes

#include "bit.hpp"
#include "common.hpp"
#include "libmy.hpp"
#include "pawn.hpp"
#include "pos.hpp"

namespace pawn {

// variables

static Bit File_[Square_Size];
static Bit Rank_[Square_Size];

static Bit Files[Square_Size];
static Bit Ranks[Square_Size];

static Bit File_Both[Square_Size];

static Bit Rank_Front[Side_Size][Square_Size];
static Bit Rank_Rear [Side_Size][Square_Size];

static Bit Ranks_Front[Side_Size][Square_Size];
static Bit Ranks_Rear [Side_Size][Square_Size];

// prototypes

static Bit bit_sides (Bit b);

static Bit pawns (const Pos & pos);

static Bit pawns_sd (const Pos & pos, Side sd);
static Bit pawns_xd (const Pos & pos, Side sd);

static Bit attacks_sd (const Pos & pos, Side sd);
static Bit attacks_xd (const Pos & pos, Side sd);

static Bit unsafe_sd (const Pos & pos, Side sd);
static Bit unsafe_xd (const Pos & pos, Side sd);

// functions

void init() {

   for (int r = 0; r < Rank_Size; r++) {

      for (int f = 0; f < File_Size; f++) {

         File fl = file_make(f);
         Rank rk = rank_make(r);

         Square sq = square_make(fl, rk);

         File_[sq] = bit::file(fl);
         Rank_[sq] = bit::rank(rk);

         if (fl > 0)             File_Both[sq] |= bit::file(fl - 1);
         if (fl < File_Size - 1) File_Both[sq] |= bit::file(fl + 1);

         if (rk > 0)             Rank_Rear [White][sq] = bit::rank(rk - 1);
         if (rk < Rank_Size - 1) Rank_Front[White][sq] = bit::rank(rk + 1);

         Rank_Front[Black][sq] = Rank_Rear [White][sq];
         Rank_Rear [Black][sq] = Rank_Front[White][sq];

         Ranks_Front[White][sq] = bit::rect(0, rk + 1, File_Size, Rank_Size);
         Ranks_Rear [White][sq] = bit::rect(0, 0,      File_Size, rk);

         Ranks_Front[Black][sq] = Ranks_Rear [White][sq];
         Ranks_Rear [Black][sq] = Ranks_Front[White][sq];

         Files[sq] = File_[sq] | File_Both[sq];
         Ranks[sq] = Rank_Front[White][sq] | Rank_[sq] | Rank_Rear[White][sq];
      }
   }
}

Bit weak(const Pos & pos, Side sd) {

   Bit pawns = pawns_sd(pos, sd);
   Bit safe = ~unsafe_sd(pos, sd);

   Bit weak = pawns;

   // forward

   Bit forward = pawns;

   while (true) {

      Bit next = forward | (bit::pawn_moves(sd, forward) & safe);
      if (next == forward) break;

      forward = next;
   }

   weak &= ~(bit_sides(forward) | bit::pawn_attacks(sd, forward));

   // backward

   Bit backward = bit_sides(pawns);

   while (true) {

      Bit next = backward | bit::pawn_moves_to(sd, backward & safe);
      if (next == backward) break;

      backward = next;
   }

   weak &= ~backward;

   // wrap up

   return weak;
}

Bit strong(const Pos & pos, Side sd) { // squares not attackable by opponent pawns

   Side xd = side_opp(sd);

   Bit safe = ~unsafe_xd(pos, sd);

   // forward

   Bit forward = pawns_xd(pos, sd);

   while (true) {

      Bit next = forward | (bit::pawn_moves(xd, forward) & safe);
      if (next == forward) break;

      forward = next;
   }

   Bit strong = ~(pawns(pos) | bit::pawn_attacks(xd, forward));

   Bit bad = Bit(0);

   for (Bit b = strong; b != 0; b = bit::rest(b)) {
      Square sq = bit::first(b);
      if (!is_strong(pos, sq, sd)) bit::set(bad, sq);
   }

   return ~bit::pawn_attacks(xd, forward);
}

Bit blocked(const Pos & pos, Side sd) {
   Bit unsafe = unsafe_sd(pos, sd);
   return pawns_sd(pos, sd) & bit::pawn_moves_to(sd, unsafe);
}

static Bit bit_sides(Bit b) {
   return Bit(b >> Inc_W) | Bit(b << Inc_W);
}

bool is_passed(const Pos & pos, Square sq, Side sd) {
   return (pawns_xd(pos, sd) & (Files[sq] & Ranks_Front[sd][sq])) == 0
       && (pawns_sd(pos, sd) & (File_[sq] & Ranks_Front[sd][sq])) == 0;
}

bool is_duo(const Pos & pos, Square sq, Side sd) {
   return (pawns_sd(pos, sd) & (File_Both[sq] & Rank_[sq])) != 0;
}

bool is_protected(const Pos & pos, Square sq, Side sd) {
   return (pawns_sd(pos, sd) & (File_Both[sq] & Rank_Rear[sd][sq])) != 0;
}

bool is_ram(const Pos & pos, Square sq, Side sd) {
   return (pawns_xd(pos, sd) & (File_[sq] & Rank_Front[sd][sq])) != 0;
}

bool is_open(const Pos & pos, Square sq, Side sd) {
   return (pawns_sd(pos, sd) & File_[sq]) == 0;
}

bool is_strong(const Pos & pos, Square sq, Side sd) {
   return (pawns_xd(pos, sd) & (File_Both[sq] & Ranks_Front[sd][sq])) == 0;
}

Bit file(Square sq) {
   return File_[sq];
}

Bit rank(Square sq) {
   return Rank_[sq];
}

Bit fronts(Square sq, Side sd) {
   return Ranks_Front[sd][sq];
}

Bit rears(Square sq, Side sd) {
   return Ranks_Rear[sd][sq];
}

static Bit pawns(const Pos & pos) {
   return pos.pieces(Pawn);
}

static Bit pawns_sd(const Pos & pos, Side sd) {
   return pos.pawns(sd);
}

static Bit pawns_xd(const Pos & pos, Side sd) {
   return pos.pawns(side_opp(sd));
}

static Bit attacks_sd(const Pos & pos, Side sd) {
   return bit::pawn_attacks(sd, pos.pawns(sd));
}

static Bit attacks_xd(const Pos & pos, Side sd) {
   Side xd = side_opp(sd);
   return bit::pawn_attacks(xd, pos.pawns(xd));
}

static Bit unsafe_sd(const Pos & pos, Side sd) {
   return pawns(pos) | (attacks_xd(pos, sd) & ~attacks_sd(pos, sd));
}

static Bit unsafe_xd(const Pos & pos, Side sd) {
   return pawns(pos) | (attacks_sd(pos, sd) & ~attacks_xd(pos, sd));
}

}

