#include "../headers/display.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static char        GLYPH_BUFFER[ALPHABET_COUNT][HEIGHT][WIDTH + 1]; // 7 rows per glyph, 6 bytes per row (5 columns of pixels + '\0')
static const char* GLYPH_ROWS[ALPHABET_COUNT][HEIGHT]; //index of glyph id x -> first row of glyph id x
static int         FONT_LOADED = 0;

void clear(){
    fputs("\033[H\033[2J\033[3J", stdout);
    fflush(stdout);
}

static void strip_newline(char* s){
    size_t n = strlen(s);
    while(n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) 
        s[--n] = '\0';
}

static int parse_label(const char* line, enum Alphabet* out){
    if(line[0] != '@') //labels start with @
        return -1;

    const char* p = line + 1;

    while (*p == ' ' || *p == '\t') //handling spaces
        p++;

    if(strncmp(p, "SPACE", 5) == 0 && (p[5] == '\0' || p[5] == ' ' || p[5] == '\t')){ //special case for SPACE
        *out = SPACE;
        return 0;
    }
    if(p[0] >= 'A' && p[0] <= 'Z' && (p[1] == '\0' || p[1] == ' ' || p[1] == '\t')){
        *out = (enum Alphabet)(p[0] - 'A'); //dynamically handles the enum lookup
        return 0;
    }
    return -1;
}

int load_font(const char* path, enum FontType font){

    FILE* f = fopen(path, "r");
    if(!f) 
        exit(EXIT_FAILURE);

    char line[GLYPH_LINE_MAX_LEN];
    enum Alphabet current = A;
    int row_idx = -1; //counter of row position, -1 if parsing a new glyph, 0 <= row_idx < HEIGHT
    int seen[ALPHABET_COUNT] = {0};
    int lineno = 0;

    while(fgets(line, GLYPH_LINE_MAX_LEN, f)){
        lineno++;
        strip_newline(line);

        if(row_idx < 0){
            if(line[0] == '\0') 
                continue;
            if(parse_label(line, &current) != 0){
                fprintf(stderr, "load_font: bad header at %s:%d\n", path, lineno);
                fclose(f);
                return -1;
            }
            row_idx = 0;
            continue;
        }

        size_t len = strlen(line);
        if(len > WIDTH){
            fprintf(stderr, "load_font: row too wide at %s:%d (%zu > %d)\n",
                    path, lineno, len, WIDTH);
            fclose(f);
            return -1;
        }
        memcpy(GLYPH_BUFFER[current][row_idx], line, len);
        for (size_t i = len; i < WIDTH; i++)
            GLYPH_BUFFER[current][row_idx][i] = ' ';
        GLYPH_BUFFER[current][row_idx][WIDTH] = '\0';
        GLYPH_ROWS[current][row_idx] = GLYPH_BUFFER[current][row_idx];

        row_idx++;
        if (row_idx == HEIGHT) {
            seen[current] = 1;
            row_idx = -1;
        }
    }
    fclose(f);

    if(row_idx >= 0){
        fprintf(stderr, "load_font: incomplete final glyph in %s\n", path);
        return -1;
    }
    for(int i = 0; i < ALPHABET_COUNT; i++){
        if (!seen[i]) {
            fprintf(stderr, "load_font: missing glyph for index %d in %s\n", i, path);
            return -1;
        }
    }

    FONT_LOADED = 1;
    return 0;
}

const char* const* get_glyph(enum Alphabet letter, enum FontType font){
    if(letter < 0 || letter >= ALPHABET_COUNT) 
        return NULL;
    if(font == BASE && FONT_LOADED) 
        return GLYPH_ROWS[letter];
    return NULL;
}

void get_term_dims(uint16_t* dims){
    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1){
        dims[0] = 0;
        dims[1] = 0;
        return;
    }
    dims[0] = ws.ws_row;
    dims[1] = ws.ws_col;
}

//returns the width (column count) of n letters if rendered in the terminal 
int rendered_width(int n_letters){
    if(n_letters <= 0)
        return 0;
    return n_letters * WIDTH + (n_letters - 1) * GLYPH_GAP;
}

int char_to_alphabet(char c, enum Alphabet* out){
    if(c >= 'A' && c <= 'Z'){ *out = (enum Alphabet)(c - 'A'); return 0; }
    if(c >= 'a' && c <= 'z'){ *out = (enum Alphabet)(c - 'a'); return 0; }
    if(c == ' ')             { *out = SPACE;                    return 0; }
    return -1;
}

//count how many chars in text map to a glyph (same predicate as char_to_alphabet).
//pair with rendered_width() to get the pixel width of what render_line/shift_line_left will draw.
int count_renderable(const char* text){
    if(!text)
        return 0;
    size_t len = strlen(text);
    int n = 0;
    for(size_t i = 0; i < len; i++){
        enum Alphabet a;
        if(char_to_alphabet(text[i], &a) == 0)
            n++;
    }
    return n;
}

void render_line(const char* text){
    size_t len = strlen(text);
    for(int row = 0; row < HEIGHT; row++){
        for(size_t i = 0; i < len; i++){
            enum Alphabet a;
            if(char_to_alphabet(text[i], &a) != 0)
                continue;
            const char* const* glyph = get_glyph(a, BASE);
            if(!glyph)
                continue;
            fputs(glyph[row], stdout);
            if(i + 1 < len)
                for(int g = 0; g < GLYPH_GAP; g++)
                    fputc(' ', stdout);
        }
        fputc('\n', stdout);
    }
}

//If the line is too long to fit the width (nb cols) of the terminal - shift it over slowly to the left
//total_w must equal rendered_width(n) where n is the count of renderable chars in text
//(i.e. the same value the caller used to decide line > term_cols)
void shift_line_left(const char* text, int total_w, uint16_t term_cols, unsigned int delay_us){
    if(!text || term_cols == 0)
        return;

    size_t len = strlen(text);

    char* buf[HEIGHT];
    for(int r = 0; r < HEIGHT; r++){
        buf[r] = malloc((size_t)total_w);
        if(!buf[r]){
            for(int k = 0; k < r; k++) free(buf[k]);
            return;
        }
        memset(buf[r], ' ', (size_t)total_w);
    }

    int col = 0;
    for(size_t i = 0; i < len; i++){
        enum Alphabet a;
        if(char_to_alphabet(text[i], &a) != 0)
            continue;
        const char* const* glyph = get_glyph(a, BASE);
        if(!glyph)
            continue;
        for(int r = 0; r < HEIGHT; r++)
            memcpy(buf[r] + col, glyph[r], WIDTH);
        col += WIDTH + GLYPH_GAP;
    }

    int max_offset = total_w - (int)term_cols;
    for(int offset = 0; offset <= max_offset; offset++){
        for(int r = 0; r < HEIGHT; r++){
            fwrite(buf[r] + offset, 1, (size_t)term_cols, stdout);
            fputc('\n', stdout);
        }
        fflush(stdout);
        if(offset < max_offset){
            usleep(delay_us);
            printf("\033[%dA\r", HEIGHT);
        }
    }

    for(int r = 0; r < HEIGHT; r++)
        free(buf[r]);
}