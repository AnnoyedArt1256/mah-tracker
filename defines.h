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

#include <cstdint>

// Magic numbers for file format
#define NOTE_OFF 0xfe // Note off in pattern
#define NOTE_EMPTY 0xff // Empty cell in pattern
#define INS_NO_LOOP 0xff // Telltale if an instrument "loops"
#define ORDER_END 0xff // End of order

// Pattern editor, for the column cursor position
enum channel_mode {
    nothing = -1,
    note = 0,
    instr = 1, 
    eff_type = 2,
    eff_arg = 3,
    end = 4
};

// Pattern row
struct pat_row {
    //      C-4     01       A        67
    uint8_t note, instr, eff_type, eff_arg;
};

// Pattern data, 64 rows each
struct pattern_data {
    pat_row rows[64];
};

struct pattern_chunk_copy {
    pattern_data ch_rows[3];
    //int row_start;
    int row_len;
    int col_start;
    int col_len;
};

// Cursor for pattern editor
struct cursor {
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
    bool new_file_popup;
    bool chip_mode;
    int drag_pat;
    int drag_x_start;
    int drag_x_end;
    enum channel_mode drag_x_start_sel;
    enum channel_mode drag_x_end_sel;
    int drag_y_start;
    int drag_y_end;
    bool dragging;
    bool already_dragged;
    // TODO: make this less memory-intensive :P
    pattern_chunk_copy pattern_copy_buffer;
};


// Instrument
// Max number of commands in an instrument struct is 128
struct instrument {
    uint8_t a, d, s, r; // ADSR envelope
    uint8_t wav_len; // also for arps
    uint8_t wav_loop; // also for arps
    uint8_t wav[128]; // Waveform select
    uint8_t arp[128]; // Arpeggio
    uint8_t filter_len; // also for arps
    uint8_t filter_loop; // also for arps
    uint8_t filter[128]; // Filter intensity
    uint8_t filter_mode[128]; // Low-pass, band-pass, high-pass
    uint8_t filter_res; // Filter resonance
    bool filter_enable; // Filter enable
    int duty_start; // Pulse duty start position
    int duty_end; // Pulse duty end position
    int duty_speed; // PWM speed
};


// Song
// Pattern, order table, order length, 128 instruments, and initial speed
struct song {
    pattern_data pattern[256];
    uint16_t order_table[3][256];
    uint8_t order_len;
    instrument instr[128];
    uint8_t init_speed;
};

extern void render_pat(song *song, cursor *cur_cursor, bool *enable);
extern void render_orders(song *song, cursor *cur_cursor, bool *enable);
extern void render_instr(song *song, cursor *cur_cursor, bool *enable);