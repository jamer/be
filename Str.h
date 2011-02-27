#ifndef STR_H
#define STR_H

#ifndef NULL
# if !defined(__cplusplus)
#  define NULL ((void*)0)
# else
#  define NULL (0)
# endif
#endif

// The String class
class Str {
public:

	// Constructors
	Str();
	Str(int len);
	Str(const char* str);

	// Copy constructor
	Str(const Str& str);

	// Deconstructor
	~Str();

	// Methods
	int length();
	void buffer(int len);
	char* copy();
	Str& trim();
	void free();

	int contains(Str& pattern, int start = 0);
	Str& insert(Str& newStuff, int index);
	Str& remove(int begin, int end = -1);
	Str& replace(Str& newStuff, int begin, int end);

	// Operators
	operator const char*();
	char& operator [](int index);

	Str& operator =(const char* str);
	Str& operator =(Str& str);

	Str& operator +=(const char* str);
	Str& operator +=(Str& str);

	Str& operator +=(bool b);
	Str& operator +=(char c);
	Str& operator +=(int c);
	Str& operator +=(short c);
	Str& operator +=(long i);
	Str& operator +=(unsigned long i);
	Str& operator +=(long long i);

	bool operator ==(const char* str);
	bool operator ==(Str& str);

	bool operator !=(const char* str);
	bool operator !=(Str& str);

	char operator *();

	bool operator !();

	// Members
	char* data;

};

extern Str nullStr;

#endif // STR_H

