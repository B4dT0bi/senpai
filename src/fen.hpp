
#ifndef FEN_HPP
#define FEN_HPP

// includes

#include <string>

#include "common.hpp"
#include "libmy.hpp"

class Pos;

// constants

const std::string Start_FEN { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };

// functions

Pos pos_from_fen (const std::string & s);

#endif // !defined FEN_HPP

