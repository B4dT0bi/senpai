
#ifndef LIST_HPP
#define LIST_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

class Pos;

// types

class Move_Score {

private :

   int p_pair;

public :

   Move_Score ();
   explicit Move_Score (Move mv);
   Move_Score (Move mv, int sc);

   friend bool operator < (Move_Score m0, Move_Score m1);

   void set_score (int sc);

   Move move  () const;
   int  score () const;
};

class List {

private :

   static const int Size = 256;

   ml::Array<Move_Score, Size> p_pair;

public :

   void clear    ();
   void add_move (Square from, Square to);
   void add_move (Square from, Square to, Piece prom);

   void add (Move mv);
   void add (Move mv, int sc);

   void set_size  (int size);
   void set_score (int i, int sc);

   void mtf  (int i); // move to front
   void sort ();

   int  size  ()      const;
   Move move  (int i) const;
   int  score (int i) const;

   Move operator [] (int i) const;
};

// functions

namespace list {

bool has  (const List & list, Move mv);
int  find (const List & list, Move mv);

}

#endif // !defined LIST_HPP

