
// includes

#include <cstdio>
#include <cmath>
#include <string>

#include "bit.hpp"
#include "common.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "move.hpp"
#include "pos.hpp"

// functions

void List::clear() {
   p_pair.clear();
}

void List::add_move(Square from, Square to) {
   add(move::make(from, to));
}

void List::add_move(Square from, Square to, Piece prom) {
   add(move::make(from, to, prom));
}

void List::add(Move mv) {
   assert(!(list::has(*this, mv)));
   p_pair.add(Move_Score(mv));
}

void List::add(Move mv, int sc) {
   assert(!(list::has(*this, mv)));
   p_pair.add(Move_Score(mv, sc));
}

void List::set_size(int size) {
   assert(size <= this->size());
   p_pair.set_size(size);
}

void List::set_score(int i, int sc) {
   assert(i >= 0 && i < size());
   p_pair[i].set_score(sc);
}

void List::mtf(int i) {

   assert(i >= 0 && i < size());

   // stable "swap"

   Move_Score pair = p_pair[i];

   for (int j = i; j > 0; j--) {
      p_pair[j] = p_pair[j - 1];
   }

   p_pair[0] = pair;
}

void List::sort() {

   // init

   int size = this->size();
   if (size <= 1) return;

   // insert sort (stable)

   p_pair.add(Move_Score(move::Null, -((1 << 15) - 1))); // HACK: sentinel

   for (int i = size - 2; i >= 0; i--) {

      Move_Score pair = p_pair[i];

      int j;

      for (j = i; pair < p_pair[j + 1]; j++) {
         p_pair[j] = p_pair[j + 1];
      }

      assert(j < size);
      p_pair[j] = pair;
   }

   p_pair.remove(); // sentinel
}

int List::size() const {
   return p_pair.size();
}

Move List::move(int i) const {
   assert(i >= 0 && i < size());
   return p_pair[i].move();
}

int List::score(int i) const {
   assert(i >= 0 && i < size());
   return p_pair[i].score();
}

Move List::operator[](int i) const {
   return move(i);
}

namespace list {

bool has(const List & list, Move mv) {
   return find(list, mv) >= 0;
}

int find(const List & list, Move mv) {

   for (int i = 0; i < list.size(); i++) {
      if (list[i] == mv) return i;
   }

   return -1;
}

}

Move_Score::Move_Score() {
   p_pair = 0;
}

Move_Score::Move_Score(Move mv) : Move_Score(mv, 0) {
}

Move_Score::Move_Score(Move mv, int sc) {

   assert(mv != move::None);
   assert(int(mv) >= 0 && int(mv) < (1 << 15));
   assert(std::abs(sc) < (1 << 15));

   p_pair = (sc << 16) | int(mv);
}

void Move_Score::set_score(int sc) {
   assert(std::abs(sc) < (1 << 15));
   p_pair = (sc << 16) | uint16(p_pair);
}

Move Move_Score::move() const {
   return Move(uint16(p_pair));
}

int Move_Score::score() const {
   return p_pair >> 16;
}

bool operator<(Move_Score m0, Move_Score m1) {
   return m0.p_pair < m1.p_pair;
}

