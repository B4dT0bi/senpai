
#ifndef SEARCH_HPP
#define SEARCH_HPP

// includes

#include <string>

#include "common.hpp"
#include "libmy.hpp"
#include "pos.hpp"
#include "score.hpp"
#include "util.hpp"

class List;
class Pos;

// constants

const Depth Depth_Max { Depth(64) };

const Ply Ply_Root { Ply(0) };
const Ply Ply_Max  { Ply(63) };
const int Ply_Size { Ply_Max + 1 };

// types

class Line {

private :

   ml::Array<Move, Ply_Size> p_move;

public :

   Line ();

   void clear ();
   void add   (Move mv);

   void set    (Move mv);
   void concat (Move mv, const Line & pv);

   int  size ()      const;
   Move move (int i) const;

   Move operator [] (int i) const;

   std::string to_uci (const Pos & pos) const;
};

class Search_Input {

public :

   bool move;
   Depth depth;

   bool smart;
   int moves;
   double time;
   double inc;
   bool ponder;

public :

   Search_Input ();

   void init ();

   void set_time (int moves, double time, double inc);
};

class Search_Output {

public :

   Move move;
   Move answer;
   Score score;
   Flag flag;
   Depth depth;
   Line pv;

   int64 node;
   int ply_max;

private :

   const Search_Input * p_si;
   Pos p_pos;
   mutable Timer p_timer;

public :

   void init (const Search_Input & si, const Pos & pos);
   void end  ();

   void start_iter (Depth depth);
   void end_iter   ();

   void new_best_move (Move mv, Score sc = score::None);
   void new_best_move (Move mv, Score sc, Flag flag, Depth depth, const Line & pv);

   void disp_best_move ();

   double time () const;
};

// functions

void search (Search_Output & so, const Pos & pos, const Search_Input & si);

Move  quick_move  (const Pos & pos);
Score quick_score (const Pos & pos);

#endif // !defined SEARCH_HPP

