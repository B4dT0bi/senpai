
Senpai 1.0 Copyright (C) 2014 Fabien Letouzey.
This program is distributed under the GNU General Public License version 3.
See LICENSE for more details.

---

Today is 2014-03-17.
Senpai is a chess engine that uses the UCI protocol.
You need a graphical interface supporting UCI to use Senpai.
Have fun with Senpai!

The following people have cooperated to make this release happen, precisely ten years after the publication of Fruit 1.0:

Joachim Rang, my partner of past and present
Ryan Benitez (Fruit 64 for iOS) and Daniel Mehrmann (Fruit reloaded), for trying to explain "modern search" to me
Miguel Ballicora and Julien Marcel, for compilation on alternative operating systems (including Android!) and other advice
Steve Maughan, for hosting Senpai; don't miss his blog: http://www.chessprogramming.net
José mº Velasco for compiling

I also want to thank:

Tord Romstad and Frank Quisinsky (www.amateurschach.de), my close friends in the community
Gerd Isenberg for his incredible work on The Chess Programming Wiki (http://chessprogramming.wikispaces.com)

---

miscellaneous sections

* Bug (yes, really).

Senpai has a known issue on Windows.  The hash table appears to be 50% full at most.  We have been unable to reproduce this problem on either OS X or Linux, using either GCC or Clang.  So there is a possibility (though by no means certain or even probable) that it is a compatibility problem with MinGW.  We haven't found any Windows programmer with Visual Studio or the Intel compiler, maybe this would fix it?

* No working executable for your system?

If you can't compile by yourself, contact me or Julien (my address is at the bottom).  We'll try to do something for you.  For OS X users with a version anterior to Mountain Lion, we are already aware of the issue.  C++11 did not exist when they were designed.  The solutions are complex and we are going to address the limitation when we find the time.  We prefer to hear from you in this case also, so that we don't work for no reason.

* How to compile?

Senpai uses C++11; make sure your compiler is compatible, in particular with std::thread.
This might require additional installation, for instance a pthread library (and MinGW-64) on Windows.

For the GCC family (including Clang), you can use the following command (assuming your system supports SSE4.2, remove the corresponding option if it doesn't):
g++ -std=c++11 -O3 -finline-functions -funroll-all-loops -fno-rtti -msse4.2 -o senpai_10 senpai_10.cpp

* source code

I haven't had the time to clean up the source code, sorry about that.  In exchange, you have SMP. Which one do you prefer?  Me too.

---

Thanks for you attention,

Fabien Letouzey (fabien_letouzey@hotmail.com).

