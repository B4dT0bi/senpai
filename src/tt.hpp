
#ifndef TT_HPP
#define TT_HPP

// includes

#include <vector>

#include "common.hpp"
#include "libmy.hpp"

namespace tt {

// types

struct Info {
   Move move;
   Score score;
   Flag flag;
   Depth depth;
   Score eval;
};

class TT {

private :

   static const int Date_Size = 16;

   struct Entry { // 16 bytes
      uint32 lock;
      int16 move;
      int16 score;
      int16 eval;
      uint16 pad_2; // #
      int8 depth;
      uint8 date;
      uint8 flag;
      uint8 pad_1; // #
   };

   std::vector<Entry> p_table;

   int p_size;
   int p_mask;
   int p_date;
   int p_age[Date_Size];

public :

   TT ();

   void set_size (int size);

   void clear    ();
   void inc_date ();

   void store (Key key, const Info & info);
   bool probe (Key key, Info & info);

private :

   void set_date (int date);
   int  age      (int date) const;
};

// variables

extern TT G_TT; // MOVE ME?

}

#endif // !defined TT_HPP

