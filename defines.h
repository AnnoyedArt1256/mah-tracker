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

enum channel_mode {
    nothing = -1,
    note = 0,
    instr = 1, 
    eff_type = 2,
    eff_arg = 3,
    end = 4
};

typedef struct {
    int ch, row;
    enum channel_mode selection;
    int octave;
    int latch;
    int order;
    int instr;
    int playing;
    int play_row;
    bool loop;
    bool do_record;
} cursor;

#define NOTE_OFF 0xfe
#define NOTE_EMPTY 0xff
typedef struct {
    uint8_t note, instr, eff_type, eff_arg;
} pat_row;

typedef struct {
    pat_row rows[64];
} pattern;

#define INS_NO_LOOP 0xff
typedef struct {
    uint8_t a, d, s, r;
    uint8_t wav_len; // also for arps
    uint8_t wav_loop; // also for arps
    uint8_t wav[128];
    uint8_t arp[128];
    uint8_t filter_len; // also for arps
    uint8_t filter_loop; // also for arps
    uint8_t filter[128];
    uint8_t filter_mode[128];
    uint8_t filter_res;
    int duty_start;
    int duty_end;
    int duty_speed;
} instrument;

#define ORDER_END 0xff
typedef struct {
    pattern pattern[256];
    uint16_t order_table[3][256];
    uint8_t order_len;
    instrument instr[128];
    uint8_t init_speed;
} song;

extern void render_pat(song *song, cursor *cur_cursor, bool *enable);
extern void render_orders(song *song, cursor *cur_cursor, bool *enable);
extern void render_instr(song *song, cursor *cur_cursor, bool *enable);