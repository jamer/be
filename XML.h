#ifndef XML_H
#define XML_H

#include <stdio.h>

class attribute;
class element;
class document;

class attribute {
public:
	// Members
	char* id;
	char* value;
	element* parent;
	document* root;
};

class element: public attribute {
public:

	// Methods
	void addElement(element* e);
	void addAttribute(attribute* a);

	attribute** getAttributes(const char* id, int* numberReturned);
	element** getChildren(const char* id, int* numberReturned);

	// Members
	attribute** attributes;
	element** children;

	int nAttributes;
	int nChildren;
};

class document: public element {
public:

	// Methods
	bool parse(const char* fname, bool mmap);

	// Members
protected:

	class reader {
	public:
		document* doc;
		element* par;
		char* buf;
		int i;
		int buflen;
		bool usemmap;

		inline char cur();
		inline char next();
		inline char next_char();
		inline char prev(int i);

		char* read_until(int bufsz, const char* delim,
			bool beware_escape = false);

		bool parse(document* d, const char* fname);

		bool read_element();
		bool read_attribute(element* e);
		bool read_comment();
		bool read_end_tag();
		bool read_file(const char* fname);

		inline void skip_whitespace();
		inline bool is_whitespace();
		inline bool assert(char c);
	};

	class writer {
		FILE* f;
		document* doc;

	public:
		writer(document* d);
		~writer();

		bool write(const char* fname);
	};
};

#endif // XML_H

