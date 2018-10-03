
#ifndef GEN_HPP
#define GEN_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

class List;
class Pos;

// functions

void gen_legals   (List & list, const Pos & pos);
void gen_moves    (List & list, const Pos & pos);
void gen_moves    (List & list, const Pos & pos, Bit checks);
void gen_evasions (List & list, const Pos & pos, Bit checks);
void gen_eva_caps (List & list, const Pos & pos, Bit checks);
void gen_captures (List & list, const Pos & pos);

void add_promotions (List & list, const Pos & pos);
void add_checks     (List & list, const Pos & pos);

void gen_captures   (List & list, const Pos & pos, Side sd);
void add_promotions (List & list, const Pos & pos, Side sd);

#endif // !defined GEN_HPP

