
// includes

#include <cctype>
#include <string>

#include "bit.hpp"
#include "common.hpp"
#include "fen.hpp"
#include "libmy.hpp"
#include "pos.hpp"
#include "util.hpp"
#include "var.hpp"

// constants

const std::string Piece_Side_Char { "PpNnBbRrQqKk" };
const std::string Side_Char       { "wb" };

// prototypes

static Square fen_square (int sq);

// functions

Pos pos_from_fen(const std::string & s) {

   int i = 0;

   // pieces

   if (s[i] == ' ') i++; // HACK to help parsing

   Bit piece_side[Piece_Side_Size];

   for (int ps = 0; ps < Piece_Side_Size; ps++) {
      piece_side[ps] = Bit(0);
   }

   int sq = 0;
   int run = 0;

   while (true) {

      char c = s[i++];
      if (c == '\0' || c == ' ') break;

      if (c == '/') {

         sq += run;
         run = 0;

         if (sq >= Square_Size) throw Bad_Input();

      } else if (std::isdigit(c)) { // run of empty squares

         run = run * 10 + (c - '0');

      } else { // piece

         sq += run;
         run = 0;

         if (sq >= Square_Size) throw Bad_Input();

         Piece_Side ps = Piece_Side(find(c, Piece_Side_Char));
         bit::set(piece_side[ps], fen_square(sq));
         sq += 1;
      }
   }

   if (sq + run != Square_Size) throw Bad_Input();

   // turn

   if (s[i] == ' ') i++;

   Side turn = White;
   if (s[i] != '\0') turn = side_make(find(s[i++], Side_Char));

   // castling rights

   if (s[i] == ' ') i++;

   Bit castling_rooks = Bit(0);

   while (s[i] != '\0' && s[i] != ' ') {

      char c = s[i++];
      if (c == '-') continue;

      Side sd;

      if (std::isupper(c)) {

         sd = White;

         if (c == 'K') c = 'H';
         if (c == 'Q') c = 'A';

      } else {

         sd = Black;

         if (c == 'k') c = 'h';
         if (c == 'q') c = 'a';
      }

      bit::set(castling_rooks, square_make(file_from_char(std::tolower(c)), rank_side(Rank_1, sd)));
   }

   // wrap up

   return Pos(turn, piece_side, castling_rooks);
}

static Square fen_square(int sq) {
   int fl = sq % 8;
   int rk = sq / 8;
   return square_make(fl, 7 - rk);
}

