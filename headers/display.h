#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define HEIGHT 7
#define WIDTH  5
#define GLYPH_GAP 1

#define GLYPH_LINE_MAX_LEN 256

enum Alphabet { A, B, C, D, E, F, G, H, I, J, K, L, M,
                N, O, P, Q, R, S, T, U, V, W, X, Y, Z, SPACE,
                //punctuation / special glyphs (keep A-Z contiguous from 0;
                //char_to_alphabet and parse_label rely on that ordering)
                APOSTROPHE, COMMA, PERIOD, COLON, EXCLAIM, QUESTION, HYPHEN };
#define ALPHABET_COUNT 34

enum FontType { BASE };

typedef struct {
    enum Alphabet letter;
    enum FontType font;
    char** ascii;
} aart_letter;

//clear the terminal
void clear();

//returns the HEIGHT, ie. first col (glyph id x -> first col of glyph id x)
//returns NULL if letter is out of range, the font is unsupported, or load_font()
//has not yet succeeded for that font
const char* const* get_glyph(enum Alphabet letter, enum FontType font);

//loads a font definition file. Returns 0 on success, -1 on error (and prints
//the failure reason to stderr). Must be called before get_glyph returns data.
int load_font(const char* path, enum FontType font);

//dims : 1D array containing [rows, cols]
void get_term_dims(uint16_t* dims);

//get dimensions and start rendering the text
void render_term();

//rendered column-width of n_letters glyphs separated by GLYPH_GAP
int rendered_width(int n_letters);

//map an ASCII character to an Alphabet enum value.
//Returns:
// 0 on success
//-1 if the character has no glyph mapping. Accepts A-Z, a-z, ' ',
//and the punctuation glyphs ' , . : ! ? -
int char_to_alphabet(char c, enum Alphabet* out);

//count how many chars in text have a glyph mapping (same predicate as char_to_alphabet).
//pair with rendered_width() so callers and shift_line_left agree on the column width.
int count_renderable(const char* text);

//render a single text line
//unsupported chars (not in the font) are skipped
void render_line(const char* text);

//scroll a too-wide rendered line leftward across a term_cols-wide window,
//delay_us: delay in microseconds between column steps
//the last column of the line has reached the left edge. Blocking.
//total_w : total width of the line in # of cols
//return once rightmost text column has reached leftmost terminal column
void shift_line_left(const char* text, int total_w, uint16_t term_cols, unsigned int delay_us);

#endif
