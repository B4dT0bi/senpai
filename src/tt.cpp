
// includes

#include <cstdio>
#include <cstring>
#include <vector>

#include "common.hpp"
#include "hash.hpp"
#include "libmy.hpp"
#include "move.hpp"
#include "score.hpp"
#include "tt.hpp"

namespace tt {

// constants

const int Cluster_Size { 4 };

const int Depth_Min { -1 };

// variables

TT G_TT;

// functions

TT::TT() {
}

void TT::set_size(int size) {

   assert(ml::is_power_2(size));

   p_size = size;
   p_mask = (size - 1) & -Cluster_Size;

   p_table.resize(p_size);

   clear();
}

void TT::clear() {

   assert(sizeof(Entry) == 16);

   Entry entry { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

   entry.move = int(move::None);
   entry.score = score::None;
   entry.depth = Depth_Min;
   entry.eval = score::None;

   for (int i = 0; i < p_size; i++) {
      p_table[i] = entry;
   }

   set_date(0);
}

void TT::inc_date() {
   set_date((p_date + 1) % Date_Size);
}

void TT::set_date(int date) {

   assert(date >= 0 && date < Date_Size);

   p_date = date;

   for (date = 0; date < Date_Size; date++) {
      p_age[date] = age(date);
   }
}

int TT::age(int date) const {

   assert(date >= 0 && date < Date_Size);

   int age = p_date - date;
   if (age < 0) age += Date_Size;

   assert(age >= 0 && age < Date_Size);
   return age;
}

void TT::store(Key key, const Info & info) {

   assert(info.move != move::Null);
   assert(int(info.move) > -(1 << 15) && int(info.move) < +(1 << 15));
   assert(info.score != score::None);
   assert(info.score > -(1 << 15) && info.score < +(1 << 15));
   assert(info.depth > Depth_Min && info.depth < (1 << 7));

   // probe

   int    index = hash::index(key, p_mask);
   uint32 lock  = hash::lock(key);

   Entry * be = nullptr;
   int bs = -64;

   for (int i = 0; i < Cluster_Size; i++) {

      assert(index + i < p_size);
      Entry & entry = p_table[index + i];

      if (entry.lock == lock) { // hash hit

         if (entry.depth <= info.depth) {

            assert(entry.lock == lock);
            entry.date = p_date;
            if (info.move != move::None) entry.move = int(info.move);
            entry.score = info.score;
            entry.flag = int(info.flag);
            entry.depth = info.depth;
            if (info.eval != score::None) entry.eval = info.eval;

         } else { // deeper entry

            entry.date = p_date;
         }

         return;
      }

      // evaluate replacement score

      int sc = 0;
      sc = sc * Date_Size + p_age[entry.date];
      sc = sc * 64 - entry.depth;
      assert(sc > -64);

      if (sc > bs) {
         be = &entry;
         bs = sc;
      }
   }

   // "best" entry found

   assert(be != nullptr);
   Entry & entry = *be;
   // assert(entry.lock != lock); // triggers in SMP

   // store

   entry.lock = lock;
   entry.date = p_date;
   entry.move = int(info.move);
   entry.score = info.score;
   entry.flag = int(info.flag);
   entry.depth = info.depth;
   entry.eval = info.eval;
}

bool TT::probe(Key key, Info & info) {

   // init

   // probe

   int    index = hash::index(key, p_mask);
   uint32 lock  = hash::lock(key);

   for (int i = 0; i < Cluster_Size; i++) {

      assert(index + i < p_size);
      const Entry & entry = p_table[index + i];

      if (entry.lock == lock) {

         // found

         info.move = Move(entry.move);
         info.score = Score(entry.score);
         info.flag = Flag(entry.flag);
         info.depth = Depth(entry.depth);
         info.eval = Score(entry.eval);

         return true;
      }
   }

   // not found

   info.move = move::None;
   return false;
}

}

