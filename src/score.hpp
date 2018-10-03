
#ifndef SCORE_HPP
#define SCORE_HPP

// includes

#include "common.hpp"
#include "libmy.hpp"

namespace score {

// constants

const Score Inf      = Score(10000);
const Score Eval_Inf = Inf - Score(100);
const Score None     = -Inf - Score(1);

// functions

template <typename T>
inline T side(T sc, Side sd) {
   return (sd == White) ? +sc : -sc;
}

bool  is_ok (int sc);
Score win   (Ply ply);
Score loss  (Ply ply);

Score to_tt   (Score sc, Ply ply);
Score from_tt (Score sc, Ply ply);

Score clamp    (Score sc);
Score add_safe (Score sc, Score inc);

bool is_win_loss (Score sc);
bool is_win      (Score sc);
bool is_loss     (Score sc);
bool is_eval     (Score sc);

int  ply (Score sc);

}

#endif // !defined SCORE_HPP

