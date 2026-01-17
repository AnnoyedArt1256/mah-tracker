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
} cursor;

#define NOTE_OFF 0xfe
#define NOTE_EMPTY 0xff
typedef struct {
    uint8_t note, instr, eff_type, eff_arg;
} pat_row;

typedef struct {
    pat_row rows[32];
} pattern;

#define ORDER_END 0xff
typedef struct {
    pattern pattern[256];
    uint16_t order_table[3][256];
} song;

extern void render_pat(song *song, cursor *cur_cursor, bool *enable);