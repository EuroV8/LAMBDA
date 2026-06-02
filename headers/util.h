#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

#define LRC_DIR "/.lambda/lyrics"

//Map a 2-byte UTF-8 codepoint (Latin-1 Supplement + Latin Extended-A subset),
//given as its two raw bytes, to its ASCII fold. Returns the substitution
//(1-2 chars) or NULL if the codepoint has no mapping.
const char* fold_2byte(uint8_t b1, uint8_t b2);

//Normalizes UTF-8 encoded chars to plain ASCII and adds null terminator
//Caller realloc required (rare are the cases where len(utf8_str) == len(ascii_str))
//utf-8 has variable encoding length : in C this equals to storing 1-4 bytes for one char in a char array
//returns the new length in bytes
size_t transliterate_latin(char* text);

//Builds a POSIX extended regex matching the song's word tokens in order
//"Song Title (Radio Edit)" -> "song.*title.*radio.*edit" (lowercased, folded to ASCII,
//split on non alphanumerical chars)
//Returns a malloc'ed pattern, or NULL if the name has no alphanumeric tokens
//CALLER free()
static char* build_token_pattern(const char* song_name);

//Finds the an lrc file in ~/.lambda/lyrics whose name best matches "song_name"
//accent- and case-insensitive token match, see build_token_pattern
//shortest filename wins on ties
//Returns a malloc'ed path or NULL. 
//CALLER free
char* find_lrc_for_song(const char* song_name);

#endif
