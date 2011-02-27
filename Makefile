CXX=/usr/bin/i586-mingw32msvc-g++
CFLAGS=-g
LDFLAGS=-g

all: bepp.exe

bepp.exe: BattleEngine.o Engine.o Str.o StrBuilder.o XML.o
	$(CXX) BattleEngine.o Engine.o Str.o StrBuilder.o XML.o -o bepp.exe

clean:
	$(RM) *.o bepp.exe

