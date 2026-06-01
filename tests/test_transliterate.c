#include "../headers/util.h"
#include <stdio.h>

//Convert a UTF-8 string to ASCII and print to stdout
//transliterate_latin() rewrites in place, so each sample must be a writable
//array (not a string literal) & len(str_utf8) <= len(str_ascii)
static void run(char* utf8){
    printf("%-34s -> ", utf8);
    transliterate_latin(utf8);
    printf("%s\n", utf8);
}

int main(void){
    //accented samples as literal UTF-8 (this source file is UTF-8 encoded)
    char french[]  = "Voilà la crème brûlée, Œuvre & cœur";
    char polish[]  = "Zażółć gęślą jaźń";
    char dropped[] = "emoji 😀 and quote ’ dropped"; //3 and 4-byte chars are dropped
    char ascii[]   = "Plain ASCII stays untouched!";

    run(french);
    run(polish);
    run(dropped);
    run(ascii);
    return 0;
}
