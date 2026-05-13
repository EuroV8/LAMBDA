#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define HEIGHT 7
#define WIDTH  5
#define GLYPH_GAP 1

#define GLYPH_LINE_MAX_LEN 256

enum Alphabet { A, B, C, D, E, F, G, H, I, J, K, L, M,
                N, O, P, Q, R, S, T, U, V, W, X, Y, Z, SPACE };
#define ALPHABET_COUNT 27

enum FontType { BASE };

typedef struct {
    enum Alphabet letter;
    enum FontType font;
    char** ascii;
} aart_letter;

const char* const* get_glyph(enum Alphabet letter, enum FontType font);

// Loads a font definition file. Returns 0 on success, -1 on error (and prints
// the failure reason to stderr). Must be called before get_glyph returns data.
int load_font(const char* path, enum FontType font);

//dims : 1D array containing [rows, cols]
void get_term_dims(uint16_t* dims);

//get dimensions and start rendering the text
void render_term();

//rendered column-width of n_letters glyphs separated by GLYPH_GAP
int rendered_width(int n_letters);

//map an ASCII character to an Alphabet enum value. Returns 0 on success,
//-1 if the character has no glyph mapping. Accepts A-Z, a-z, and ' '.
int char_to_alphabet(char c, enum Alphabet* out);

//render a single text line as HEIGHT rows of glyphs to stdout.
//Unsupported characters are silently skipped. Requires load_font() first.
void render_line(const char* text);

#endif
