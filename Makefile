CFLAGS=-g
LDFLAGS=-g

all: bepp

bepp: BattleEngine.o Engine.o Str.o StrBuilder.o XML.o
	$(CXX) *.o -o bepp

clean:
	$(RM) *.o bepp

