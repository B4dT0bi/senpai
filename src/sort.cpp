
// includes

#include "attack.hpp"
#include "common.hpp"
#include "gen.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "search.hpp"
#include "sort.hpp"

// types

class Killer {

private :

   static const int Size = Ply_Size;

   Move p_table[Size];

public :

   void clear ();
   void set   (Move mv, Ply ply);

   Move move (Ply ply) const { return p_table[ply]; }
};

class Counter {

private :

   static const int Size = Move_Index_Size;

   Move p_table[Size];

public :

   void clear ();
   void set   (Move mv, Move_Index last_index);

   Move move (Move_Index last_index) const { return p_table[last_index]; }
};

class History {

private :

   static const int Size = Move_Index_Size;

   static const int Prob_Bit   = 12;
   static const int Prob_One   = 1 << Prob_Bit;
   static const int Prob_Half  = 1 << (Prob_Bit - 1);
   static const int Prob_Shift = 5; // smaller => more adaptive

   int p_table[Size];

public :

   void clear ();

   void good (Move_Index index);
   void bad  (Move_Index index);

   int score (Move_Index index) const { return p_table[index]; }
};

// variables

static Killer G_Killer;
static Counter G_Counter;
static History G_History;

// prototypes

static int capture_score (Move mv, const Pos & pos);

// functions

void sort_clear() {

   G_Killer.clear();
   G_Counter.clear();
   G_History.clear();
}

void good_move(Move mv, const Pos & pos, Ply ply) {

   assert(ply >= 0 && ply < Ply_Size);

   Move_Index index = move::index(mv, pos);
   Move_Index last_index = move::index_last_move(pos);

   G_Killer.set(mv, ply);
   if (last_index != Move_Index_None) G_Counter.set(mv, last_index);
   G_History.good(index);
}

void bad_move(Move mv, const Pos & pos, Ply /* ply */) {
   Move_Index index = move::index(mv, pos);
   G_History.bad(index);
}

void sort_mvv_lva(List & list, const Pos & pos) {

   if (list.size() <= 1) return;

   for (int i = 0; i < list.size(); i++) {

      Move mv = list[i];

      int sc = capture_score(mv, pos);
      list.set_score(i, sc);
   }

   list.sort();
}

void sort_all(List & list, const Pos & pos, Move tt_move, Ply ply) {

   assert(ply >= 0 && ply < Ply_Size);

   if (list.size() <= 1) return;

   Move_Index last_index = move::index_last_move(pos);

   for (int i = 0; i < list.size(); i++) {

      Move mv = list.move(i);
      Move_Index index = move::index(mv, pos);

      int sc;

      if (mv == tt_move) {
         sc = (2 << 12) - 1;
      } else if (move::is_tactical(mv, pos)) {
         sc = (1 << 12);
         sc += capture_score(mv, pos);
         if (!move_is_safe(mv, pos)) sc -= (2 << 12);
      } else if (mv == G_Killer.move(ply)) {
         sc = (1 << 12) - 1;
      } else if (last_index != Move_Index_None && mv == G_Counter.move(last_index)) {
         sc = (1 << 12) - 2;
      } else {
         sc = (0 << 12);
         sc += G_History.score(index);
      }

      assert(std::abs(sc) < (1 << 15));
      list.set_score(i, sc);
   }

   list.sort();
}

void sort_tt_move(List & list, const Pos & /* pos */, Move tt_move) {
   int i = list::find(list, tt_move);
   if (i >= 0) list.mtf(i);
}

static int capture_score(Move mv, const Pos & pos) { // MVV/LVA

   assert(!move::is_castling(mv));

   Square from = move::from(mv);
   Square to   = move::to(mv);

   Piece pc = pos.piece(from);
   Piece cp = pos.is_empty(to) ? Piece_None : pos.piece(to);

   if (move::is_en_passant(mv)) cp = Pawn;

   const int pc_score[Piece_Size]     { 5, 4, 3, 2, 1, 0 };
   const int cp_score[Piece_Size + 1] { 2, 3, 4, 5, 6, 7, 0 };

   int sc = cp_score[cp] * 8 + pc_score[pc];
   assert(sc >= 0 && sc < 8 * 8);

   if (move::is_promotion(mv)) sc += 8; // cp++

   return sc;
}

void Killer::clear() {

   for (int ply = 0; ply < Size; ply++) {
      p_table[ply] = move::None;
   }
}

void Killer::set(Move mv, Ply ply) {
   p_table[ply] = mv;
}

void Counter::clear() {

   for (int last_index = 0; last_index < Size; last_index++) {
      p_table[last_index] = move::None;
   }
}

void Counter::set(Move mv, Move_Index last_index) {
   p_table[last_index] = mv;
}

void History::clear() {

   for (int index = 0; index < Size; index++) {
      p_table[index] = Prob_Half;
   }
}

void History::good(Move_Index index) {
   p_table[index] += (Prob_One - p_table[index]) >> Prob_Shift;
}

void History::bad(Move_Index index) {
   p_table[index] -= p_table[index] >> Prob_Shift;
}

