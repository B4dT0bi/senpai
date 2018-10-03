
#ifndef COMMON_HPP
#define COMMON_HPP

// includes

#include <string>

#include "libmy.hpp"

// constants

const int File_Size { 8 };
const int Rank_Size { 8 };
const int Square_Size { File_Size * Rank_Size };

const int Vector_File_Size { File_Size * 2 - 1 };
const int Vector_Rank_Size { Rank_Size * 2 - 1 };

const int Side_Size { 2 };
const int Piece_Size { 6 };
const int Piece_Size_2 { 1 << 3 }; // for array index
const int Piece_Side_Size { Piece_Size * Side_Size }; // excludes Empty #

const int Move_Index_Size { 1 << 10 };

const int Stage_Size { 24 };

// types

enum Square : int { Square_None = -1 };
enum File   : int { File_A, File_B, File_C, File_D, File_E, File_F, File_G, File_H };
enum Rank   : int { Rank_1, Rank_2, Rank_3, Rank_4, Rank_5, Rank_6, Rank_7, Rank_8 };

enum Vec : int { Vector_Max = (Rank_Size - 1) * Vector_File_Size + (File_Size - 1) };

enum Inc : int {
   Inc_N  = 1,
   Inc_SW = File_Size - 1,
   Inc_W  = File_Size,
   Inc_NW = File_Size + 1,
};

enum Side  : int { White, Black };
enum Piece : int { Pawn, Knight, Bishop, Rook, Queen, King, Piece_None };

enum Piece_Side : int { Empty = Piece_Side_Size };

enum class Key  : uint64;
enum class Move : int;

enum Move_Index : int { Move_Index_None = -1 };

enum Depth : int;
enum Ply   : int;
enum Score : int;

enum class Flag : int {
   None  = 0,
   Upper = 1 << 0,
   Lower = 1 << 1,
   Exact = Upper | Lower,
};

class Bit {

private :

   uint64 p_bit;

public :

   Bit ();
   explicit Bit (uint64 bit);

   operator uint64 () const;

   void operator |= (Bit b);
   void operator &= (uint64 b);
   void operator ^= (Bit b);
};

// operators

inline File operator + (File fl, int inc) { return File(int(fl) + inc); }
inline File operator - (File fl, int inc) { return File(int(fl) - inc); }

inline Rank operator + (Rank rk, int inc) { return Rank(int(rk) + inc); }
inline Rank operator - (Rank rk, int inc) { return Rank(int(rk) - inc); }

inline Vec  operator + (Vec v0, Vec v1) { return Vec(int(v0) + int(v1)); }
inline Vec  operator - (Vec v0, Vec v1) { return Vec(int(v0) - int(v1)); }

inline Inc  operator + (Inc inc) { return Inc(+int(inc)); }
inline Inc  operator - (Inc inc) { return Inc(-int(inc)); }

inline Inc  operator * (Inc inc, int steps) { return Inc(int(inc) * steps); }

inline void operator ^= (Key & k0, Key k1) { k0 = Key(uint64(k0) ^ uint64(k1)); }

inline Depth operator + (Depth d0, Depth d1) { return Depth(int(d0) + int(d1)); }
inline Depth operator - (Depth d0, Depth d1) { return Depth(int(d0) - int(d1)); }

inline Ply  operator + (Ply p0, Ply p1) { return Ply(int(p0) + int(p1)); }
inline Ply  operator - (Ply p0, Ply p1) { return Ply(int(p0) - int(p1)); }

inline Score operator + (Score sc) { return Score(+int(sc)); }
inline Score operator - (Score sc) { return Score(-int(sc)); }

inline Score operator + (Score s0, Score s1) { return Score(int(s0) + int(s1)); }
inline Score operator - (Score s0, Score s1) { return Score(int(s0) - int(s1)); }

inline void operator += (Score & s0, Score s1) { s0 = s0 + s1; }
inline void operator -= (Score & s0, Score s1) { s0 = s0 - s1; }

inline Flag operator | (Flag f0, Flag f1) { return Flag(int(f0) | int(f1)); }

inline void operator |= (Flag & f0, Flag f1) { f0 = f0 | f1; }

inline Bit  operator ~ (Bit b) { return Bit(~uint64(b)); }

inline Bit  operator | (Bit b0, Bit    b1) { return Bit(uint64(b0) | uint64(b1)); }
inline Bit  operator & (Bit b0, uint64 b1) { return Bit(uint64(b0) & b1); }
inline Bit  operator ^ (Bit b0, Bit    b1) { return Bit(uint64(b0) ^ uint64(b1)); }

// functions

bool   square_is_ok (int fl, int rk);
bool   square_is_ok (int sq);
Square square_make  (int fl, int rk);
Square square_make  (int fl, int rk, Side sd);
Square square_make  (int sq);

File square_file (Square sq);
Rank square_rank (Square sq);

Rank square_rank         (Square sq, Side sd);
bool square_is_promotion (Square sq);
int  square_colour       (Square sq);

Inc    square_inc   (Side sd);
Square square_front (Square sq, Side sd);
Square square_rear  (Square sq, Side sd);
Square square_prom  (Square sq, Side sd);

int  square_dist      (Square s0, Square s1);
int  square_dist_file (Square s0, Square s1);
int  square_dist_rank (Square s0, Square s1);

std::string square_to_string   (Square sq);
Square      square_from_string (const std::string & s);

bool file_is_ok (int fl);
bool rank_is_ok (int rk);
File file_make  (int fl);
Rank rank_make  (int rk);

File file_opp  (File fl);
Rank rank_opp  (Rank rk);
Rank rank_side (Rank rk, Side sd);

char file_to_char (File fl);
char rank_to_char (Rank rk);

File file_from_char (char c);
Rank rank_from_char (char c);

Vec    vector_make (int df, int dr);
Square square_add  (Square from, Vec vec);

bool  piece_is_ok (int pc);
Piece piece_make  (int pc);

bool  piece_is_minor (Piece pc);

char  piece_to_char   (Piece pc);
Piece piece_from_char (char c);

bool side_is_ok (int sd);
Side side_make  (int sd);
Side side_opp   (Side sd);

std::string side_to_string (Side sd);

bool       piece_side_is_ok (int ps);
Piece_Side piece_side_make  (int ps);
Piece_Side piece_side_make  (Piece pc, Side sd);
Piece      piece_side_piece (Piece_Side ps);
Side       piece_side_side  (Piece_Side ps);

bool flag_is_lower (Flag flag);
bool flag_is_upper (Flag flag);
bool flag_is_exact (Flag flag);

#endif // !defined COMMON_HPP

