
#ifndef UTIL_HPP
#define UTIL_HPP

// includes

#include <string>

#include <chrono>

#include "libmy.hpp"

// classes

class Bad_Input : public std::exception {
};

class Timer {

private :

   typedef std::chrono::time_point<std::chrono::system_clock> time_t;
   typedef std::chrono::duration<double> second_t;

   double p_elapsed;
   bool p_running;
   time_t p_start;

public :

   Timer() {
      reset();
   }

   void reset() {
      p_elapsed = 0;
      p_running = false;
   }

   void start() {
      p_start = now();
      p_running = true;
   }

   void stop() {
      p_elapsed += time();
      p_running = false;
   }

   double elapsed() const {
      double time = p_elapsed;
      if (p_running) time += this->time();
      return time;
   }

private :

   static time_t now() {
      return std::chrono::system_clock::now();
   }

   double time() const {
      assert(p_running);
      return std::chrono::duration_cast<second_t>(now() - p_start).count();
   }
};

// functions

int find (char c, const std::string & s);

#endif // !defined UTIL_HPP

