#include "../headers/display.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FONT_PATH      "fonts/base.txt"
#define TEXT_PATH      "tests/text/renderer_test.txt"
#define MAX_LINES      32
#define MAX_TEXT_LEN   256
#define SCROLL_DELAY_US 30000

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

    uint16_t dims[2] = {0, 0};
    get_term_dims(dims);
    clear();
    for(int i = 0; i < nlines; i++){
        clear();
        int w = rendered_width(count_renderable(lines[i]));
        if(dims[1] > 0 && w > (int)dims[1])
            shift_line_left(lines[i], w, dims[1], SCROLL_DELAY_US);
        else
            render_line(lines[i]);
        fputc('\n', stdout);
    }

    return 0;
}
