
// includes

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "bit.hpp"
#include "common.hpp"
#include "libmy.hpp"
#include "util.hpp"

// constants

const std::string Piece_Char { "PNBRQK" };

// functions

bool square_is_ok(int fl, int rk) {
   return file_is_ok(fl) && rank_is_ok(rk);
}

bool square_is_ok(int sq) {
   return sq >= 0 && sq < Square_Size;
}

Square square_make(int fl, int rk) {
   assert(square_is_ok(fl, rk));
   return square_make((fl << 3) | rk); // file major for pawns
}

Square square_make(int fl, int rk, Side sd) {
   assert(square_is_ok(fl, rk));
   return square_make(fl, rank_side(Rank(rk), sd));
}

Square square_make(int sq) {
   assert(square_is_ok(sq));
   return Square(sq);
}

File square_file(Square sq) {
   return File(sq >> 3);
}

Rank square_rank(Square sq) {
   return Rank(sq & 7);
}

Rank square_rank(Square sq, Side sd) {
   return rank_side(square_rank(sq), sd);
}

bool square_is_promotion(Square sq) {
   return ((square_rank(sq) + 1) & 7) < 2;
}

int square_colour(Square sq) {
   return (square_file(sq) ^ square_rank(sq)) & 1;
}

Inc square_inc(Side sd) { // not used externally
   return Inc(1 - sd * 2);
}

Square square_front(Square sq, Side sd) {
   return square_make(sq + square_inc(sd));
}

Square square_rear(Square sq, Side sd) {
   return square_make(sq - square_inc(sd));
}

Square square_prom(Square sq, Side sd) {
   return square_make(square_file(sq), Rank_8, sd);
}

int square_dist(Square s0, Square s1) {
   return std::max(square_dist_file(s0, s1), square_dist_rank(s0, s1));
}

int square_dist_file(Square s0, Square s1) {
   return std::abs(square_file(s0) - square_file(s1));
}

int square_dist_rank(Square s0, Square s1) {
   return std::abs(square_rank(s0) - square_rank(s1));
}

std::string square_to_string(Square sq) {

   std::string s;
   s += file_to_char(square_file(sq));
   s += rank_to_char(square_rank(sq));

   return s;
}

bool file_is_ok(int fl) {
   return fl >= 0 && fl < File_Size;
}

bool rank_is_ok(int rk) {
   return rk >= 0 && rk < Rank_Size;
}

File file_make(int fl) {
   assert(file_is_ok(fl));
   return File(fl);
}

Rank rank_make(int rk) {
   assert(rank_is_ok(rk));
   return Rank(rk);
}

File file_opp(File fl) {
   return File(fl ^ 7);
}

Rank rank_opp(Rank rk) {
   return Rank(rk ^ 7);
}

Rank rank_side(Rank rk, Side sd) {
   return Rank((rk ^ -sd) & 7);
}

char file_to_char(File fl) {
   return 'a' + fl;
}

char rank_to_char(Rank rk) {
   return '1' + rk;
}

File file_from_char(char c) {

   int fl = c - 'a';
   if (!file_is_ok(fl)) throw Bad_Input();

   return File(fl);
}

Rank rank_from_char(char c) {

   int rk = c - '1';
   if (!rank_is_ok(rk)) throw Bad_Input();

   return Rank(rk);
}

Square square_from_string(const std::string & s) {

   if (s.size() != 2) throw Bad_Input();

   File fl = file_from_char(s[0]);
   Rank rk = rank_from_char(s[1]);

   return square_make(fl, rk);
}

Vec vector_make(int df, int dr) {

   assert(std::abs(df) < File_Size);
   assert(std::abs(dr) < Rank_Size);

   return Vec((dr + (Rank_Size - 1)) * Vector_File_Size + (df + (File_Size - 1)));
}

Square square_add(Square from, Vec vec) {

   Vec to = vector_make(square_file(from), square_rank(from)) + vec - Vector_Max;

   int df = to % Vector_File_Size - (File_Size - 1);
   int dr = to / Vector_File_Size - (Rank_Size - 1);

   return !square_is_ok(df, dr) ? Square_None : square_make(df, dr);
}

bool piece_is_ok(int pc) {
   return pc >= 0 && pc < Piece_Size;
}

Piece piece_make(int pc) {
   assert(piece_is_ok(pc));
   return Piece(pc);
}

bool piece_is_minor(Piece pc) {
   return pc == Knight || pc == Bishop;
}

char piece_to_char(Piece pc) {
   assert(pc != Piece_None);
   return Piece_Char[pc];
}

Piece piece_from_char(char c) {
   return piece_make(find(c, Piece_Char));
}

bool side_is_ok(int sd) {
   return sd >= 0 && sd < Side_Size;
}

Side side_make(int sd) {
   assert(side_is_ok(sd));
   return Side(sd);
}

Side side_opp(Side sd) {
   return Side(sd ^ 1);
}

std::string side_to_string(Side sd) {
   return (sd == White) ? "white" : "black";
}

bool piece_side_is_ok(int ps) { // excludes Empty
   return ps >= 0 && ps < Piece_Side_Size;
}

Piece_Side piece_side_make(int ps) {
   assert(piece_side_is_ok(ps));
   return Piece_Side(ps);
}

Piece_Side piece_side_make(Piece pc, Side sd) {
   assert(pc != Piece_None);
   return piece_side_make((pc << 1) | sd);
}

Piece piece_side_piece(Piece_Side ps) {
   assert(ps != Empty);
   return piece_make(ps >> 1);
}

Side piece_side_side(Piece_Side ps) {
   assert(ps != Empty);
   return side_make(ps & 1);
}

bool flag_is_lower(Flag flag) {
   return (int(flag) & int(Flag::Lower)) != 0;
}

bool flag_is_upper(Flag flag) {
   return (int(flag) & int(Flag::Upper)) != 0;
}

bool flag_is_exact(Flag flag) {
   return flag == Flag::Exact;
}

Bit::Bit() {
   p_bit = 0;
}

Bit::Bit(uint64 bit) {
   p_bit = bit;
}

Bit::operator uint64() const {
   return p_bit;
}

void Bit::operator|=(Bit b) {
   p_bit |= uint64(b);
}

void Bit::operator&=(uint64 b) {
   p_bit &= b;
}

void Bit::operator^=(Bit b) {
   p_bit ^= uint64(b);
}

