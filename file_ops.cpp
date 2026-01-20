/*
mah_tracker: AArt1256's custom SID chiptune tracker
Copyright (C) 2026 AArt1256

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see
<https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "defines.h"

const char magic_string[] = "MAHTRACK";

void load_file(char *filename, song *song) {
    FILE *f = fopen(filename, "rb");

    char magic_string_file[8];
    fread(magic_string_file, 1, 8, f);
    if (strncmp(magic_string_file, magic_string, 8) != 0) {
        fclose(f);
        return;
    }

    song->init_speed = fgetc(f);

    fseek(f, 7L, SEEK_CUR);

    fseek(f, 32L, SEEK_CUR);
    fseek(f, 32L, SEEK_CUR);
    fseek(f, 32L, SEEK_CUR);

    fseek(f, 16L, SEEK_CUR);

    // orders
    song->order_len = fgetc(f);
    for (int ch = 0; ch < 3; ch++) {
        for (int ord = 0; ord < song->order_len; ord++) {
            song->order_table[ch][ord] = fgetc(f);
        }
    }

    // patterns
    int max_pat = fgetc(f);
    for (int pat = 0; pat < max_pat; pat++) {
        for (int row = 0; row < 64; row++) {
            song->pattern[pat].rows[row].note = fgetc(f);
            song->pattern[pat].rows[row].instr = fgetc(f);
            song->pattern[pat].rows[row].eff_type = fgetc(f);
            song->pattern[pat].rows[row].eff_arg = fgetc(f);
        }
    }  
    
    // instruments (WIP)
    for (int inst = 0; inst < 128; inst++) {
        instrument *instr = &song->instr[inst];
        instr->a = fgetc(f); // A
        instr->d = fgetc(f); // D
        instr->s = fgetc(f); // S
        instr->r = fgetc(f); // R

        instr->wav_len = fgetc(f);
        instr->wav_loop = fgetc(f);

        for (int col = 0; col < instr->wav_len; col++) instr->arp[col] = fgetc(f);
        for (int col = 0; col < instr->wav_len; col++) instr->wav[col] = fgetc(f);

        instr->duty_start = fgetc(f);
        instr->duty_start |= fgetc(f)<<8;

        instr->duty_end = fgetc(f);
        instr->duty_end |= fgetc(f)<<8;

        instr->duty_speed = fgetc(f);
        instr->duty_speed |= fgetc(f)<<8;
    }

    fclose(f);
}

void save_file(char *filename, song *song) {
    FILE *f = fopen(filename, "wb");

    // header
    fwrite(magic_string, 1, 8, f); // header
    fputc(song->init_speed, f); // init. speed
    for (int i = 0; i < 7; i++) fputc(0,f); // reserved

    for (int i = 0; i < 32; i++) fputc(0,f); // reserved
    for (int i = 0; i < 32; i++) fputc(0,f); // reserved
    for (int i = 0; i < 32; i++) fputc(0,f); // reserved
    
    for (int i = 0; i < 16; i++) fputc(0,f); // reserved

    // orders
    fputc(song->order_len,f); // table length
    for (int ch = 0; ch < 3; ch++) {
        for (int ord = 0; ord < song->order_len; ord++) {
            fputc(song->order_table[ch][ord], f);
        }
    }

    // patterns
    int max_pat = 0;
    for (int ch = 0; ch < 3; ch++) {
        for (int ord = 0; ord < song->order_len; ord++) {
            if (song->order_table[ch][ord] >= max_pat)
                max_pat = song->order_table[ch][ord];
        }
    }
    fputc(max_pat&0xff, f);
    // write the patterns
    for (int pat = 0; pat < max_pat; pat++) {
        for (int row = 0; row < 64; row++) {
            fputc(song->pattern[pat].rows[row].note, f);
            fputc(song->pattern[pat].rows[row].instr, f);
            fputc(song->pattern[pat].rows[row].eff_type, f);
            fputc(song->pattern[pat].rows[row].eff_arg, f);
        }
    }   

    // instruments (WIP)
    for (int inst = 0; inst < 128; inst++) {
        instrument *instr = &song->instr[inst];
        // i know i could group two nibbles, but i want to futureproof it i guess?
        fputc(instr->a, f); // A
        fputc(instr->d, f); // D
        fputc(instr->s, f); // S
        fputc(instr->r, f); // R

        fputc(instr->wav_len, f);
        fputc(instr->wav_loop, f);

        for (int col = 0; col < instr->wav_len; col++) fputc(instr->arp[col], f);
        for (int col = 0; col < instr->wav_len; col++) fputc(instr->wav[col], f);

        fputc(instr->duty_start&0xff, f);
        fputc(instr->duty_start>>8&0xff, f);

        fputc(instr->duty_end&0xff, f);
        fputc(instr->duty_end>>8&0xff, f);

        fputc(instr->duty_speed&0xff, f);
        fputc(instr->duty_speed>>8&0xff, f);
    }

    fclose(f);
}