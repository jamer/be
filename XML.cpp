/*
 * Version 1.1
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//#include <sys/mman.h>
#include <fcntl.h>

#include "XML.h"

#define null 0

#define CDATA_BUFSZ 1024
#define ELEMENT_ID_BUFSZ 32
#define ATTRIBUTE_ID_BUFSZ 32
#define ATTRIBUTE_VAL_BUFSZ 512

// TODO: Instead of using an arraylist, use a linkedlist or a multimap


////////////////////////////////////////
// class element

// Methods
void element::addElement(element* e) {
	element** newChildren = (element**)calloc(sizeof(element*), nChildren + 1);

	for (int i = 0; i < nChildren; i++)
		newChildren[i] = children[i];
	newChildren[nChildren] = e;

	if (nChildren++ != 0)
		free(children);
	children = newChildren;
}

void element::addAttribute(attribute* a) {
	attribute** newAttributes = (attribute**)calloc(sizeof(attribute*), nAttributes + 1);

	for (int i = 0; i < nAttributes; i++)
		newAttributes[i] = attributes[i];
	newAttributes[nAttributes] = a;

	if (nAttributes++ != 0)
		free(attributes);
	attributes = newAttributes;
}

attribute** element::getAttributes(const char* id, int* numberReturned) {
	int nMatching = 0;
	attribute** matching;
	for (int i = 0; i < nAttributes; i++)
		if (!strcmp(attributes[i]->id, id))
			nMatching++;
	matching = new attribute*[nMatching];
	*numberReturned = nMatching;
	if (nMatching == 0)
		return null;
	nMatching = 0;
	for (int i = 0; i < nAttributes; i++)
		if (!strcmp(attributes[i]->id, id))
			matching[nMatching++] = attributes[i];
	return matching;
}

element** element::getChildren(const char* id, int* numberReturned) {
	int nMatching = 0;
	element** matching;
	for (int i = 0; i < nChildren; i++)
		if (!strcmp(children[i]->id, id))
			nMatching++;
	matching = new element*[nMatching];
	*numberReturned = nMatching;
	if (nMatching == 0)
		return null;
	nMatching = 0;
	for (int i = 0; i < nChildren; i++)
		if (!strcmp(children[i]->id, id))
			matching[nMatching++] = children[i];
	return matching;
}

////////////////////////////////////////
// class document


// Methods
bool document::parse(const char* fname, bool mmap) {
	reader r;
	r.usemmap = mmap;
	return r.parse(this, fname);
}

////////////////////////////////////////
// class document::reader

inline char document::reader::cur() {
	return buf[i];
}

inline char document::reader::next() {
	return buf[i++];
}

inline char document::reader::next_char() {
	return buf[++i];
}

inline char document::reader::prev(int index) {
	return buf[i - index];
}

char* document::reader::read_until(int bufsz, const char* delim,
		bool beware_escape)
{
	int l = 0;
	int alloc = bufsz;
	char* s = (char*)malloc(bufsz);
	int i = 0;
	s[0] = 0;

	while (true) {
		const char* found = strchr(delim, cur());
		if (found != NULL) {
			if (!beware_escape || prev(1) != '\\') {
				s[l] = 0;
				return s;
			}
		}
		if (1 == alloc - l - 1) {
			alloc *= 3;
			s = (char*)realloc(s, alloc);
		}

		s[l++] = next();
	}
}

bool document::reader::parse(document* d, const char* fname) {
	doc = d;
	doc->nChildren = doc->nAttributes = 0;
	i = 0;

	par = doc;
	doc->id = (char*) "";

	if (!read_file(fname))
		return false;

	while (true) {
		if (par == doc) {
			while (true) {
				if (cur() == '\0' || cur() == '<')
					break;
				if (!is_whitespace()) {
					printf("illegal character "
						"outside of document... ");
					goto die;
				}
				i++;
			}
		}
		else {
			// Look for an element, add cdata while we find it
			char* cdata = read_until(CDATA_BUFSZ, "<");
			if (par->value != NULL) // FIXME: better check
				par->value = cdata;
		}

		// End of file
		if (cur() == 0)
			break;

		// We've found a '<', what's the next char?
		switch (next_char()) {
			case '?':
				while (true) {
					char c = next_char();
					if (c == '>' && prev(1) == '?')
						break;
				}
				break;
			case '/':
				if (!read_end_tag()) goto die;
				break;
			case '!':
				if (!read_comment()) goto die;
				break;
			default:
				if (!read_element()) goto die;
				break;
		}

		i++;

	}

//	if (usemmap)
//		munmap(buf, buflen);
//	else
		delete buf;
	if (par != doc) {
		printf("ending while still in the middle of tag '%s'... ",
				par->id);
		return false;
	}
	return true;

die:
//	if (usemmap)
//		munmap(buf, buflen);
//	else
		delete buf;
	return false;
}

bool document::reader::read_element() {
	element* e = new element;
	e->nChildren = e->nAttributes = 0;
	e->parent = par;
	e->root = doc;

	skip_whitespace();

	// Get element tag
	char* elId = read_until(ELEMENT_ID_BUFSZ, " \t\r\n>/");

	e->id = elId;

	par->addElement(e);
	par = e;

	// Read attributes
	if (is_whitespace())
		while (read_attribute(e));

	// Element has no children
	if (cur() == '/') {
		par = e->parent;
	}

	else if (cur() != '/' && cur() != '>') {
		printf("Error reading end of open tag '%s'... ", e->id);
		return false;
	}
	
	return true;
}

bool document::reader::read_attribute(element* e) {
	attribute* at = new attribute;
	at->parent = e;
	at->root = doc;

	skip_whitespace();

	// End of element
	if (cur() == '/' || cur() == '>') {
		return false;
	}

	// Read attribute name
	char* atId = read_until(ATTRIBUTE_ID_BUFSZ, " \t\r\n=");

	if (strlen(atId) == 0) {
		printf("Attribute id length zero... ");
		return false;
	}

	at->id = atId;

	skip_whitespace();

	if (!assert('=')) return false;
	i++;

	skip_whitespace();
	if (!assert('\"')) return false;
	i++;

	// Read attribute value
	char* atVal = read_until(ATTRIBUTE_VAL_BUFSZ, "\"", true);
	at->value = atVal;
	e->addAttribute(at);

	i++;

	return true;
}

bool document::reader::read_comment() {
	while (true) {
		if (cur() == 0)
			break;
		if ((prev(2) == '-') &&
		    (prev(1) == '-') &&
		    (cur() == '>'))
			break;
		i++;
	}

	return true;
}

bool document::reader::read_end_tag() {
	const char* id = par->id;

	i++;
	// TODO: read until whitespace
	char* end = read_until(ELEMENT_ID_BUFSZ, ">");

	if (strcmp(end, id)) {
		printf("Out of order end-tag '%s' should be '%s'... ",
				end, id);
		return false;
	}

	par = par->parent;
	return true;
}

/*
 * read_file()
 * Open file, get it's size, and read it into a buffer.
 */
bool document::reader::read_file(const char* fname) {
//	if (!usemmap) {
		FILE* f = fopen(fname, "r");
		if (!f)	{
			printf("Open file failed '%s'... ", fname);
			return false;
		}

		struct stat stats;
		stat(fname, &stats);
		long len = stats.st_size;

		buf = new char[len + 1];
		fread(buf, len, 1, f);

		fclose(f);

		return true;
/*
	} else {
		int fildes = open(fname, O_RDONLY);
		if (fildes == -1) {
			printf("Open file failed '%s'... ", fname);
			return false;
		}

		struct stat stats;
		stat(fname, &stats);
		buflen = stats.st_size;

		buf = (char*)mmap(0, buflen, PROT_READ, MAP_PRIVATE, fildes, 0);
		if (buf == MAP_FAILED) {
			printf("Mapping failed, errno: %i... ", errno);
			return false;
		}

		return true;
	}
*/
}

inline void document::reader::skip_whitespace() {
	while (is_whitespace())
		i++;
}

inline bool document::reader::is_whitespace() {
	char c = cur();
	switch (c) {
		case ' ': case '\t': case '\n': case '\r':
			return true;
		default:
			return false;
	}
}

inline bool document::reader::assert(char c) {
	if (cur() != c) {
		printf("Expecting '%c', got '%c'... ", c, cur());
		return false;
	}
	return true;
}

document::writer::writer(document* d)
	: doc(d) {
}

document::writer::~writer() {
	doc = NULL;
}

bool document::writer::write(const char* fname) {
	return false;
}

