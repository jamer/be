#include <stdarg.h>
#include <stdio.h>

#define null 0

#define CharToNumber(x)		(x-'0')
#define NumberToChar(x)		(x+'0')

extern char CanQuit; // Allow the (X) button to close the window

extern long long holdRandom;

long Random();
long long GetRandomSeed();
void RandomSeed(long long seed);
long long GetMillisecondCount();

void ClearScreen();
void FlushInput();
long PressAnyKey();
long GetChar();

void Print(const char* format, ...);
const char* StrPrint(const char* format, ...);

void Quit();

void fwrite(long i, FILE* f);
void fwrite(long long i, FILE* f);
void fread(long* i, FILE* f);
void fread(long long* i, FILE* f);

