
// includes

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "bit.hpp"
#include "common.hpp"
#include "eval.hpp"
#include "fen.hpp"
#include "game.hpp"
#include "gen.hpp"
#include "hash.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "math.hpp"
#include "move.hpp"
#include "pawn.hpp"
#include "pos.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "thread.hpp"
#include "tt.hpp"
#include "util.hpp"
#include "var.hpp"

// constants

const std::string Engine_Name    { "Senpai" };
const std::string Engine_Version { "2.0" };

// prototypes

static void uci_loop ();

// functions

int main(int argc, char * argv[]) {

   std::string arg = "";
   if (argc > 1) arg = argv[1];

   math::init();
   bit::init();
   hash::init();
   pawn::init();
   pos::init();
   var::init();

   listen_input();

   var::update();

   uci_loop();

   return EXIT_SUCCESS;
}

static void uci_loop() {

   Game game;
   game.clear();

   Search_Input si;
   si.init();

   bool init_done = false;

   while (true) {

      std::string line;
      if (!get_line(line)) { // EOF
         std::exit(EXIT_SUCCESS);
      }

      if (line.empty()) continue;

      std::stringstream ss(line);

      std::string command;
      ss >> command;

      if (false) {

      } else if (command == "uci") {

         std::cout << "id name " << Engine_Name + " " + Engine_Version << std::endl;
         std::cout << "id author " << "Fabien Letouzey" << std::endl;

         std::cout << "option name " << "Hash" << " type spin default " << var::get("Hash") << " min 1 max 16384" << std::endl;
         std::cout << "option name " << "Ponder" << " type check default " << var::get("Ponder") << std::endl;
         std::cout << "option name " << "Threads" << " type spin default " << var::get("Threads") << " min 1 max 16" << std::endl;
         std::cout << "option name " << "UCI_Chess960" << " type check default " << var::get("UCI_Chess960") << std::endl;

         std::cout << "option name " << "Clear Hash" << " type button" << std::endl;

         std::cout << "uciok" << std::endl;

      } else if (command == "isready") {

         if (!init_done) {

            var::update();

            clear_pawn_table();
            tt::G_TT.set_size(int64(var::Hash) << (20 - 4)); // * 1MiB / 16 bytes

            init_done = true;
         }

         std::cout << "readyok" << std::endl;

      } else if (command == "setoption") {

         std::string name;
         std::string value;

         bool parsing_name  = false;
         bool parsing_value = false;

         std::string arg;

         while (ss >> arg) {

            if (false) {

            } else if (arg == "name") {

               name = "";

               parsing_name  = true;
               parsing_value = false;

            } else if (arg == "value") {

               value = "";

               parsing_name  = false;
               parsing_value = true;

            } else if (parsing_name) {

               if (!name.empty()) name += " ";
               name += arg;

            } else if (parsing_value) {

               if (!value.empty()) value += " ";
               value += arg;
            }
         }

         if (name == "Clear Hash") {
            tt::G_TT.clear();
         } else {
            var::set(name, value);
            var::update();
         }

      } else if (command == "ucinewgame") {

         tt::G_TT.clear();

      } else if (command == "position") {

         std::string fen = Start_FEN;
         std::string moves;

         bool parsing_fen   = false;
         bool parsing_moves = false;

         std::string arg;

         while (ss >> arg) {

            if (false) {

            } else if (arg == "startpos") {

               fen = Start_FEN;

               parsing_fen   = false;
               parsing_moves = false;

            } else if (arg == "fen") {

               fen = "";

               parsing_fen   = true;
               parsing_moves = false;

            } else if (arg == "moves") {

               moves = "";

               parsing_fen   = false;
               parsing_moves = true;

            } else if (parsing_fen) {

               if (!fen.empty()) fen += " ";
               fen += arg;

            } else if (parsing_moves) {

               if (!moves.empty()) moves += " ";
               moves += arg;
            }
         }

         game.init(pos_from_fen(fen));

         std::stringstream ss(moves);

         while (ss >> arg) {
            game.add_move(move::from_uci(arg, game.pos()));
         }

         si.init(); // reset level

      } else if (command == "go") {

         int depth = -1;
         double move_time = -1.0;

         bool smart = false;
         int moves = 0;
         double game_time = 30.0;
         double inc = 0.0;

         bool ponder = false;
         bool analyze = false;

         std::string arg;

         while (ss >> arg) {

            if (false) {
            } else if (arg == "depth") {
               ss >> arg;
               depth = std::stoi(arg);
            } else if (arg == "movetime") {
               ss >> arg;
               move_time = std::stod(arg) / 1000.0;
            } else if (arg == "movestogo") {
               smart = true;
               ss >> arg;
               moves = std::stoi(arg);
            } else if (arg == (game.turn() == White ? "wtime" : "btime")) {
               smart = true;
               ss >> arg;
               game_time = std::stod(arg) / 1000.0;
            } else if (arg == (game.turn() == White ? "winc" : "binc")) {
               smart = true;
               ss >> arg;
               inc = std::stod(arg) / 1000.0;
            } else if (arg == "ponder") {
               ponder = true;
            } else if (arg == "infinite") {
               analyze = true;
            }
         }

         if (depth >= 0) si.depth = Depth(depth);
         if (move_time >= 0.0) si.time = move_time;

         if (smart) si.set_time(moves, game_time - inc, inc); // GUIs add the increment only after the move :(

         si.move = !analyze;
         si.ponder = ponder;

         Search_Output so;
         search(so, game.pos(), si);

         Move move = so.move;
         Move answer = so.answer;

         if (move == move::None) {
            move = quick_move(game.pos());
         }

         if (move != move::None && answer == move::None) {
            answer = quick_move(game.pos().succ(move));
         }

         std::cout << "bestmove " << move::to_uci(move, game.pos());
         if (answer != move::None) std::cout << " ponder " << move::to_uci(answer, game.pos().succ(move));
         std::cout << std::endl;

         si.init(); // reset level

      } else if (command == "stop") {

         // no-op (handled during search)

      } else if (command == "ponderhit") {

         // no-op (handled during search)

      } else if (command == "quit") {

         std::exit(EXIT_SUCCESS);
      }
   }
}

