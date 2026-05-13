#include "../headers/display.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FONT_PATH      "fonts/base.txt"
#define TEXT_PATH      "tests/text/renderer_test.txt"
#define MAX_LINE_COLS  120
#define MAX_LINES      32
#define MAX_TEXT_LEN   64

int main(void){
    if(load_font(FONT_PATH, BASE) != 0){
        fprintf(stderr, "test_renderer: failed to load font %s\n", FONT_PATH);
        return 1;
    }

    FILE* f = fopen(TEXT_PATH, "r");
    if(!f){
        fprintf(stderr, "test_renderer: cannot open %s\n", TEXT_PATH);
        return 1;
    }

    char lines[MAX_LINES][MAX_TEXT_LEN];
    int nlines = 0;
    while(nlines < MAX_LINES && fgets(lines[nlines], MAX_TEXT_LEN, f)){
        size_t n = strlen(lines[nlines]);
        while(n > 0 && (lines[nlines][n-1] == '\n' || lines[nlines][n-1] == '\r'))
            lines[nlines][--n] = '\0';
        if(n > 0)
            nlines++;
    }
    fclose(f);

    if(nlines == 0){
        fprintf(stderr, "test_renderer: no text in %s\n", TEXT_PATH);
        return 1;
    }

    int first_width = rendered_width((int)strlen(lines[0]));

    if(first_width > MAX_LINE_COLS){
        fprintf(stderr,
                "test_renderer: first line needs %d cols, MAX_LINE_COLS is %d\n",
                first_width, MAX_LINE_COLS);
        return 1;
    }

    uint16_t dims[2] = {0, 0};
    get_term_dims(dims);
    if(dims[1] > 0 && (int)dims[1] < first_width){
        fprintf(stderr,
                "test_renderer: terminal too narrow (%u cols, need >= %d for first line)\n",
                dims[1], first_width);
        return 1;
    }

    int cap = MAX_LINE_COLS;
    if(dims[1] > 0 && (int)dims[1] < cap)
        cap = dims[1];

    for(int i = 0; i < nlines; i++){
        int w = rendered_width((int)strlen(lines[i]));
        if(w > cap){
            fprintf(stderr,
                    "test_renderer: line %d needs %d cols > cap %d; skipping\n",
                    i + 1, w, cap);
            continue;
        }
        render_line(lines[i]);
        fputc('\n', stdout);
    }

    return 0;
}
