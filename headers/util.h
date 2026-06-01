#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

//Map a 2-byte UTF-8 codepoint (Latin-1 Supplement + Latin Extended-A subset),
//given as its two raw bytes, to its ASCII fold. Returns the substitution
//(1-2 chars) or NULL if the codepoint has no mapping.
const char* fold_2byte(uint8_t b1, uint8_t b2);

//Normalizes UTF-8 encoded chars to plain ASCII and adds null terminator
//Caller realloc required (rare are the cases where len(utf8_str) == len(ascii_str))
//utf-8 has variable encoding length : in C this equals to storing 1-4 bytes for one char in a char array
//returns the new length in bytes
size_t transliterate_latin(char* text);

#endif
