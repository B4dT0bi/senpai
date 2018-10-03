
#ifndef THREAD_HPP
#define THREAD_HPP

// includes

#include <string>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "libmy.hpp"

// classes

class Lockable {

protected : // HACK for Waitable::wait()

   mutable std::mutex p_mutex;

public :

   void lock   () const;
   void unlock () const;
};

class Waitable : public Lockable {

private :

   std::condition_variable_any p_cond;

public :

   void wait   ();
   void signal ();
};

// functions

void listen_input ();

bool has_input ();
bool peek_line (std::string & line);
bool get_line  (std::string & line);

#endif // !defined THREAD_HPP

