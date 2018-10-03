
// includes

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "attack.hpp"
#include "bit.hpp"
#include "common.hpp"
#include "eval.hpp"
#include "gen.hpp"
#include "hash.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "math.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "score.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "thread.hpp"
#include "tt.hpp"
#include "var.hpp"

// types

enum ID : int { ID_Main = 0 };

class Time {

private :

   double p_time_0; // target
   double p_time_1; // extended
   double p_time_2; // maximum

public :

   void init (const Search_Input & si, const Pos & pos);

   double time_0 () const { return p_time_0; }
   double time_1 () const { return p_time_1; }
   double time_2 () const { return p_time_2; }

private :

   void init (double time);
   void init (int moves, double time, double inc, const Pos & pos);
};

struct Node {

   const Pos * p_pos; // HACK: should be private

   Score alpha;
   Score beta;
   Depth depth;
   Ply ply;
   bool root;
   bool pv_node;
   Bit checks;
   bool in_check;
   Score eval;
   Move skip_move;
   Move sing_move;
   Score sing_score;
   bool futile;

   List list;
   List searched;
   int i;
   int j;

   Move move;
   Score score;
   Line pv;

   const Pos & pos () const { return *p_pos; }
};

class Search_Global;
class Search_Local;

class Split_Point : public Lockable {

private :

   Split_Point * p_parent;
   Search_Global * p_sg;

   Node p_node;

   std::atomic<uint64> p_workers;
   std::atomic<bool> p_stop;

public :

   void init_root  (int master);
   void init       (int master, Split_Point * parent, Search_Global & sg, const Node & node);
   void get_result (Node & node);

   void enter (ID id);
   void leave (ID id);

   Move get_move (Node & node);
   void update   (Move mv, Score sc, const Line & pv);

   void stop_root ();

   bool is_child (Split_Point * sp);

   bool stop () const { return p_stop; }
   bool free () const { return p_workers == 0; }

   Split_Point * parent () const { return p_parent; }
   const Node  & node   () const { return p_node; }
};

class Search_Local : public Lockable {

private :

   static const int Pool_Size = 10;

   std::thread p_thread;
   ID p_id;

   std::atomic<Split_Point *> p_work;
   ml::Array<Split_Point *, Ply_Size> p_stack;
   Split_Point p_pool[Pool_Size];
   std::atomic<int> p_pool_size;

   Search_Global * p_sg;

   int64 p_node;
   int p_ply_max;

public :

   void init (ID id, Search_Global & sg);
   void end  ();

   void start_iter ();
   void end_iter   (Search_Output & so);

   void search_all_try  (const Pos & pos, List & list, Depth depth);
   void search_root_try (const Pos & pos, const List & list, Depth depth);

   void give_work (Split_Point * sp);

   bool idle (Split_Point * parent) const;
   bool idle () const;

private :

   static void launch (Search_Local * sl, Split_Point * root_sp);

   void idle_loop (Split_Point * wait_sp);

   void join      (Split_Point * sp);
   void move_loop (Split_Point * sp);

   void  search_all  (const Pos & pos, List & list, Depth depth, Ply ply);
   void  search_asp  (const Pos & pos, const List & list, Depth depth, Ply ply);
   void  search_root (const Pos & pos, const List & list, Score alpha, Score beta, Depth depth, Ply ply);
   Score search      (const Pos & pos, Score alpha, Score beta, Depth depth, Ply ply, Move skip_move, Line & pv);
   Score qs          (const Pos & pos, Score alpha, Score beta, Depth depth, Ply ply, Line & pv);
   Score snmp        (const Pos & pos, Score beta, Score eval);

   void  move_loop   (Node & node);
   Score search_move (Move mv, const Node & node, Line & pv);

   void split (Node & node);

   static void gen_tacticals (List & list, const Pos & pos, Bit checks);

   static bool  prune  (Move mv, const Node & node);
   static Depth extend (Move mv, const Node & node);
   static Depth reduce (Move mv, const Node & node);

   static bool move_is_dangerous (Move mv, const Node & node);

   static bool null_bad (const Pos & pos, Side sd);

   void inc_node ();

   Score leaf      (Score sc, Ply ply);
   void  mark_leaf (Ply ply);

   Score eval     (const Pos & pos);
   Key   hash_key (const Pos & pos);

   void poll ();
   bool stop () const;

   void push_sp (Split_Point * sp);
   void pop_sp  (Split_Point * sp);

   Split_Point * top_sp () const;
};

class Search_Global : public Lockable {

private :

   const Search_Input * p_si;
   Search_Output * p_so;

   const Pos * p_pos;
   List p_list;

   Search_Local p_sl[16];

   Split_Point p_root_sp;

   std::atomic<bool> p_ponder;
   std::atomic<bool> p_flag;

   Move p_last_move;
   Score p_last_score;

   std::atomic<bool> p_change;
   bool p_first;
   std::atomic<int> p_high;
   bool p_drop;
   double p_factor;

   Depth p_depth;
   Move p_current_move;
   int p_current_number;

   double p_last_poll;

public :

   void init (const Search_Input & si, Search_Output & so, const Pos & pos, const List & list);
   void end  ();

   void search        (Depth depth);
   void collect_stats ();

   void new_best_move (Move mv, Score sc, Flag flag, Depth depth, const Line & pv, bool fail_low);

   void poll  ();
   void abort ();

   void disp_info (bool disp_move);

   bool has_worker () const;
   void broadcast  (Split_Point * sp);

   const Pos & pos () const { return *p_pos; }
   List & list () { return p_list; } // HACK

   Split_Point * root_sp () { return &p_root_sp; }

   void search_move (Move mv, int searched_size);

   void set_flag   () { p_flag = true; }
   void clear_flag () { p_flag = false; p_change = true; }

   void set_high   () { p_high += 1; clear_flag(); }
   void clear_high () { p_high -= 1; }

   Score  score () const { return p_so->score; }
   Depth  depth () const { return p_so->depth; }
   double time  () const { return p_so->time(); }

   bool ponder () const { return p_ponder; }

   Move  last_move  () const { return p_last_move; }
   Score last_score () const { return p_last_score; }

   bool   change () const { return p_change; }
   bool   first  () const { return p_first; }
   bool   high   () const { return p_high != 0; }
   bool   drop   () const { return p_drop; }
   double factor () const { return p_factor; }

   tt::TT & tt () const { return tt::G_TT; }

   const Search_Local & sl (ID id) const { return p_sl[id]; }
         Search_Local & sl (ID id)       { return p_sl[id]; }
};

struct SMP : public Lockable {
   std::atomic<bool> busy;
};

class Abort : public std::exception {
};

// variables

static int LMR_Red[32][64];

static Time G_Time;

static SMP G_SMP; // lock to create and broadcast split points
static Lockable G_IO;

// prototypes

static double alloc_moves (const Pos & pos);
static double alloc_early (const Pos & pos);

static double lerp (double mg, double eg, double phase);

static double time_lag (double time);

static void node_update (Node & node, Move mv, Score sc, const Line & pv, Search_Global & sg);

static Flag flag (Score sc, Score alpha, Score beta);

// functions

void search(Search_Output & so, const Pos & pos, const Search_Input & si) {

   for (int d = 1; d < 32; d++) {
      for (int l = 1; l < 64; l++) {
         LMR_Red[d][l] = int(math::log_2(l) * math::log_2(d) * 0.4);
      }
   }

   // init

   var::update();

   so.init(si, pos);

   // special cases

   List list;
   gen_legals(list, pos);
   assert(list.size() != 0);

   if (si.move && !si.ponder && list.size() == 1) {

      Move mv = list[0];
      Score sc = quick_score(pos);

      so.new_best_move(mv, sc);
      return;
   }

   // more init

   G_Time.init(si, pos);

   Search_Global sg;
   sg.init(si, so, pos, list); // also launches threads

   Move easy_move = move::None;

   if (si.smart && list.size() > 1) {

      List & list = sg.list();

      sg.sl(ID_Main).search_all_try(pos, list, Depth(1));

      if (list.score(0) - list.score(1) >= +200 && list[0] == quick_move(pos)) {
         easy_move = list[0];
      }
   }

   // iterative deepening

   try {

      for (int d = 1; d <= si.depth; d++) {

         Depth depth = Depth(d);

         sg.search(depth);
         sg.collect_stats();

         Move mv = so.move;
         double time = so.time();

         // early exit?

         bool abort = false;

         if (mv == easy_move && !sg.change() && time >= G_Time.time_0() / 16.0) abort = true;

         if (si.smart && time >= G_Time.time_0() * sg.factor() * alloc_early(pos)) abort = true;

         if (si.smart && sg.drop()) abort = false;

         if (abort) {
            sg.set_flag();
            if (!sg.ponder()) break;
         }
      }

   } catch (const Abort &) {

      // no-op
   }

   sg.disp_info(false);
   sg.end(); // sync with threads

   so.end();

   // UCI analysis/ponder buffering

   while (!si.move || sg.ponder()) {

      std::string line;
      if (!peek_line(line)) { // EOF
         std::exit(EXIT_SUCCESS);
      }

      if (!line.empty()) {

         std::stringstream ss(line);

         std::string command;
         ss >> command;

         if (false) {
         } else if (command == "isready") {
            get_line(line);
            std::cout << "readyok" << std::endl;
         } else if (command == "ponderhit") {
            get_line(line);
            return;
         } else { // other command => abort search
            return;
         }
      }
   }
}

Move quick_move(const Pos & pos) {

   // init

   List list;
   gen_legals(list, pos);

   if (list.size() == 0) return move::None;
   if (list.size() == 1) return list[0];

   // transposition table

   tt::Info tt_info;

   if (tt::G_TT.probe(hash::key(pos), tt_info)
    && tt_info.move != move::None
    && list::has(list, tt_info.move)
    ) {
      return tt_info.move;
   }

   return move::None;
}

Score quick_score(const Pos & pos) {

   // transposition table

   tt::Info tt_info;

   if (tt::G_TT.probe(hash::key(pos), tt_info)) {
      return score::from_tt(tt_info.score, Ply_Root);
   }

   return score::None;
}

static double alloc_moves(const Pos & pos) {
   return lerp(30.0, 10.0, pos::phase(pos));
}

static double alloc_early(const Pos & pos) {
   return lerp(0.4, 0.8, pos::phase(pos));
}

static double lerp(double mg, double eg, double phase) {
   assert(phase >= 0.0 && phase <= 1.0);
   return mg + (eg - mg) * phase;
}

Search_Input::Search_Input() {
   init();
}

void Search_Input::init() {

   var::update();

   move = true;
   depth = Depth_Max;

   smart = false;
   moves = 0;
   time = 1E6;
   inc = 0.0;
   ponder = false;
}

void Search_Input::set_time(int moves, double time, double inc) {
   smart = true;
   this->moves = moves;
   this->time = time;
   this->inc = inc;
}

void Search_Output::init(const Search_Input & si, const Pos & pos) {

   p_si = &si;
   p_pos = pos;

   move = move::None;
   answer = move::None;
   score = score::None;
   flag = Flag::None;
   depth = Depth(0);
   pv.clear();

   p_timer.reset();
   p_timer.start();
   node = 0;
   ply_max = 0;
}

void Search_Output::end() {
   p_timer.stop();
}

void Search_Output::new_best_move(Move mv, Score sc) {

   Line pv;
   pv.set(mv);

   new_best_move(mv, sc, Flag::Exact, Depth(0), pv);
}

void Search_Output::new_best_move(Move mv, Score sc, Flag flag, Depth depth, const Line & pv) {

   if (pv.size() != 0) assert(pv[0] == mv);

   move = mv;
   answer = (pv.size() < 2) ? move::None : pv[1];
   score = sc;
   this->flag = flag;
   this->depth = depth;
   this->pv = pv;

   disp_best_move();
}

void Search_Output::disp_best_move() {

   if (var::SMP) G_IO.lock();

   double time = this->time();
   double speed = (time < 0.01) ? 0.0 : double(node) / time;

   std::string line = "info";
   if (depth != 0)   line += " depth "    + std::to_string(depth);
   if (ply_max != 0) line += " seldepth " + std::to_string(ply_max);

   if (score != score::None) {

      if (score::is_win(score)) {
         line += " score mate " + std::to_string(+(score::ply(score) + 1) / 2);
      } else if (score::is_loss(score)) {
         line += " score mate " + std::to_string(-(score::ply(score) + 1) / 2);
      } else {
         line += " score cp " + std::to_string(score);
      }

      if (flag == Flag::Lower) line += " lowerbound";
      if (flag == Flag::Upper) line += " upperbound";
   }

   if (node != 0)      line += " nodes " + std::to_string(node);
   if (time >= 0.001)  line += " time "  + std::to_string(ml::round(time * 1000));
   if (speed != 0.0)   line += " nps "   + std::to_string(ml::round(speed));
   if (pv.size() != 0) line += " pv "    + pv.to_uci(p_pos);
   std::cout << line << std::endl;

   if (var::SMP) G_IO.unlock();
}

double Search_Output::time() const {
   return p_timer.elapsed();
}

void Time::init(const Search_Input & si, const Pos & pos) {

   if (si.smart) {
      init(si.moves, si.time, si.inc, pos);
   } else {
      init(si.time);
   }
}

void Time::init(double time) {
   p_time_0 = time;
   p_time_1 = time;
   p_time_2 = time;
}

void Time::init(int moves, double time, double inc, const Pos & pos) {

   moves = std::min(moves, 30);

   double moves_left = alloc_moves(pos);
   if (moves != 0) moves_left = std::min(moves_left, double(moves));

   double factor = 1.3;
   if (var::Ponder) factor *= 1.2;

   double total = std::max(time + inc * moves_left, 0.0);
   double alloc = total / moves_left * factor;

   if (moves > 1) { // save some time for the following moves
      double total_safe = std::max((time / double(moves - 1) + inc - (time / double(moves) + inc) * 0.5) * double(moves - 1), 0.0);
      total = std::min(total, total_safe);
   }

   double max = time_lag(std::min(total, time + inc) * 0.95);

   p_time_0 = std::min(time_lag(alloc), max);
   p_time_1 = std::min(time_lag(alloc * 4.0), max);
   p_time_2 = max;

   assert(0.0 <= p_time_0 && p_time_0 <= p_time_1 && p_time_1 <= p_time_2);
}

static double time_lag(double time) {
   return std::max(time - 0.1, 0.0); // assume 100ms of lag
}

void Search_Global::init(const Search_Input & si, Search_Output & so, const Pos & pos, const List & list) {

   p_si = &si;
   p_so = &so;

   p_pos = &pos;
   p_list = list;

   p_ponder = si.ponder;
   p_flag = false;

   p_last_move = move::None;
   p_last_score = score::None;

   p_change = false;
   p_first = false;
   p_high = 0;
   p_drop = false;
   p_factor = 1.0;

   p_depth = Depth(0);
   p_current_move = move::None;
   p_current_number = 0;

   p_last_poll = 0.0;

   // new search

   G_SMP.busy = false;
   p_root_sp.init_root(ID_Main);

   for (int i = 0; i < var::Threads; i++) {
      ID id = ID(i);
      sl(id).init(id, *this); // also launches a thread if id /= 0
   }

   tt::G_TT.inc_date();
   sort_clear();
}

void Search_Global::collect_stats() {

   p_so->node = 0;
   p_so->ply_max = 0;

   for (int id = 0; id < var::Threads; id++) {
      sl(ID(id)).end_iter(*p_so);
   }
}

void Search_Global::end() {

   abort();

   p_root_sp.leave(ID_Main);
   assert(p_root_sp.free());

   for (int id = 0; id < var::Threads; id++) {
      sl(ID(id)).end();
   }
}

void Search_Global::search(Depth depth) {

   p_depth = depth;
   p_current_move = move::None;
   p_current_number = 0;

   for (int id = 0; id < var::Threads; id++) {
      sl(ID(id)).start_iter();
   }

   sl(ID_Main).search_root_try(pos(), p_list, depth);

   // time management

   Move  mv = p_so->move;
   Score sc = p_so->score;

   if (p_si->smart && depth > 1 && mv == p_last_move) {
      p_factor = std::max(p_factor * 0.9, 0.6);
   }

   p_last_move = mv;
   p_last_score = sc;
}

void Search_Global::search_move(Move mv, int searched_size) {

   p_current_move = mv;
   p_current_number = searched_size + 1;

   p_first = searched_size == 0;
}

void Search_Global::new_best_move(Move mv, Score sc, Flag flag, Depth depth, const Line & pv, bool fail_low) {

   if (var::SMP) lock();

   Move bm = p_so->move;

   collect_stats(); // update search info
   p_so->new_best_move(mv, sc, flag, depth, pv);

   int i = list::find(p_list, mv);
   p_list.mtf(i);

   if (depth > 1 && mv != bm) {

      clear_flag();

      if (p_si->smart) {
         p_factor = std::max(p_factor, 1.0);
         p_factor = std::min(p_factor * 1.2, 2.0);
      }
   }

   Score delta = sc - p_last_score;
   p_drop = fail_low || delta <= -20;
   if (delta <= -20) clear_flag();

   if (var::SMP) unlock();
}

void Search_Global::poll() {

   if (depth() <= 1) return;

   bool abort = false;

   // input event?

   if (var::SMP) G_IO.lock();

   if (has_input()) {

      std::string line;
      if (!peek_line(line)) { // EOF
         std::exit(EXIT_SUCCESS);
      }

      if (!line.empty()) {

         std::stringstream ss(line);

         std::string command;
         ss >> command;

         if (false) {
         } else if (command == "isready") {
            get_line(line);
            std::cout << "readyok" << std::endl;
         } else if (command == "ponderhit") {
            get_line(line);
            p_ponder = false;
            if (p_flag || p_list.size() == 1) abort = true;
         } else { // other command => abort search
            p_ponder = false;
            abort = true;
         }
      }
   }

   if (var::SMP) G_IO.unlock();

   // time limit?

   double time = this->time();

   if (false) {
   } else if (time >= G_Time.time_2()) {
      abort = true;
   } else if (p_si->smart && (high() || drop())) {
      // no-op
   } else if (time >= G_Time.time_1()) {
      abort = true;
   } else if (p_si->smart && !first()) {
      // no-op
   } else if (time >= G_Time.time_0() * factor()) {
      abort = true;
   }

   if (abort) {
      p_flag = true;
      if (!p_ponder) this->abort();
   }

   // send search info every second

   if (var::SMP) lock();

   if (time >= p_last_poll + 1.0) {
      disp_info(true);
      p_last_poll += 1.0;
   }

   if (var::SMP) unlock();
}

void Search_Global::disp_info(bool disp_move) {

   if (var::SMP) G_IO.lock();

   collect_stats();

   double time = p_so->time();
   double speed = (time < 0.01) ? 0.0 : double(p_so->node) / time;

   std::string line = "info";
   if (p_depth != 0)       line += " depth "    + std::to_string(p_depth);
   if (p_so->ply_max != 0) line += " seldepth " + std::to_string(p_so->ply_max);

   if (disp_move && p_current_move != move::None) line += " currmove "       + move::to_uci(p_current_move, pos());
   if (disp_move && p_current_number != 0)        line += " currmovenumber " + std::to_string(p_current_number);

   if (p_so->node != 0) line += " nodes " + std::to_string(p_so->node);
   if (time >= 0.001)   line += " time "  + std::to_string(ml::round(time * 1000));
   if (speed != 0.0)    line += " nps "   + std::to_string(ml::round(speed));
   std::cout << line << std::endl;

   if (var::SMP) G_IO.unlock();
}

void Search_Global::abort() {
   p_root_sp.stop_root();
}

bool Search_Global::has_worker() const {

   if (G_SMP.busy) return false;

   for (int id = 0; id < var::Threads; id++) {
      if (sl(ID(id)).idle()) return true;
   }

   return false;
}

void Search_Global::broadcast(Split_Point * sp) {

   for (int id = 0; id < var::Threads; id++) {
      sl(ID(id)).give_work(sp);
   }
}

void Search_Local::init(ID id, Search_Global & sg) {

   p_id = id;

   p_work = sg.root_sp(); // to make it non-null
   p_stack.clear();
   p_pool_size = 0;

   p_sg = &sg;

   p_node = 0;
   p_ply_max = 0;

   if (var::SMP && p_id != ID_Main) p_thread = std::thread(launch, this, sg.root_sp());
}

void Search_Local::launch(Search_Local * sl, Split_Point * root_sp) {
   sl->idle_loop(root_sp);
}

void Search_Local::end() {
   if (var::SMP && p_id != ID_Main) p_thread.join();
}

void Search_Local::start_iter() {
   // p_node = 0;
   p_ply_max = 0;
}

void Search_Local::end_iter(Search_Output & so) {

   if (var::SMP || p_id == ID_Main) {
      so.node += p_node;
      so.ply_max = std::max(so.ply_max, p_ply_max);
   }
}

void Search_Local::idle_loop(Split_Point * wait_sp) {

   push_sp(wait_sp);

   while (true) {

      assert(p_work == p_sg->root_sp());
      p_work = nullptr;

      while (!wait_sp->free() && p_work.load() == nullptr) // spin
         ;

      Split_Point * work = p_work.exchange(p_sg->root_sp()); // to make it non-null
      if (work == nullptr) break;

      join(work);
   }

   pop_sp(wait_sp);

   assert(wait_sp->free());
   assert(p_work == p_sg->root_sp());
}

void Search_Local::give_work(Split_Point * sp) {

   if (idle(sp->parent())) {

      sp->enter(p_id);

      assert(p_work.load() == nullptr);
      p_work = sp;
   }
}

bool Search_Local::idle(Split_Point * parent) const {

   lock();
   bool idle = this->idle() && parent->is_child(top_sp());
   unlock();

   return idle;
}

bool Search_Local::idle() const {
   return p_work.load() == nullptr;
}

void Search_Local::search_all_try(const Pos & pos, List & list, Depth depth) {

   assert(is_legal(pos));
   assert(list.size() != 0);
   assert(depth > 0 && depth <= Depth_Max);

   assert(p_stack.empty());
   push_sp(p_sg->root_sp());

   try {
      search_all(pos, list, depth, Ply_Root);
   } catch (const Abort &) {
      pop_sp(p_sg->root_sp());
      assert(p_stack.empty());
      // throw;
   }

   pop_sp(p_sg->root_sp());
   assert(p_stack.empty());
}

void Search_Local::search_root_try(const Pos & pos, const List & list, Depth depth) {

   assert(is_legal(pos));
   assert(list.size() != 0);
   assert(depth > 0 && depth <= Depth_Max);

   assert(p_stack.empty());
   push_sp(p_sg->root_sp());

   try {
      search_asp(pos, list, depth, Ply_Root);
   } catch (const Abort &) {
      pop_sp(p_sg->root_sp());
      assert(p_stack.empty());
      throw;
   }

   pop_sp(p_sg->root_sp());
   assert(p_stack.empty());
}

void Search_Local::join(Split_Point * sp) {

   // sp->enter(p_id);
   push_sp(sp);

   try {
      move_loop(sp);
   } catch (const Abort &) {
      // no-op
   }

   pop_sp(sp);
   sp->leave(p_id);
}

void Search_Local::move_loop(Split_Point * sp) {

   Node node = sp->node(); // local copy

   while (true) {

      Move mv = sp->get_move(node); // also updates "node"
      if (mv == move::None) break;

      if (!prune(mv, node)) {

         Line pv;
         Score sc = search_move(mv, node, pv);

         sp->update(mv, sc, pv);
      }
   }
}

void Search_Local::search_all(const Pos & pos, List & list, Depth depth, Ply ply) {

   assert(is_legal(pos));
   assert(list.size() != 0);
   assert(depth > 0 && depth <= Depth_Max);
   assert(ply == Ply_Root);

   // move loop

   for (int i = 0; i < list.size(); i++) {

      // search move

      Move mv = list[i];

      inc_node();

      Line pv;
      Score sc = -search(pos.succ(mv), -score::Inf, +score::Inf, depth - Depth(1), ply + Ply(1), move::None, pv);

      // update state

      list.set_score(i, sc);
   }

   list.sort();
}

void Search_Local::search_asp(const Pos & pos, const List & list, Depth depth, Ply ply) {

   assert(is_legal(pos));
   assert(list.size() != 0);
   assert(depth > 0 && depth <= Depth_Max);
   assert(ply == Ply_Root);

   Score last_score = p_sg->last_score();
   assert(depth < 2 || last_score == p_sg->score());

   // window loop

   if (depth >= 4 && score::is_eval(last_score)) {

      int alpha_margin = 10;
      int beta_margin  = 10;

      while (std::max(alpha_margin, beta_margin) < 500) {

         Score alpha = score::add_safe(last_score, -Score(alpha_margin));
         Score beta  = score::add_safe(last_score, +Score(beta_margin));
         assert(-score::Eval_Inf <= alpha && alpha < beta && beta <= +score::Eval_Inf);

         search_root(pos, list, alpha, beta, depth, ply);
         Score sc = p_sg->score();

         if (score::is_win_loss(sc)) {
            break;
         } else if (sc <= alpha) {
            alpha_margin *= 2;
         } else if (sc >= beta) {
            beta_margin *= 2;
         } else {
            assert(sc > alpha && sc < beta);
            return;
         }
      }
   }

   search_root(pos, list, -score::Inf, +score::Inf, depth, ply);
}

void Search_Local::search_root(const Pos & pos, const List & list, Score alpha, Score beta, Depth depth, Ply ply) {

   assert(is_legal(pos));
   assert(list.size() != 0);
   assert(-score::Inf <= alpha && alpha < beta && beta <= +score::Inf);
   assert(depth > 0 && depth <= Depth_Max);
   assert(ply == Ply_Root);

   // init

   Node node;

   node.p_pos = &pos;
   node.alpha = alpha;
   node.beta = beta;
   node.depth = depth;
   node.ply = ply;
   node.root = ply == Ply_Root /* && skip_move == move::None */;
   node.pv_node = beta != alpha + Score(1);
   node.checks = checks(pos);
   node.in_check = node.checks != 0;
   node.eval = eval(pos);
   node.skip_move = move::None;
   node.sing_move = move::None;
   node.sing_score = score::None;
   node.futile = false;

   node.searched.clear();

   node.move = move::None;
   node.score = score::None;
   node.pv.clear();

   // move loop

   node.list = list;
   move_loop(node);
}

Score Search_Local::search(const Pos & pos, Score alpha, Score beta, Depth depth, Ply ply, Move skip_move, Line & pv) {

   assert(is_legal(pos));
   assert(-score::Inf <= alpha && alpha < beta && beta <= +score::Inf);
   assert(depth <= Depth_Max);
   assert(ply <= Ply_Max);

   assert(!p_stack.empty());
   assert(p_stack[0] == p_sg->root_sp());

   // QS

   if (depth <= 0) {
      assert(skip_move == move::None);
      return qs(pos, alpha, beta, Depth(0), ply, pv);
   }

   // init

   pv.clear();

   if (score::win(ply + Ply(1)) <= alpha) return leaf(score::win(ply + Ply(1)), ply);

   if (pos.is_draw()) return leaf(Score(0), ply);

   Node node;

   node.p_pos = &pos;
   node.alpha = alpha;
   node.beta = beta;
   node.depth = depth;
   node.ply = ply;
   node.root = ply == Ply_Root && skip_move == move::None;
   node.pv_node = beta != alpha + Score(1);
   node.checks = Bit(0);
   node.in_check = false;
   node.eval = score::None;
   node.skip_move = skip_move;
   node.sing_move = move::None;
   node.sing_score = score::None;
   node.futile = false;

   node.searched.clear();

   node.move = move::None;
   node.score = score::None;
   node.pv.clear();

   // transposition table

   Move tt_move = move::None;
   Key key = hash_key(pos);

   if (node.skip_move != move::None) key ^= Key(node.skip_move);

   {
      tt::Info tt_info;

      if (p_sg->tt().probe(key, tt_info)) {

         tt_move = tt_info.move;
         node.eval = tt_info.eval;

         tt_info.score = score::from_tt(tt_info.score, node.ply);

         if (!node.pv_node && tt_info.depth >= node.depth) {

            if ((flag_is_lower(tt_info.flag) && tt_info.score >= node.beta)
             || (flag_is_upper(tt_info.flag) && tt_info.score <= node.alpha)
             ) {
               return tt_info.score;
            }
         }

         if (tt_info.depth >= node.depth - 4
          && flag_is_lower(tt_info.flag)
          && score::is_eval(tt_info.score)
          ) {
            node.sing_move  = tt_info.move;
            node.sing_score = tt_info.score;
         }
      }
   }

   // more init

   if (node.ply >= Ply_Max) return leaf(eval(pos), node.ply);

   node.checks = checks(pos);
   node.in_check = node.checks != 0;

   if (!node.in_check && score::loss(node.ply + Ply(2)) >= node.beta) {
      return leaf(score::loss(node.ply + Ply(2)), node.ply);
   }

   if (node.eval == score::None && !node.in_check) node.eval = eval(pos);

   // reverse futility pruning / eval pruning

   if (!node.in_check && node.depth <= 2) {

      Score sc = score::add_safe(node.eval, -Score(node.depth * 100));

      if (sc >= node.beta) {
         node.score = sc;
         node.pv.clear();
         goto cont;
      }
   }

   // null-move pruning

   if (!node.in_check
    && !node.pv_node
    && node.depth >= 1
    && score::is_eval(node.beta)
    && node.eval >= node.beta
    && !null_bad(pos, pos.turn())
    ) {

      Score sc;
      Line new_pv;

      if (node.depth <= 3) {
         sc = snmp(pos, node.beta, node.eval);
      } else {
         inc_node();
         sc = -search(pos.null(), -node.beta, -node.beta + Score(1), node.depth - Depth(node.depth / 4 + 2) - Depth(1), node.ply + Ply(1), move::None, new_pv);
      }

      if (sc >= node.beta) {

         if (sc > +score::Eval_Inf) sc = +score::Eval_Inf; // not a sure win

         node.score = sc;
         node.pv.concat(move::Null, new_pv);
         goto cont;
      }
   }

   // futility pruning

   if (!node.in_check && node.depth <= 4) {

      Score sc = score::add_safe(node.eval, +Score(node.depth * 60));

      if (sc <= node.alpha) {
         node.score = sc; // stand pat
         node.futile = true;
      }
   }

   // move loop

   if (node.futile) {

      gen_tacticals(node.list, pos, node.checks);
      add_checks(node.list, pos);

      if (tt_move != move::None) sort_tt_move(node.list, pos, tt_move);

   } else {

      gen_moves(node.list, pos, node.checks);
      sort_all(node.list, pos, tt_move, node.ply);
   }

   move_loop(node);

cont : // epilogue

   if (node.score == score::None) {

      if (!node.in_check && node.skip_move == move::None) {
         assert(is_stalemate(pos));
         return leaf(Score(0), node.ply);
      } else {
         assert(is_mate(pos) || node.skip_move != move::None);
         return leaf(score::loss(node.ply), node.ply);
      }
   }

   assert(score::is_ok(node.score));

   // transposition table

   {
      tt::Info tt_info;

      tt_info.move = (node.score > node.alpha) ? node.move : move::None;
      tt_info.score = score::to_tt(node.score, node.ply);
      tt_info.flag = flag(node.score, node.alpha, node.beta);
      tt_info.depth = node.depth;
      tt_info.eval = node.eval;

      p_sg->tt().store(key, tt_info);
   }

   // move-ordering statistics

   if (node.score > node.alpha
    && node.move != move::None
    && !move::is_tactical(node.move, pos)
    && node.skip_move == move::None
    ) {

      good_move(node.move, pos, node.ply);

      assert(list::has(node.searched, node.move));

      for (int i = 0; i < node.searched.size(); i++) {

         Move mv = node.searched[i];
         if (mv == node.move) break;

         if (!move::is_tactical(mv, pos)) bad_move(mv, pos, node.ply);
      }
   }

   pv = node.pv;
   return node.score;
}

void Search_Local::move_loop(Node & node) {

   node.searched.clear();
   node.i = 0;
   node.j = 0;

   while (node.score < node.beta && node.i < node.list.size()) {

      int searched_size = node.j;
      assert(searched_size == node.searched.size());

      // SMP

      if (var::SMP
       && node.depth >= 6
       && searched_size != 0
       && node.list.size() - searched_size >= 5
       && p_sg->has_worker()
       && p_pool_size < Pool_Size
       ) {
         split(node);
         return;
      }

      // search move

      Move mv = node.list[node.i++];

      if (node.root) p_sg->search_move(mv, searched_size);

      if (!prune(mv, node)) {

         Line pv;
         Score sc = search_move(mv, node, pv);

         node_update(node, mv, sc, pv, *p_sg);
      }
   }
}

Score Search_Local::search_move(Move mv, const Node & node, Line & pv) {

   // init

   const Pos & pos = node.pos();

   int searched_size = node.j;

   Depth ext = extend(mv, node);
   Depth red = reduce(mv, node);
   assert(ext == 0 || red == 0);

   // singular extension

   if (node.pv_node
    && node.depth >= 6
    && mv == node.sing_move
    && node.skip_move == move::None
    && ext == 0
    ) {

      assert(red == 0);

      Score new_alpha = score::add_safe(node.sing_score, -Score(50));

      Line new_pv;
      Score sc = search(pos, new_alpha, new_alpha + Score(1), node.depth - Depth(4), node.ply, mv, new_pv);

      if (sc <= new_alpha) ext = Depth(1);
   }

   Score new_alpha = std::max(node.alpha, node.score);
   Depth new_depth = node.depth + ext - Depth(1);

   assert(new_alpha < node.beta);

   if (red != 0 && new_depth - red <= 0) red = new_depth - Depth(1); // don't drop to QS

   // search move

   Score sc;

   inc_node();

   Pos new_pos = pos.succ(mv);

   if ((node.pv_node && searched_size != 0) || red != 0) {

      sc = -search(new_pos, -new_alpha - Score(1), -new_alpha, new_depth - red, node.ply + Ply(1), move::None, pv);

      if (sc > new_alpha) { // PVS/LMR re-search

         if (node.root) p_sg->set_high();
         sc = -search(new_pos, -node.beta, -new_alpha, new_depth, node.ply + Ply(1), move::None, pv);
         if (node.root) p_sg->clear_high();
      }

   } else {

      sc = -search(new_pos, -node.beta, -new_alpha, new_depth, node.ply + Ply(1), move::None, pv);
   }

   assert(score::is_ok(sc));
   return sc;
}

Score Search_Local::qs(const Pos & pos, Score alpha, Score beta, Depth depth, Ply ply, Line & pv) {

   assert(is_legal(pos));
   assert(-score::Inf <= alpha && alpha < beta && beta <= +score::Inf);
   assert(depth <= 0);
   assert(ply <= Ply_Max);

   // init

   pv.clear();

   if (score::win(ply + Ply(1)) <= alpha) return leaf(score::win(ply + Ply(1)), ply);

   if (pos.is_draw()) return leaf(Score(0), ply);

   Score eval = score::None;

   // transposition table

   Move tt_move = move::None;
   Key key = hash_key(pos);

   if (depth == 0) {

      tt::Info tt_info;

      if (p_sg->tt().probe(key, tt_info)) {

         tt_move = tt_info.move;
         eval = tt_info.eval;

         tt_info.score = score::from_tt(tt_info.score, ply);

         if ((flag_is_lower(tt_info.flag) && tt_info.score >= beta)
          || (flag_is_upper(tt_info.flag) && tt_info.score <= alpha)
          ||  flag_is_exact(tt_info.flag)
          ) {
            return tt_info.score;
         }
      }
   }

   // more init

   if (ply >= Ply_Max) return leaf(this->eval(pos), ply);

   Bit checks = ::checks(pos);
   bool in_check = depth > -2 && checks != 0;

   if (!in_check && score::loss(ply + Ply(2)) >= beta) {
      return leaf(score::loss(ply + Ply(2)), ply);
   }

   // move-loop init

   Move bm = move::None;
   Score bs = score::None;
   bool is_leaf = true;

   List list;

   if (in_check) {

      gen_evasions(list, pos, checks);
      sort_mvv_lva(list, pos);

   } else {

      // stand pat

      if (eval == score::None) eval = this->eval(pos);

      bs = eval;
      if (bs >= beta) goto cont;

      gen_tacticals(list, pos, checks);
      if (depth == 0) add_checks(list, pos);
   }

   if (tt_move != move::None) sort_tt_move(list, pos, tt_move);

   // move loop

   for (int i = 0; i < list.size(); i++) {

      Move mv = list[i];

      // depth limit

      if (!in_check && depth <= -4 && move::to(mv) != pos.cap_sq()) continue;

      // delta pruning

      if (!in_check
       && eval + see_max(mv, pos) + 200 <= alpha
       && !(depth == 0 && move::is_check(mv, pos))
       ) {
         continue;
      }

      // SEE pruning

      if (!in_check && !move_is_safe(mv, pos)) continue;

      if (!move::pseudo_is_legal(mv, pos)) continue;

      is_leaf = false;

      inc_node();

      Line new_pv;
      Score sc = -qs(pos.succ(mv), -beta, -std::max(alpha, bs), depth - Depth(1), ply + Ply(1), new_pv);

      if (sc > bs) {

         bm = mv;
         bs = sc;
         pv.concat(mv, new_pv);

         if (sc >= beta) break;
      }
   }

cont : // epilogue

   if (is_leaf) mark_leaf(ply);

   if (bs == score::None) {
      assert(in_check);
      assert(is_mate(pos));
      return score::loss(ply);
   }

   assert(score::is_ok(bs));

   // transposition table

   if (depth == 0) {

      tt::Info tt_info;

      tt_info.move = (bs > alpha) ? bm : move::None;
      tt_info.score = score::to_tt(bs, ply);
      tt_info.flag = flag(bs, alpha, beta);
      tt_info.depth = Depth(0);
      tt_info.eval = eval;

      p_sg->tt().store(key, tt_info);
   }

   return bs;
}

Score Search_Local::snmp(const Pos & pos, Score beta, Score eval) { // static NMP with SEE

   assert(is_legal(pos));
   assert(score::is_ok(beta));
   assert(eval >= beta);

   assert(!in_check(pos));

   // stand pat

   eval = score::add_safe(eval, -Score(28)); // STM-bonus ~= +14

   Score bs = eval;
   if (bs < beta) return bs;

   // move loop

   Side sd = pos.turn();
   Side xd = side_opp(sd);

   List list;
   gen_captures  (list, pos, xd);
   sort_mvv_lva  (list, pos);
   add_promotions(list, pos, xd);

   Bit done = Bit(0);

   for (int i = 0; i < list.size(); i++) {

      Move mv = list[i];

      Square to = move::to(mv);
      if (bit::has(done, to)) continue; // only try LVA capture for each victim
      bit::set(done, to);

      Score see = ::see(mv, pos);
      if (see <= 0) continue;

      Score sc = eval - see - Score(100); // positional gain

      if (sc < bs) {
         bs = sc;
         if (sc < beta) break;
      }
   }

   assert(score::is_ok(bs));
   return bs;
}

void Search_Local::split(Node & node) {

   p_sg->poll();
   poll();

   G_SMP.lock(); // useful?

   assert(!G_SMP.busy);
   G_SMP.busy = true;

   assert(p_pool_size < Pool_Size);
   Split_Point * sp = &p_pool[p_pool_size++];
   sp->init(p_id, top_sp(), *p_sg, node);

   p_sg->broadcast(sp);

   assert(G_SMP.busy);
   G_SMP.busy = false;

   G_SMP.unlock();

   join(sp);
   idle_loop(sp);

   sp->get_result(node);

   assert(p_pool_size > 0);
   p_pool_size--;
   assert(sp == &p_pool[p_pool_size]);

   poll();
}

void Search_Local::gen_tacticals(List & list, const Pos & pos, Bit checks) {

   if (checks != 0) {
      gen_eva_caps(list, pos, checks);
      sort_mvv_lva(list, pos);
   } else {
      gen_captures  (list, pos);
      sort_mvv_lva  (list, pos);
      add_promotions(list, pos);
   }
}

bool Search_Local::prune(Move mv, const Node & node) {

   const Pos & pos = node.pos();

   if (mv == node.skip_move) return true;

   // SEE pruning for FP

   if (node.futile && !move_is_safe(mv, pos)) return true;

   // late-move pruning

   if (node.depth <= 2
    && node.j >= node.depth * 6
    && node.score >= -score::Eval_Inf
    && !move_is_dangerous(mv, node)
    ) {
      return true;
   }

   // SEE pruning

   if (node.depth <= 4
    && node.score >= -score::Eval_Inf
    && !move_is_dangerous(mv, node)
    && !move_is_safe(mv, pos)
    ) {
      return true;
   }

   if (node.depth <= 1
    && node.score >= -score::Eval_Inf
    && move::is_tactical(mv, pos)
    && !move_is_safe(mv, pos)
    ) {
      return true;
   }

   if (!move::pseudo_is_legal(mv, pos)) return true;

   return false;
}

Depth Search_Local::extend(Move mv, const Node & node) {

   int ext = 0;

   const Pos & pos = node.pos();

   if (node.depth <= 2 && move::is_check (mv, pos)) ext += 1;

   if (node.pv_node && move::is_check    (mv, pos)) ext += 1;
   if (node.pv_node && move::is_recapture(mv, pos)) ext += 1;

   return Depth(std::min(ext, 1));
}

Depth Search_Local::reduce(Move mv, const Node & node) {

   int red = 0;

   const Pos & pos = node.pos();

   if (node.depth >= 3
    && node.j >= 1
    && !move_is_dangerous(mv, node)
    ) {

      red = LMR_Red[std::min(node.depth, Depth(31))][std::min(node.j, 63)];
      if (node.pv_node) red /= 2; // half reduction for PV nodes

   } else if (!node.pv_node
           && node.depth >= 3
           && node.j >= 3
           && move_is_dangerous(mv, node)
           && !move_is_safe(mv, pos)
           ) {

      red = 1; // reduce bad moves a little bit
   }

   return Depth(red);
}

bool Search_Local::move_is_dangerous(Move mv, const Node & node) {

   const Pos & pos = node.pos();

   return move::is_tactical(mv, pos)
       || node.in_check
       || move::is_check(mv, pos)
       ;
}

void Search_Local::inc_node() {
   p_node += 1;
   if ((p_node & ml::bit_mask(8)) == 0) p_sg->poll();
   if ((p_node & ml::bit_mask(4)) == 0) poll();
}

Score Search_Local::leaf(Score sc, Ply ply) {
   assert(score::is_ok(sc));
   mark_leaf(ply);
   return sc;
}

void Search_Local::mark_leaf(Ply ply) {
   p_ply_max = std::max(p_ply_max, int(ply));
}

Score Search_Local::eval(const Pos & pos) {
   return ::eval(pos, pos.turn());
}

Key Search_Local::hash_key(const Pos & pos) {
   return pos.key();
}

bool Search_Local::null_bad(const Pos & pos, Side sd) {
   return pos::force(pos, sd) <= 1 // ; // at most one minor
       || pos.pawns(sd) == 0; // no pawns
}

void Search_Local::poll() {
   if (stop()) throw Abort();
}

bool Search_Local::stop() const {

   for (Split_Point * sp = top_sp(); sp != nullptr; sp = sp->parent()) {
      if (sp->stop()) return true;
   }

   return false;
}

void Search_Local::push_sp(Split_Point * sp) {

   lock();

   if (!p_stack.empty()) assert(sp->is_child(top_sp()));
   p_stack.add(sp);

   unlock();
}

void Search_Local::pop_sp(Split_Point * sp) { // sp for debug

   lock();
   assert(top_sp() == sp);
   p_stack.remove();
   unlock();
}

Split_Point * Search_Local::top_sp() const {
   assert(!p_stack.empty());
   return p_stack[p_stack.size() - 1];
}

void Split_Point::init_root(int master) {

   p_parent = nullptr;

   p_workers = ml::bit(master);
   p_stop = false;
}

void Split_Point::init(int master, Split_Point * parent, Search_Global & sg, const Node & node) {

   assert(parent != nullptr);

   p_parent = parent;
   p_sg = &sg;

   p_node = node;

   p_workers = ml::bit(master);
   p_stop = false;
}

void Split_Point::get_result(Node & node) {
   node = p_node;
}

void Split_Point::enter(ID id) {
   assert(!ml::bit_has(p_workers, id));
   p_workers |= ml::bit(id);
}

void Split_Point::leave(ID id) {
   assert(ml::bit_has(p_workers, id));
   p_workers &= ~ml::bit(id);
}

Move Split_Point::get_move(Node & node) {

   Move mv = move::None;

   lock();

   if (p_node.score < p_node.beta && p_node.i < p_node.list.size()) {

      mv = p_node.list[p_node.i++];

      node.score = p_node.score;
      node.j = p_node.j;
   }

   unlock();

   return mv;
}

void Split_Point::update(Move mv, Score sc, const Line & pv) {

   lock();

   if (p_node.score < p_node.beta) { // ignore superfluous moves after a fail high
      node_update(p_node, mv, sc, pv, *p_sg);
      if (p_node.score >= p_node.beta) p_stop = true;
   }

   unlock();
}

static void node_update(Node & node, Move mv, Score sc, const Line & pv, Search_Global & sg) {

   node.searched.add(mv);
   node.j++;
   assert(node.j <= node.i);

   if (sc > node.score) {

      node.move = mv;
      node.score = sc;
      node.pv.concat(mv, pv);

      if (node.root && (node.j == 1 || sc > node.alpha)) {
         sg.new_best_move(node.move, node.score, flag(node.score, node.alpha, node.beta), node.depth, node.pv, sc <= node.alpha);
      }
   }
}

void Split_Point::stop_root() {
   p_stop = true;
}

bool Split_Point::is_child(Split_Point * sp) {

   for (Split_Point * s = this; s != nullptr; s = s->p_parent) {
      if (s == sp) return true;
   }

   return false;
}

Line::Line() {
   clear();
}

void Line::clear() {
   p_move.clear();
}

void Line::add(Move mv) {
   assert(mv != move::None);
   p_move.add(mv);
}

void Line::set(Move mv) {
   clear();
   add(mv);
}

void Line::concat(Move mv, const Line & pv) {

   clear();
   add(mv);

   for (int i = 0; i < pv.size(); i++) {
      add(pv[i]);
   }
}

int Line::size() const {
   return p_move.size();
}

Move Line::move(int i) const {
   return p_move[i];
}

Move Line::operator[](int i) const {
   return move(i);
}

std::string Line::to_uci(const Pos & pos) const {

   std::string s;

   Pos new_pos = pos;

   for (int i = 0; i < this->size(); i++) {

      Move mv = move(i);
      if (mv == move::Null) break;

      if (!s.empty()) s += " ";
      s += move::to_uci(mv, new_pos);

      new_pos = new_pos.succ(mv);
      assert(is_legal(new_pos));
   }

   return s;
}

static Flag flag(Score sc, Score alpha, Score beta) {

   assert(-score::Inf <= alpha && alpha < beta && beta <= +score::Inf);

   Flag flag = Flag::None;
   if (sc > alpha) flag |= Flag::Lower;
   if (sc < beta)  flag |= Flag::Upper;

   return flag;
}

