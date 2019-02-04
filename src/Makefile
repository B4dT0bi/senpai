
# files

EXE = senpai

OBJS = attack.o bit.o common.o eval.o fen.o game.o gen.o \
       hash.o libmy.o list.o main.o math.o move.o pawn.o \
       pos.o score.o search.o sort.o thread.o tt.o util.o var.o

# rules

all: $(EXE)

clean:
	$(RM) $(OBJS) .depend # keep exe

# general

CXX      = clang++ # g++ if you must, but beware of NPS!
CXXFLAGS = -pthread
LDFLAGS  = -pthread

# warnings

CXXFLAGS += -ansi -pedantic -Wall -Wextra

# C++

CXXFLAGS += -std=c++11 -fno-rtti

# optimisation

CXXFLAGS += -O2 -mpopcnt -mbmi2 -DBMI
LDFLAGS  += -O2

# dependencies

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

.depend:
	$(CXX) $(CXXFLAGS) -MM $(OBJS:.o=.cpp) > $@

include .depend

