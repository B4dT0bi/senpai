
#ifndef POS_HPP
#define POS_HPP

// includes

#include "bit.hpp"
#include "common.hpp"
#include "libmy.hpp"

// types

class Pos { // 200 bytes

private :

   const Pos * p_parent;

   Bit p_piece[Piece_Size];
   Bit p_side[Side_Size];
   Bit p_all;
   Side p_turn;

   Square p_ep_sq;
   Bit p_castling_rooks;
   int p_ply;
   int p_rep;

   int8 p_pc[Square_Size];

   Move p_last_move;
   Square p_cap_sq;
   Key p_key_piece;
   Key p_key_pawn;
   Key p_key_full;

public :

   Pos ();
   Pos (Side turn, Bit piece_side[], Bit castling_rooks);

   Pos  succ (Move mv) const;
   Pos  null ()        const;

   Side turn () const { return p_turn; }

   Bit  empties ()                  const { return ~p_all; }
   Bit  pieces  ()                  const { return p_all; }
   Bit  pieces  (Piece pc)          const { return p_piece[pc]; }
   Bit  pieces  (Side sd)           const { return p_side[sd]; }
   Bit  pieces  (Piece pc, Side sd) const { return p_piece[pc] & p_side[sd]; }

   int  count (Piece pc, Side sd) const { return bit::count(pieces(pc, sd)); }

   Bit    pawns     (Side sd) const { return pieces(Pawn, sd); }
   Bit    sliders   (Side sd) const { return pieces(Bishop, sd) | pieces(Rook, sd) | pieces(Queen, sd); }
   Bit    non_pawns (Side sd) const { return pieces(sd) ^ pawns(sd); }
   Bit    non_king  (Side sd) const { return pieces(sd) ^ pawns(sd) ^ pieces(King, sd); }
   Square king      (Side sd) const { return bit::first(pieces(King, sd)); }

   bool is_empty (Square sq)           const { return p_pc[sq] == Piece_None; }
   bool is_piece (Square sq, Piece pc) const { return p_pc[sq] == pc; }
   bool is_side  (Square sq, Side sd)  const { return bit::has(pieces(sd), sq); }

   Piece piece (Square sq) const { return piece_make(p_pc[sq]); }
   Side  side  (Square sq) const { return side_make(bit::bit(pieces(Black), sq)); }

   Bit    castling_rooks (Side sd) const { return p_castling_rooks & pieces(sd); }
   Square ep_sq          ()        const { return p_ep_sq; }

   Move   last_move () const { return p_last_move; }
   Square cap_sq    () const { return p_cap_sq; }
   Key    key       () const { return p_key_full; }
   Key    key_pawn  () const { return p_key_pawn; }

   int  ply () const { return p_ply; }

   bool is_draw () const;

private :

   void clear  ();
   void update ();

   Pos  castle (Move mv) const;

   void switch_turn ();

   void move_piece   (Piece pc, Side sd, Square from, Square to);
   void add_piece    (Piece pc, Side sd, Square sq);
   void remove_piece (Piece pc, Side sd, Square sq);

   bool is_rep () const;
};

namespace pos { // ###

// variables

extern Pos Start;

// functions

void init ();

double phase (const Pos & pos);
int    stage (const Pos & pos);

bool lone_king       (const Pos & pos, Side sd);
bool opposit_bishops (const Pos & pos);

int  force (const Pos & pos, Side sd);

}

#endif // !defined POS_HPP

