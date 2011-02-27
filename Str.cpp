#include <string.h>

#include "Str.h"

Str nullStr = "";

void xtoa(unsigned long val, char *buf, unsigned radix, int is_neg) {
	char *p; /* pointer to traverse string */
	char *firstdig; /* pointer to first digit */
	char temp; /* temp char */
	unsigned digval; /* value of digit */

	p = buf;

	if (is_neg) {
		/* negative, so output '-' and negate */
		*p++ = '-';
		val = (unsigned long) (-(long) val);
	}

	firstdig = p; /* save pointer to first digit */

	do {
		digval = (unsigned) (val % radix);
		val /= radix; /* get next digit */

		/* convert to ascii and store */
		if (digval > 9)
			*p++ = (char) (digval - 10 + 'a'); /* a letter */
		else
			*p++ = (char) (digval + '0'); /* a digit */
	} while (val > 0);

	/* We now have the digit of the number in the buffer, but in reverse
	 order.  Thus we reverse them now. */

	*p-- = '\0'; /* terminate string; p points to last digit */

	do {
		temp = *p;
		*p = *firstdig;
		*firstdig = temp; /* swap *p and *firstdig */
		--p;
		++firstdig; /* advance to next two digits */
	} while (firstdig < p); /* repeat until halfway */
}

/* Actual functions just call conversion helper with neg flag set correctly,
 and return pointer to buffer. */

char* itoa(int val, char *buf, int radix) {
	if (radix == 10 && val < 0)
		xtoa((unsigned long) val, buf, radix, 1);
	else
		xtoa((unsigned long) (unsigned int) val, buf, radix, 0);
	return buf;
}

Str::Str() :
	data(NULL) {
}

Str::Str(int len) :
	data(NULL) {
	buffer(len);
}

Str::Str(const char* str) :
	data(NULL) {
	if (!str)
		return;
	buffer(strlen(str) + 1);
	strcpy(data, str);
}

Str::Str(const Str& str) :
	data(NULL) {
	if (!str.data)
		return;
	buffer(strlen(str.data) + 1);
	strcpy(data, str.data);
}

Str::~Str() {
	free();
}

int Str::length() {
	return (data == NULL ? 0 : strlen(data));
}

void Str::buffer(int len) {
	free();
	data = new char[len];
	if(len > 0)
		data[0] = 0;
}

char* Str::copy() {
	if (!data)
		return NULL;
	char* s = new char[length() + 1];

	strcpy(s, data);

	return s;
}

Str& Str::trim() {
	int begin = 0;
	int end = -1;

	for (int i = 0; i < length(); i++) {
		if (data[i] == ' ' || data[i] == '\n' || data[i] == '\r' || data[i]
				== '\t')
			begin++;
		else
			break;
	}

	for (int i = length() - 1; i >= begin; i--) {
		if (data[i] == ' ' || data[i] == '\n' || data[i] == '\r' || data[i]
				== '\t')
			end--;
		else
			break;
	}

	if (begin != 0)
		return replace(nullStr, 0, begin);
	if (end != -1)
		return replace(nullStr, end, -1);
	return *this;
}

void Str::free() {
	if (data)
		delete[] data;
	data = NULL;
}

int Str::contains(Str& pattern, int start) {
	int l = pattern.length();
	for (int i = start; i < length() - l; i++) {
		int j = 0;
		for (; j < l; j++)
			if (data[i + j] != pattern[j])
				break;
		if (j == l)
			return i;
	}

	return -1;
}

Str& Str::insert(Str& newStuff, int index) {
	return replace(newStuff, index, index);
}

Str& Str::remove(int begin, int end) {
	return replace(nullStr, begin, end);
}

Str& Str::replace(Str& newStuff, int begin, int end) {
	int l = (newStuff == NULL ? 0 : newStuff.length());
	int j = length();
	char* newStr = new char[j + l + 1 + begin - end];

	if (begin < 0)
		begin = j + 1 + begin;
	if (end < 0)
		end = j + 1 + end;

	for (int i = 0; i < begin; i++)
		newStr[i] = data[i];

	if (newStuff != NULL && l != 0)
		strcpy(newStr + begin, newStuff.data);

	if (begin < end)
		strcpy(newStr + begin + l, data + end);

	free();

	data = newStr;
	return *this;
}

Str::operator const char*() {
	return data;
}

char& Str::operator [](int index) {
	return data[index];
}

Str& Str::operator =(const char* str) {
	free();

	if (!str)
		return *this;
	buffer(strlen(str) + 1);
	strcpy(data, str);
	return *this;
}

Str& Str::operator =(Str& str) {
	free();

	if (!str)
		return *this;
	buffer(strlen(str) + 1);
	strcpy(data, str.data);
	return *this;
}

Str& Str::operator +=(const char* str) {
	if (!str)
		return *this;

	int newLen = strlen(str) + length();

	char* newData = new char[newLen + 1];
	newData[0] = 0;

	if (data)
		strcpy(newData, data);
	strcat(newData, str);

	free();
	data = newData;
	return *this;
}

Str& Str::operator +=(Str& str) {
	if (!str)
		return *this;
	return operator+=(str.data);
}

Str& Str::operator +=(bool b) {
	return b ? operator +=("true") : operator +=("false");
}

Str& Str::operator +=(char c) {
	int newLen = length() + 1;

	char* newData = new char[newLen + 1];

	if (data)
		strcpy(newData, data);
	newData[newLen - 1] = c;
	newData[newLen] = 0;

	free();
	data = newData;
	return *this;
}

Str& Str::operator +=(int i) {
	char buf[256];
	itoa(i, buf, 10);
	return operator+=(buf);
}

Str& Str::operator +=(short i) {
	char buf[256];
	itoa(i, buf, 10);
	return operator+=(buf);
}

Str& Str::operator +=(long i) {
	char buf[256];
	itoa(i, buf, 10);
	return operator+=(buf);
}

Str& Str::operator +=(unsigned long i) {
	char buf[256];
	itoa(i, buf, 10);
	return operator+=(buf);
}

Str& Str::operator +=(long long i) {
	long l = (long) i;
	char buf[256];
	itoa(l, buf, 10);
	return operator+=(buf);
}

bool Str::operator ==(const char* str) {
	if (!str)
		return !data;
	return !strcmp(data, str);
}

bool Str::operator ==(Str& str) {
	if (!str)
		return !data;
	//printf("\n%s == %s", data, str.data);
	return !strcmp(data, str.data);
}

bool Str::operator !=(const char* str) {
	if (!str)
		return !(!data);
	return strcmp(data, str);
}

bool Str::operator !=(Str& str) {
	if (!str)
		return !(!data);
	return strcmp(data, str.data);
}

char Str::operator *() {
	return data[0];
}

bool Str::operator !() {
	return !data;
}
