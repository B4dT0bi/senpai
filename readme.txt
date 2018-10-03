
Senpai 2.0 Copyright (C) 2014-2017 Fabien Letouzey.
This program is distributed under the GNU General Public License version 3.
See licence.txt for more details.

---

Today is 2017-11-10.
Senpai is a chess (also 960) engine that uses the UCI protocol.
You need a graphical interface supporting UCI to use Senpai.
Have fun with Senpai!

Thanks to everybody who helped with this release:
- Graham Banks
- Michael Byrne
- Wilhelm Hudetz
- Steve Maughan (https://www.chessprogramming.net)
- Daniel José Queraltó
- Frank Quisinsky (http://www.amateurschach.de)
- XXX (<- this line for people I forgot, sorry in advance).

Greetings to all game programmers; Gens una sumus.

Until my next random apparition,

Fabien Letouzey (fabien_letouzey@hotmail.com).

---

Compilation

Senpai uses C++11.  A Linux/Mac Makefile is provided.  Senpai seems particularly sensitive to link-time optimisation (LTO), aka whole-program optimisation (WPO).  In my experience, Clang (LLVM) is better at this than GCC.

An optional preprocessor BMI definition can be provided externally (like -DBMI), or inserted in libmy.hpp
In case of a portability problem, intrinsics are defined in libmy.hpp

---

Known issues

The halfmove-clock part of FEN is ignored.  This doesn't affect game playing, but will appear if you analyse a new position that is close to a fifty-move draw.

