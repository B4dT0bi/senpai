
#ifndef GAME_HPP
#define GAME_HPP

// includes

#include <string>

#include "common.hpp"
#include "libmy.hpp"
#include "pos.hpp"

// types

class Game {

private :

   static const int Size = 1024;

   Pos p_pos_start;

   ml::Array<Pos,  Size> p_pos;
   ml::Array<Move, Size> p_move;

public :

   Game ();

   void clear    ();
   void init     (const Pos & pos);
   void add_move (Move mv);

   Side turn () const;

   const Pos & pos () const;
};

#endif // !defined GAME_HPP

