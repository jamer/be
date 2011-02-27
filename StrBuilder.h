#ifndef STRBUILDER_H
#define STRBUILDER_H

class StrBuilder {
public:

	// Constructors
//	StrBuilder();
	StrBuilder(int len);
//	StrBuilder(const char* str);
//	StrBuilder(Str& str);

	// Deconstructor
	~StrBuilder();

	// Methods
	int length();
	void buffer(int len);
	void empty();
	void free();
	char* copy();

	operator const char*();

	// Operators
//	char& operator [](int index);

//	StrBuilder& operator =(const char* str);
//	StrBuilder& operator =(Str& str);

	StrBuilder& operator +=(const char* str);

	StrBuilder& operator +=(bool b);
	StrBuilder& operator +=(char c);
	StrBuilder& operator +=(short i);
	StrBuilder& operator +=(long i);
	StrBuilder& operator +=(unsigned long i);
	StrBuilder& operator +=(long long i);

	bool operator ==(const char* str);
//	bool operator ==(Str& str);

	bool operator !=(const char* str);
//	bool operator !=(Str& str);

//	bool operator !();

private:
	inline void expand(int len);

	// Members
	char* data;
	int len;
	int alloc;
};

#endif // STRBUILDER_H

