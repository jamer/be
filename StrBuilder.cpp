#include <string.h>
#include "StrBuilder.h"

// defined in Str.cpp
char* itoa(int val, char *buf, int radix);


StrBuilder::StrBuilder(int len) : data(NULL), len(0), alloc(0) {
	buffer(len);
}

StrBuilder::~StrBuilder() {
	free();
}

int StrBuilder::length() {
	return len;
}

void StrBuilder::buffer(int sz) {
	if (len == 0) {
		free();
		data = new char[sz];
		alloc = sz;
		return;
	}

	// TODO: make sure newData can hold data
	int l = len;
	char* newData = new char[sz];
	strcpy(newData, data);

	free();
	len = l;
	alloc = sz;
	data = newData;
}

void StrBuilder::empty() {
	len = 0;
	if (data)
		data[0] = '\0';
}

inline void StrBuilder::expand(int sz) {
	while (sz > alloc - len - 1)
		buffer(alloc * 3 + 1);
}

void StrBuilder::free() {
	if (data == NULL)
		return;
	delete[] data;
	data = NULL;
	len = alloc = 0;
}

char* StrBuilder::copy() {
	if (!data)
		return NULL;
	char* s = new char[len + 1];

	strcpy(s, data);

	return s;
}

StrBuilder::operator const char*() {
	return data;
}

StrBuilder& StrBuilder::operator +=(const char* str) {
	if (!str || str[0] == '\0')
		return *this;

	int l = strlen(str);
	expand(l);
	strcpy(&data[len], str);
	len += l;
	data[len] = 0;
	return *this;
}

StrBuilder& StrBuilder::operator +=(bool b) {
	return b ? operator +=("true") : operator +=("false");
}

StrBuilder& StrBuilder::operator +=(char c) {
	expand(1);
	data[len++] = c;
	data[len] = 0;
	return *this;
}

StrBuilder& StrBuilder::operator +=(short i) {
	char buf[64];
	itoa(i, buf, 10);
	return operator +=(buf);
}

StrBuilder& StrBuilder::operator +=(long i) {
	char buf[64];
	itoa(i, buf, 10);
	return operator +=(buf);
}

StrBuilder& StrBuilder::operator +=(unsigned long i) {
	char buf[64];
	itoa(i, buf, 10);
	return operator +=(buf);
}

StrBuilder& StrBuilder::operator +=(long long i) {
	char buf[64];
	itoa(i, buf, 10);
	return operator +=(buf);
}

bool StrBuilder::operator ==(const char* str) {
	if (!str)
		return !data;
	return !strcmp(data, str);
}

bool StrBuilder::operator !=(const char* str) {
	if (!str)
		return !data;
	return strcmp(data, str);
}

