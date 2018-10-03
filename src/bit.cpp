
// includes

#include <cstdio>
#include <iostream>

#include "bit.hpp"
#include "common.hpp"
#include "libmy.hpp"

namespace bit {

// constants

const int  Bishop_Bits {  9 };
const int  Rook_Bits   { 12 };

const int  Bishop_Size { 1 << Bishop_Bits };
const int  Rook_Size   { 1 << Rook_Bits };

// variables

Bit Pawn_Squares;
Bit Promotion_Squares;
Bit Colour_Squares[2];

static Bit File_[File_Size];
static Bit Rank_[Rank_Size];

static Bit Pawn_Moves   [Side_Size][Square_Size];
static Bit Pawn_Attacks [Side_Size][Square_Size];
static Bit Piece_Attacks[Side_Size][Piece_Size_2][Square_Size];

#if BMI
static Bit Bishop_Attacks[Square_Size][Bishop_Size];
static Bit Rook_Attacks  [Square_Size][Rook_Size];
#endif

static Bit Blocker[Square_Size];

static Bit Ray    [Square_Size][Square_Size];
static Bit Beyond [Square_Size][Square_Size];
static Bit Between[Square_Size][Square_Size];

// prototypes

static Bit ray_1      (Square from, Vec vec);
static Bit ray_almost (Square from, Vec vec);
static Bit ray        (Square from, Vec vec);

static Bit piece_attacks (Square from, Bit tos, Bit pieces);

// functions

void init() {

   // files and ranks

   for (int s = 0; s < Square_Size; s++) {

      Square sq = square_make(s);

      set(File_[square_file(sq)], sq);
      set(Rank_[square_rank(sq)], sq);

      set(Colour_Squares[square_colour(sq)], sq);
   }

   Pawn_Squares      = rect(0, 1, File_Size, Rank_Size - 1);
   Promotion_Squares = ~Pawn_Squares;

   // piece init

   Vec vec_nw  = vector_make(-1, +1);
   Vec vec_n   = vector_make( 0, +1);
   Vec vec_ne  = vector_make(+1, +1);
   Vec vec_w   = vector_make(-1,  0);
   Vec vec_e   = vector_make(+1,  0);
   Vec vec_sw  = vector_make(-1, -1);
   Vec vec_s   = vector_make( 0, -1);
   Vec vec_se  = vector_make(+1, -1);

   Vec vec_nnw = vector_make(-1, +2);
   Vec vec_nne = vector_make(+1, +2);
   Vec vec_nww = vector_make(-2, +1);
   Vec vec_nee = vector_make(+2, +1);
   Vec vec_sww = vector_make(-2, -1);
   Vec vec_see = vector_make(+2, -1);
   Vec vec_ssw = vector_make(-1, -2);
   Vec vec_sse = vector_make(+1, -2);

   const Vec Queen_Vec[8] {
      vec_nw, vec_n, vec_ne, vec_w, vec_e, vec_sw, vec_s, vec_se,
   };

   const Vec Knight_Vec[8] {
      vec_nnw, vec_nne, vec_nww, vec_nee, vec_sww, vec_see, vec_ssw, vec_sse,
   };

   // piece attacks

   for (int f = 0; f < Square_Size; f++) {

      Square from = square_make(f);

      Bit knight = Bit(0);
      Bit bishop = Bit(0);
      Bit rook = Bit(0);
      Bit king = Bit(0);

      for (int dir = 0; dir < 8; dir++) {
         Vec vec = Knight_Vec[dir];
         knight |= ray_1(from, vec);
      }

      bishop |= ray(from, vec_nw);
      bishop |= ray(from, vec_ne);
      bishop |= ray(from, vec_sw);
      bishop |= ray(from, vec_se);

      rook |= ray(from, vec_n);
      rook |= ray(from, vec_w);
      rook |= ray(from, vec_e);
      rook |= ray(from, vec_s);

      for (int dir = 0; dir < 8; dir++) {
         Vec vec = Queen_Vec[dir];
         king |= ray_1(from, vec);
      }

      Pawn_Moves[White][from] = ray_1(from, vec_n);
      Pawn_Moves[Black][from] = ray_1(from, vec_s);

      if (square_rank(from, White) == Rank_2) Pawn_Moves[White][from] |= ray_1(square_front(from, White), vec_n);
      if (square_rank(from, Black) == Rank_2) Pawn_Moves[Black][from] |= ray_1(square_front(from, Black), vec_s);

      Pawn_Attacks[White][from] = ray_1(from, vec_nw) | ray_1(from, vec_ne);
      Pawn_Attacks[Black][from] = ray_1(from, vec_sw) | ray_1(from, vec_se);

      Piece_Attacks[White][Pawn][from] = Pawn_Attacks[White][from];
      Piece_Attacks[Black][Pawn][from] = Pawn_Attacks[Black][from];

      Piece_Attacks[White][Knight][from] = knight;
      Piece_Attacks[Black][Knight][from] = knight;

      Piece_Attacks[White][Bishop][from] = bishop;
      Piece_Attacks[Black][Bishop][from] = bishop;

      Piece_Attacks[White][Rook][from] = rook;
      Piece_Attacks[Black][Rook][from] = rook;

      Piece_Attacks[White][Queen][from] = bishop | rook;
      Piece_Attacks[Black][Queen][from] = bishop | rook;

      Piece_Attacks[White][King][from] = king;
      Piece_Attacks[Black][King][from] = king;
   }

   // range attacks

   for (int f = 0; f < Square_Size; f++) {

      Square from = square_make(f);

      for (int dir = 0; dir < 8; dir++) {

         Vec vec = Queen_Vec[dir];

         Blocker[from] |= ray_almost(from, vec);

         for (Bit b = ray(from, vec); b != 0; b = rest(b)) {

            Square to = first(b);

            Ray    [from][to] = ray(from, vec);
            Beyond [from][to] = ray(to, vec);
            Between[from][to] = ray(from, vec) & ~bit(to) & ~ray(to, vec);
         }
      }
   }

   // slider attacks

#if BMI

   for (int f = 0; f < Square_Size; f++) {

      Square from = square_make(f);

      // bishop

      {
         Bit tos  = piece_attacks(Bishop, from);
         Bit mask = tos & Blocker[from];

         for (int index = 0; index < (1 << count(mask)); index++) {
            Bit blockers = Bit(ml::pdep(uint64(index), mask));
            Bishop_Attacks[from][index] = piece_attacks(from, tos, blockers);
         }
      }

      // rook

      {
         Bit tos  = piece_attacks(Rook, from);
         Bit mask = tos & Blocker[from];

         for (int index = 0; index < (1 << count(mask)); index++) {
            Bit blockers = Bit(ml::pdep(uint64(index), mask));
            Rook_Attacks[from][index] = piece_attacks(from, tos, blockers);
         }
      }
   }

#endif
}

static Bit ray_1(Square from, Vec vec) {

   Bit b = Bit(0);

   Square to = square_add(from, vec);
   if (to != Square_None) set(b, to);

   return b;
}

static Bit ray_almost(Square from, Vec vec) {

   Bit b = Bit(0);

   for (Square sq = square_add(from, vec); sq != Square_None && square_add(sq, vec) != Square_None; sq = square_add(sq, vec)) {
      set(b, sq);
   }

   return b;
}

static Bit ray(Square from, Vec vec) {

   Bit b = Bit(0);

   for (Square sq = square_add(from, vec); sq != Square_None; sq = square_add(sq, vec)) {
      set(b, sq);
   }

   return b;
}

Bit bit(Square sq) {
   return Bit(ml::bit(sq));
}

bool has(Bit b, Square sq) {
   return (b & bit(sq)) != 0;
}

int bit(Bit b, Square sq) {
   return (b >> sq) & 1;
}

bool is_single(Bit b) {
   return b != 0 && rest(b) == 0;
}

bool is_incl(Bit b0, Bit b1) {
   return (b0 & ~b1) == 0;
}

void set(Bit & b, Square sq) {
   b |= bit(sq);
}

void clear(Bit & b, Square sq) {
   b &= ~bit(sq);
}

void flip(Bit & b, Square sq) {
   b ^= bit(sq);
}

Bit add(Bit b, Square sq) {
   assert(!has(b, sq));
   return b ^ bit(sq);
}

Bit remove(Bit b, Square sq) {
   assert(has(b, sq));
   return b ^ bit(sq);
}

Square first(Bit b) {
   assert(b != 0);
   return Square(ml::bit_first(b));
}

Bit rest(Bit b) {
   assert(b != 0);
   return b & (b - 1);
}

int count(Bit b) {
   return ml::bit_count(b);
}

Bit file(File fl) {
   return File_[fl];
}

Bit rank(Rank rk) {
   return Rank_[rk];
}

Bit rank(Rank rk, Side sd) {
   return rank(rank_side(rk, sd));
}

Bit rect(int left, int bottom, int right, int top) {

   if (left   < 0) left   = 0;
   if (bottom < 0) bottom = 0;

   if (right > File_Size) right = File_Size;
   if (top   > Rank_Size) top   = Rank_Size;

   assert(0 <= left   && left   <= right && right <= File_Size);
   assert(0 <= bottom && bottom <= top   && top   <= Rank_Size);

   Bit files = Bit(0);
   Bit ranks = Bit(0);

   for (int fl = left; fl < right; fl++) {
      files |= file(file_make(fl));
   }

   for (int rk = bottom; rk < top; rk++) {
      ranks |= rank(rank_make(rk));
   }

   return files & ranks;
}

Bit ray(Square from, Square to) {
   return Ray[from][to];
}

Bit beyond(Square from, Square to) {
   return Beyond[from][to];
}

Bit between(Square from, Square to) {
   return Between[from][to];
}

Bit line(Square from, Square to) {

   Bit line = between(from, to);
   set(line, from);
   set(line, to);

   return line;
}

Bit pawn_moves(Side sd, Bit froms) {

   if (sd == White) {
      return Bit(froms << Inc_N);
   } else {
      return Bit(froms >> Inc_N);
   }
}

Bit pawn_attacks(Side sd, Bit froms) {

   if (sd == White) {
      return Bit(froms >> Inc_SW) | Bit(froms << Inc_NW);
   } else {
      return Bit(froms << Inc_SW) | Bit(froms >> Inc_NW);
   }
}

Bit pawn_moves_to(Side sd, Bit tos) {
   return pawn_moves(side_opp(sd), tos); // HACK: does not work for double pushes #
}

Bit pawn_attacks_to(Side sd, Bit tos) {
   return pawn_attacks(side_opp(sd), tos);
}

Bit pawn_moves(Side sd, Square from) {
   return Pawn_Moves[sd][from];
}

Bit pawn_attacks(Side sd, Square from) {
   return Pawn_Attacks[sd][from];
}

Bit pawn_attacks_to(Side sd, Square to) {
   return pawn_attacks(side_opp(sd), to);
}

Bit piece_attacks(Piece pc, Square from) {
   return Piece_Attacks[White][pc][from];
}

Bit piece_attacks(Piece pc, Side sd, Square from) {
   return Piece_Attacks[sd][pc][from];
}

Bit piece_attacks(Piece pc, Square from, Bit pieces) {

   assert(pc != Pawn);

#if BMI

   switch (pc) {
      case Bishop : return bit::bishop_attacks(from, pieces);
      case Rook :   return bit::rook_attacks  (from, pieces);
      case Queen :  return bit::queen_attacks (from, pieces);
      default :     return bit::piece_attacks(pc, from);
   }

#else

   return piece_attacks(from, piece_attacks(pc, from), pieces);

#endif
}

static Bit piece_attacks(Square from, Bit tos, Bit pieces) {

   for (Bit b = tos & Blocker[from] & pieces; b != 0; b = rest(b)) {
      Square to = first(b);
      tos &= ~Beyond[from][to];
   }

   return tos;
}

Bit piece_attacks_to(Piece pc, Side sd, Square to) {
   return piece_attacks(pc, side_opp(sd), to);
}

Bit piece_attacks_to(Piece pc, Square to, Bit pieces) {
   assert(pc != Pawn);
   return piece_attacks(pc, to, pieces);
}

bool piece_attack(Piece pc, Side sd, Square from, Square to) {
   return has(piece_attacks(pc, sd, from), to);
}

bool piece_attack(Piece pc, Side sd, Square from, Square to, Bit pieces) {
   return piece_attack(pc, sd, from, to) && line_is_empty(from, to, pieces);
}

bool line_is_empty(Square from, Square to, Bit pieces) {
   return (between(from, to) & pieces) == 0;
}

Bit knight_attacks(Square from) {
   return Piece_Attacks[White][Knight][from];
}

Bit bishop_attacks(Square from, Bit pieces) {

#if BMI

   Bit mask  = Piece_Attacks[White][Bishop][from] & Blocker[from];
   int index = int(ml::pext(mask & pieces, mask));

   return Bishop_Attacks[from][index];

#else

   return piece_attacks(from, piece_attacks(Bishop, from), pieces);

#endif
}

Bit rook_attacks(Square from, Bit pieces) {

#if BMI

   Bit mask  = Piece_Attacks[White][Rook][from] & Blocker[from];
   int index = int(ml::pext(mask & pieces, mask));

   return Rook_Attacks[from][index];

#else

   return piece_attacks(from, piece_attacks(Rook, from), pieces);

#endif
}

Bit queen_attacks(Square from, Bit pieces) {

#if BMI
   return bishop_attacks(from, pieces) | rook_attacks(from, pieces);
#else
   return piece_attacks(from, piece_attacks(Queen, from), pieces);
#endif
}

Bit king_attacks(Square from) {
   return Piece_Attacks[White][King][from];
}

}

