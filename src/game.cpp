
// includes

#include <cstdlib>
#include <iostream>
#include <string>

#include "attack.hpp"
#include "common.hpp"
#include "game.hpp"
#include "libmy.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "score.hpp"

// functions

Game::Game() {
   clear();
}

void Game::clear() {
   init(pos::Start);
}

void Game::init(const Pos & pos) {

   p_pos_start = pos;

   p_move.clear();
   p_pos.clear();

   p_pos.add_ref(p_pos_start);
}

void Game::add_move(Move mv) {

   assert(mv != move::None);

   p_move.add(mv);
   p_pos.add_ref(pos().succ(mv));
}

Side Game::turn() const {
   return pos().turn();
}

const Pos & Game::pos() const {
   assert(p_pos.size() > 0);
   return p_pos[p_pos.size() - 1];
}

