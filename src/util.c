#include "../headers/util.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dirent.h>
#include <regex.h>

//Map a 2-byte UTF-8 codepoint (Latin-1 Supplement + Latin Extended-A subset)
//to its ASCII equivalent. 
//Returns the substitution (1-2 chars) or NULL if not supported
const char* fold_2byte(uint8_t b1, uint8_t b2){
    uint16_t codepoint = ((uint16_t)(b1 & 0x1F) << 6) | (b2 & 0x3F);
    switch(codepoint){
        //Polish
        case 0x0104: return "A"; case 0x0105: return "a"; // Ą ą
        case 0x0106: return "C"; case 0x0107: return "c"; // Ć ć
        case 0x0118: return "E"; case 0x0119: return "e"; // Ę ę
        case 0x0141: return "L"; case 0x0142: return "l"; // Ł ł
        case 0x0143: return "N"; case 0x0144: return "n"; // Ń ń
        case 0x015A: return "S"; case 0x015B: return "s"; // Ś ś
        case 0x0179: return "Z"; case 0x017A: return "z"; // Ź ź
        case 0x017B: return "Z"; case 0x017C: return "z"; // Ż ż

        //French (Latin Extended-A)
        case 0x0152: return "OE"; case 0x0153: return "oe"; // Œ œ
        case 0x0178: return "Y"; // Ÿ (lowercase ÿ is 0x00FF, see below)

        //Latin-1 Supplement: A (é è à ç î ï ô û ü … among others)
        case 0x00C0: case 0x00C1: case 0x00C2:
        case 0x00C3: case 0x00C4: case 0x00C5: return "A";
        case 0x00E0: case 0x00E1: case 0x00E2:
        case 0x00E3: case 0x00E4: case 0x00E5: return "a";
        case 0x00C6: return "AE";
        case 0x00E6: return "ae";
        case 0x00C7: return "C";
        case 0x00E7: return "c";
        case 0x00C8: case 0x00C9: case 0x00CA: case 0x00CB: return "E";
        case 0x00E8: case 0x00E9: case 0x00EA: case 0x00EB: return "e";
        case 0x00CC: case 0x00CD: case 0x00CE: case 0x00CF: return "I";
        case 0x00EC: case 0x00ED: case 0x00EE: case 0x00EF: return "i";
        case 0x00D1: return "N";
        case 0x00F1: return "n";
        case 0x00D2: case 0x00D3: case 0x00D4:
        case 0x00D5: case 0x00D6: case 0x00D8: return "O";
        case 0x00F2: case 0x00F3: case 0x00F4:
        case 0x00F5: case 0x00F6: case 0x00F8: return "o";
        case 0x00D9: case 0x00DA: case 0x00DB: case 0x00DC: return "U";
        case 0x00F9: case 0x00FA: case 0x00FB: case 0x00FC: return "u";
        case 0x00DD: return "Y";
        case 0x00FD: case 0x00FF: return "y";
        case 0x00DF: return "ss"; // ß

        default: 
            return NULL;
    }
}

size_t transliterate_latin(char* text){
    size_t read_pos = 0;
    size_t write_pos = 0;
    while(text[read_pos]){ //exits if text[read_pos] '\0', since '\0' = 0, if NULL str is sent, NULL str is returned
        uint8_t lead_byte = (uint8_t)text[read_pos];
        if(lead_byte < 0x80){ //check if char is ASCII already
            text[write_pos] = text[read_pos];
            write_pos++;
            read_pos++;
            continue;
        }
        //2-byte UTF-8 check: 110xxxxx 10xxxxxx
        if((lead_byte & 0xE0) == 0xC0 && ((uint8_t)text[read_pos+1] & 0xC0) == 0x80){
            const char* ascii_fold = fold_2byte(lead_byte, (uint8_t)text[read_pos+1]);
            read_pos += 2; //we just read 2 bytes
            if(ascii_fold){
                while(*ascii_fold){
                    text[write_pos] = *ascii_fold;
                    write_pos++;
                    ascii_fold++;
                }
            }
            continue;
        }
        //3-byte or 4-byte sequences (and malformed leads): are dropped 
        //Advance past the lead byte and any continuation bytes (start utf8 byte is always 0xC0)
        read_pos++;
        while(((uint8_t)text[read_pos] & 0xC0) == 0x80)//skips multibyte chars
            read_pos++;
    }
    text[write_pos] = '\0'; //
    return write_pos;
}

static char* build_token_pattern(const char* song_name){
    size_t n = strlen(song_name);
    char* folded_name = (char*) malloc(n + 1);
    if(!folded_name)
        return NULL;
    memcpy(folded_name, song_name, n + 1);
    transliterate_latin(folded_name); //fold to ASCII in place (most likely shorter unless only ascii chars present in the name)

    //worst case: every char is a token char (<= strlen) plus a 2-char ".*" separator 
    //before each token (< strlen), so 3*strlen + 1 is always enough
    char* pat = malloc(strlen(folded_name) * 3 + 1);
    if(!pat){
        free(folded_name);
        return NULL;
    }

    size_t out = 0;
    int inside_token = 0;
    int wrote_token = 0;
    for(size_t i = 0; folded_name[i]; i++){
        unsigned char c = (unsigned char)folded_name[i];
        if(isalnum(c)){
            if(!inside_token && wrote_token){ //starting a new token after a previous one
                pat[out++] = '.';
                pat[out++] = '*';
            }
            pat[out++] = (char)tolower(c);
            inside_token = 1;
            wrote_token = 1;
        } else {
            inside_token = 0;
        }
    }
    pat[out] = '\0';

    free(folded_name);
    if(!wrote_token){ //we didn't find a single alphanum char
        free(pat);
        return NULL;
    }
    return pat;
}

char* find_lrc_for_song(const char* song_name){
    if(!song_name || !*song_name)
        return NULL;

    const char* home = getenv("HOME");
    if(!home || !*home)
        return NULL;

    //default dir = "~/.lambda/lyrics"
    static const char* subdir = LRC_DIR;
    size_t home_len = strlen(home);
    size_t dir_len = home_len + strlen(subdir);
    char* dir = malloc(dir_len + 1);
    if(!dir)
        return NULL;
    memcpy(dir, home, home_len);
    memcpy(dir + home_len, subdir, strlen(subdir) + 1);

    char* pattern = build_token_pattern(song_name);
    if(!pattern){
        free(dir);
        return NULL;
    }

    regex_t re;
    //REG_NOSUB: we only need yes/no, not the match offsets
    if(regcomp(&re, pattern, REG_EXTENDED | REG_ICASE | REG_NOSUB) != 0){
        free(pattern);
        free(dir);
        return NULL;
    }
    free(pattern);

    DIR* d = opendir(dir);
    if(!d){
        regfree(&re);
        free(dir);
        return NULL;
    }

    char* best = NULL;
    size_t best_len = 0; //filename length of current best (shorter = closer fit)

    struct dirent* ent;
    while((ent = readdir(d)) != NULL){ //checks every file in the dir
        const char* name = ent->d_name;
        size_t name_len = strlen(name);

        //keep only "*.lrc" (case-insensitive extension)
        if(name_len < 4 || strcasecmp(name + name_len - 4, ".lrc") != 0)
            continue;
        //already have a shorter match -> this one can't be better
        if(best && name_len >= best_len)
            continue;

        //fold the basename (without ".lrc") to ASCII before matching
        char* cand = malloc(name_len + 1);
        if(!cand)
            continue;
        memcpy(cand, name, name_len - 4);
        cand[name_len - 4] = '\0';
        transliterate_latin(cand);

        if(regexec(&re, cand, 0, NULL, 0) == 0){
            //build "<dir>/<name>"
            char* path = malloc(dir_len + 1 + name_len + 1);
            if(path){
                memcpy(path, dir, dir_len);
                path[dir_len] = '/';
                memcpy(path + dir_len + 1, name, name_len + 1);
                free(best);
                best = path;
                best_len = name_len;
            }
        }
        free(cand);
    }

    closedir(d);
    regfree(&re);
    free(dir);
    return best;
}
