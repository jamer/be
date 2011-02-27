#include <sys/time.h>
#include <stdlib.h>

#include "Engine.h"
#include "StrBuilder.h"

long long holdRandom;

/* FIXME with curses
COORD coord = {0, 0}; // Screen co-ordinates
WORD color;

HANDLE ConsoleInput;
HANDLE ConsoleOutput;
*/

char CanQuit = true; // Allow the (X) button to close the window

void RandomSeed(long long seed)
{
	holdRandom = seed;
}

long long GetRandomSeed()
{
	return holdRandom;
}


// Returns a pseudo-Randomom number 0 through 4,294,967,295.
long Random()
{
	return (int)(((holdRandom = holdRandom * 214013L + 2531011L) >> 16) & 0xffffffff);
}


long long GetMillisecondCount()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec;
}


void ClearScreen()
{
/*
//	CONSOLE_SCREEN_BUFFER_INFO buf;
	DWORD charsRead;
//	COORD size;

//	GetConsoleScreenBufferInfo(ConsoleOutput, &buf);
//	size = buf.dwSize;

//	Print("Size = %i * %i\n", buf.dwSize.X, buf.dwSize.Y);
//	PressAnyKey();

	coord.X = 0;
	coord.Y = 0;

	SetConsoleCursorPosition(ConsoleOutput, coord);

//	FillConsoleOutputCharacter(ConsoleOutput, (TCHAR) ' ', size.X * size.Y, coord, &charsRead);
	FillConsoleOutputCharacter(ConsoleOutput, (TCHAR) ' ', 80 * 50, coord, &charsRead);

	SetConsoleCursorPosition(ConsoleOutput, coord);
*/
}

void FlushInput()
{
	/*
	FlushConsoleInputBuffer(ConsoleInput);
	*/
}

long PressAnyKey()
{
	/*
	FlushInput();
	return GetChar();
	*/
	return 0;
}

/*
INPUT_RECORD ConInpRec;
PINPUT_RECORD GetInput()
{
		static DWORD NumRead;

		if (ConsoleInput == (HANDLE)-1)
			return null;

		ReadConsoleInput(ConsoleInput, &ConInpRec, 1, &NumRead);
		return &ConInpRec;
}
*/



long GetChar()
{
	/*
	long ch;

	while (GetInput())
	{
		// Look for, and decipher, key events.
		if ((ConInpRec.EventType == KEY_EVENT) &&
			 ConInpRec.Event.KeyEvent.bKeyDown)
		{
			if ((ch = ((long)(unsigned char)(ConInpRec.Event.KeyEvent.uChar.AsciiChar))))
				return ch;
		}
	}

	return null;
	*/
	return getchar();
}

long putch(long c)
{
/*
	// Can't use ch directly unless sure we have a big-endian machine.
	unsigned char ch = (unsigned char)c;
	unsigned long num_written;

	// Write character to console file handle.
	if ((ConsoleOutput == (HANDLE)-1) ||
		!WriteConsole(ConsoleOutput,
			(void far *)&ch,
			1,
			&num_written,
			null)
		)
			// Return error indicator.
			return -1;

	if (ch == '\n')
	{
		coord.X = 0;
		coord.Y++;
	}
	else if (ch == '\r')
		coord.X = 0;
	else if (ch == 8)
		coord.X--;
	else
	{
		if (coord.X == 80)
		{
			coord.X = 0;
			coord.Y++;
		}
		if (coord.Y > 299)
			coord.Y = 299;
		FillConsoleOutputAttribute(
			ConsoleOutput,
			color,
			1,
			coord,
			&num_written
		);

		coord.X++;
	}
	return ch;
*/
	return putchar(c);
}

void PrintI(long i)
{
	char c;

	if (i < 0)
	{
		i = -i;
		putch('-');
	}
	c = i%10;
	if (i/10 > 0)
		PrintI(i/10);
	putch(c + '0');
}

void Print(const char* format, ...)
{
	va_list args;
	char ch;
	long i;
	char* tmpstr;
	va_start(args, format);

	while ((ch = *format++) != null)
	{
		if (ch != '%' && ch != '|')
		{
			putch(ch);
			continue;
		}

		if (ch == '%')
		{
			ch = *format++;
			if (ch == null)
				break;

			switch (ch)
			{
			case '%':
				putch(ch);
				break;
			case 's':
				tmpstr = va_arg(args, char*);
				char tmpch;
				while ((tmpch = *tmpstr++) != 0)
					putch(tmpch);
				break;
			case 'i':
				i = va_arg(args, long);
				PrintI(i);
				break;
			case 'c':
				ch = va_arg(args, int);
				putch(ch);
				break;
			}
		}
		else
		{
			ch = *format++;
			if (ch == null)
				break;

/* FIXME with ANSI escape sequences
			switch (ch)
			{
			case '|':
				putch(ch);
				break;
			case 'D': // Dark
				color -= FOREGROUND_INTENSITY;
				break;
			case 'R': // Red
				color = FOREGROUND_RED | FOREGROUND_INTENSITY;
				break;
			case 'B': // Blue
				color = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
				break;
			case 'P': // Purple
				color = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
				break;
			case 'O': // Orange
			case 'Y': // Yellow
				color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			case 'T': // Teal
				color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			case 'W': // White
				color = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			case 'G': // Green
				color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			}
*/
		}
	}
}



const char* StrPrint(const char* format, ...)
{
	static StrBuilder str(1000);
	va_list args;
	char ch;
	long i;
	char* s;
	va_start(args, format);

	str.empty();

	while ((ch = *format++) != null)
	{
		if (ch != '%')
		{
			str += ch;
			continue;
		}

		if (ch == '%')
		{
			ch = *format++;
			if (ch == null)
				break;

			switch (ch)
			{
			case '%':
				str += ch;
				break;
			case 's':
				s = va_arg(args, char*);
				str += s;
				break;
			case 'i':
				i = va_arg(args, long);
				str += i;
				break;
			case 'c':
				ch = (char)va_arg(args, int);
				str += ch;
				break;
			}
		}
	}
	return str;
}



/* FIXME with atexit or on_exit
BOOL WINAPI OnQuit(DWORD CtrlType)
{
	while (!CanQuit)
		Sleep(1);

	return false;
}
*/



void Quit()
{
	CanQuit = true;
	Print("\n\n");
	exit(0);
}



void fwrite(long i, FILE* f)
{
	fwrite(&i, sizeof(i), 1, f);
}

void fwrite(long long i, FILE* f)
{
	fwrite(&i, sizeof(i), 1, f);
}

void fread(long* i, FILE* f)
{
	fread(&i, sizeof(i), 1, f);
}

void fread(long long* i, FILE* f)
{
	fread(&i, sizeof(i), 1, f);
}

