
#ifndef ATTACK_HPP
#define ATTACK_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

class Pos;

// types

class Attack_Info {

private :

   Bit p_piece_attacks[Square_Size];

   Bit p_attacks[Side_Size];
   Bit p_support[Side_Size];

   Bit p_le_pieces [Side_Size][Piece_Size_2];
   Bit p_le_attacks[Side_Size][Piece_Size_2];

public :

   void init (const Pos & pos);

   Bit piece_attacks (Square sq) const;

   Bit attacks       (Side sd) const;
   Bit pawn_attacks  (Side sd) const;
   Bit queen_attacks (Side sd) const;
   Bit queen_safe    (Side sd) const;
};

// functions

bool is_mate      (const Pos & pos);
bool is_stalemate (const Pos & pos);

bool is_legal (const Pos & pos);
bool in_check (const Pos & pos);
Bit  checks   (const Pos & pos);

bool move_is_safe (Move mv, const Pos & pos);
bool move_is_win  (Move mv, const Pos & pos);

Score see     (Move mv, const Pos & pos);
Score see_max (Move mv, const Pos & pos); // optimistic gain for delta pruning

bool has_attack (const Pos & pos, Side sd, Square to);
bool has_attack (const Pos & pos, Side sd, Square to, Bit pieces);

Bit  attacks_to        (const Pos & pos, Side sd, Square to, Bit pieces);
Bit  pseudo_attacks_to (const Pos & pos, Side sd, Square to);

bool   is_pinned (const Pos & pos, Square king, Square sq, Side sd);
Square pinned_by (const Pos & pos, Square king, Square sq, Side sd);
Bit    pins      (const Pos & pos, Square king);

#endif // !defined ATTACK_HPP

