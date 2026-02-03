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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "defines.h"

// Note string definitions.
/* TODO? Maybe? 
// Add flat-rendering support. Some people like flats in their trackers. 
 Those people are weird. */

/*
const char* flats[12] = {
    "C-", "Db", "D-", "Eb", "E-", "F-", "Gb", "G-", "Ab", "A-", "Bb", "B-"

};

*/

const char* note_str[12] = {
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

// Piano keys
ImGuiKey piano_keys[] = {
    ImGuiKey_Z, ImGuiKey_S, ImGuiKey_X, ImGuiKey_D, ImGuiKey_C, ImGuiKey_V,
    ImGuiKey_G, ImGuiKey_B, ImGuiKey_H, ImGuiKey_N, ImGuiKey_J, ImGuiKey_M,
    ImGuiKey_Q, ImGuiKey_2, ImGuiKey_W, ImGuiKey_3, ImGuiKey_E, ImGuiKey_R,
    ImGuiKey_5, ImGuiKey_T, ImGuiKey_6, ImGuiKey_Y, ImGuiKey_7, ImGuiKey_U, 
    ImGuiKey_I, ImGuiKey_9, ImGuiKey_O, ImGuiKey_0, ImGuiKey_P
};

// Hex keys
const ImGuiKey hex_keys[16] = {
    ImGuiKey_0, ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, 
    ImGuiKey_4, ImGuiKey_5, ImGuiKey_6, ImGuiKey_7, 
    ImGuiKey_8, ImGuiKey_9, ImGuiKey_A, ImGuiKey_B, 
    ImGuiKey_C, ImGuiKey_D, ImGuiKey_E, ImGuiKey_F, 
};

extern void init_routine(song *song); // player.cpp
extern void play_note_live(song *song, uint8_t ch, uint8_t note, uint8_t instr);

// get render offset for channel select mode
int get_select_offset(enum channel_mode ch_select) {
    switch (ch_select) {
        case note: { return 0; }
        case instr: { return 4; }
        case eff_type: { return 7; }
        case eff_arg: { return 8; }
        default: return 0;
    }
    return 0;
}

// get selection width, idk
// how many spaces something takes up, check what it is and return how many spaces it should have
int get_select_width(enum channel_mode ch_select) {
    switch (ch_select) {
        case note: { return 3; }
        case instr: { return 2; }
        case eff_type: { return 1; }
        case eff_arg: { return 2; }
        default: return 0;
    }
    return 0;
}

void copy_pat(song *song, cursor *cur_cursor) {
    // skip if the cursor has not already dragged an area at the moment
    if (!cur_cursor->already_dragged) return;

    // get the row and column areas (and swap coords if necessary)
    int row_start = cur_cursor->drag_y_start;
    int row_end = cur_cursor->drag_y_end;
    if (row_start > row_end) {
        int temp = row_end;
        row_end = row_start;
        row_start = temp;
    }
    int row_len = (row_end-row_start)+1;

    int col_start = cur_cursor->drag_x_start;
    int col_end = cur_cursor->drag_x_end;
    if (col_start > col_end) {
        int temp = col_end;
        col_end = col_start;
        col_start = temp;
    }
    int col_len = (col_end-col_start)+1;

    cur_cursor->pattern_copy_buffer.row_len = row_len;
    cur_cursor->pattern_copy_buffer.col_start = col_start;
    cur_cursor->pattern_copy_buffer.col_len = col_len;

    for (int ch = 0; ch < 3; ch++) {
        memcpy(cur_cursor->pattern_copy_buffer.ch_rows[ch].rows,
               &song->pattern[song->order_table[ch][cur_cursor->order]].rows[row_start],
               sizeof(pat_row)*row_len);
    }
}

void paste_pat(song *song, cursor *cur_cursor) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 char_size_xy = ImGui::CalcTextSize("A");
    char_size_xy.y += io.FontGlobalScale+2;

    pattern_chunk_copy *copy_buffer = &cur_cursor->pattern_copy_buffer;
    int cursor_row = cur_cursor->row;
    for (int row = 0; row < copy_buffer->row_len; row++) {
        if ((row+cursor_row) >= 64) break;
        for (int col = copy_buffer->col_start; col < copy_buffer->col_start+copy_buffer->col_len; col++) {
            // i could use memcpy, but just in case someone's using big-endian or smth...
            int rel_col = col-copy_buffer->col_start;
            pat_row *cur_pat_rows = song->pattern[song->order_table[cur_cursor->ch+(rel_col>>2)][cur_cursor->order]].rows;
            switch (col&3) {
                case 0: cur_pat_rows[row+cursor_row].note = copy_buffer->ch_rows[col>>2].rows[row].note; break;
                case 1: cur_pat_rows[row+cursor_row].instr = copy_buffer->ch_rows[col>>2].rows[row].instr; break;
                case 2: cur_pat_rows[row+cursor_row].eff_type = copy_buffer->ch_rows[col>>2].rows[row].eff_type; break;
                case 3: cur_pat_rows[row+cursor_row].eff_arg = copy_buffer->ch_rows[col>>2].rows[row].eff_arg; break;
            }
        }
        cur_cursor->row++;
        if (cur_cursor->row >= 64) cur_cursor->row = 63;
    }
    ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);   
}

void clear_pat_selection(song *song, cursor *cur_cursor) {
    // skip if the cursor has not already dragged an area at the moment
    if (!cur_cursor->already_dragged) return;

    // get the row and column areas (and swap coords if necessary)
    int row_start = cur_cursor->drag_y_start;
    int row_end = cur_cursor->drag_y_end;
    if (row_start > row_end) {
        int temp = row_end;
        row_end = row_start;
        row_start = temp;
    }
    int row_len = (row_end-row_start)+1;

    int col_start = cur_cursor->drag_x_start;
    int col_end = cur_cursor->drag_x_end;
    if (col_start > col_end) {
        int temp = col_end;
        col_end = col_start;
        col_start = temp;
    }
    int col_len = (col_end-col_start)+1;
    
    int cursor_row = row_start;
    for (int row = 0; row < row_len; row++) {
        if ((row+cursor_row) >= 64) break;
        for (int col = col_start; col < col_start+col_len; col++) {
            // i could use memcpy, but just in case someone's using big-endian or smth...
            int rel_col = col-col_start;
            pat_row *cur_pat_rows = song->pattern[song->order_table[cur_cursor->ch+(rel_col>>2)][cur_cursor->order]].rows;
            switch (col&3) {
                case 0: cur_pat_rows[row+cursor_row].note = NOTE_EMPTY; break;
                case 1: cur_pat_rows[row+cursor_row].instr = 0; break;
                case 2: cur_pat_rows[row+cursor_row].eff_type = 0; break;
                case 3: cur_pat_rows[row+cursor_row].eff_arg = 0; break;
            }
        }
    }
}

// Row/column controls
void do_pat_keyboard(song *song, cursor *cur_cursor) {
    static uint8_t last_eff_type;
    static uint8_t last_eff_arg;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 char_size_xy = ImGui::CalcTextSize("A");
    char_size_xy.y += io.FontGlobalScale+2;
    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        // set jam/record mode
        cur_cursor->do_record = !cur_cursor->do_record;
    }

    // Copy-paste function
    bool ctrl_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
    if (ctrl_pressed) {
        if (ImGui::IsKeyPressed(ImGuiKey_C)) copy_pat(song, cur_cursor);
        if (ImGui::IsKeyPressed(ImGuiKey_V)) paste_pat(song, cur_cursor);
    }

    // Down one row
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        cur_cursor->latch = 0;
        cur_cursor->already_dragged = false;
        cur_cursor->dragging = false;
        cur_cursor->row = (cur_cursor->row+1)%64;
        ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
    }

    // Up one row
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        cur_cursor->latch = 0;
        cur_cursor->already_dragged = false;
        cur_cursor->dragging = false;
        cur_cursor->row--;
        if (cur_cursor->row < 0) cur_cursor->row = 64-1;
        ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
    }

    // Column right
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        cur_cursor->latch = 0;
        cur_cursor->already_dragged = false;
        cur_cursor->dragging = false;
        switch (cur_cursor->selection) {
            case note: {
                cur_cursor->selection = instr;
                break;
            }
            case instr: {
                cur_cursor->selection = eff_type;
                break;
            }
            case eff_type: {
                cur_cursor->selection = eff_arg;
                break;
            }
            case eff_arg: {
                cur_cursor->selection = note;
                cur_cursor->ch = (cur_cursor->ch+1)%3;
                break;
            }
            default: break;
        }
    }

    // Column left
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        cur_cursor->latch = 0;
        cur_cursor->already_dragged = false;
        cur_cursor->dragging = false;
        switch (cur_cursor->selection) {
            case note: {
                cur_cursor->selection = eff_arg;
                if (--cur_cursor->ch < 0) {
                    cur_cursor->ch = 3-1;
                }
                break;
            }
            case instr: {
                cur_cursor->selection = note;
                break;
            }
            case eff_type: {
                cur_cursor->selection = instr;
                break;
            }
            case eff_arg: {
                cur_cursor->selection = eff_type;
                break;
            }
            default: break;
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && cur_cursor->already_dragged) {
        // clear dragged selection if backspace is pressed (thx theduccinator for the advice!)
        clear_pat_selection(song, cur_cursor);
    }

    pat_row *cur_pattern_rows = song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows;

    if (cur_cursor->selection == note) {
        for (int key_ind = 0; key_ind < sizeof(piano_keys)/sizeof(ImGuiKey); key_ind++) {
            if (ImGui::IsKeyPressed(piano_keys[key_ind]) && !ctrl_pressed) {
                if (cur_cursor->do_record) {
                    cur_pattern_rows[cur_cursor->row].note = 
                        cur_cursor->octave*12+key_ind;
                    cur_pattern_rows[cur_cursor->row].instr = 
                        cur_cursor->instr;
                    cur_cursor->row = (cur_cursor->row+1)%64;
                    ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
                }
                cur_cursor->latch = 0;
                cur_cursor->already_dragged = false;
                cur_cursor->dragging = false;
                play_note_live(song,0,cur_cursor->octave*12+key_ind,cur_cursor->instr);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_1)) {
            if (cur_cursor->do_record) {
                cur_pattern_rows[cur_cursor->row].note = 
                    NOTE_OFF;
                cur_cursor->row = (cur_cursor->row+1)%64;
                cur_cursor->latch = 0;
                cur_cursor->already_dragged = false;
                cur_cursor->dragging = false;
                ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
            }   
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !cur_cursor->already_dragged) {
            cur_pattern_rows[cur_cursor->row].note = 
                NOTE_EMPTY;
            cur_pattern_rows[cur_cursor->row].instr = 0;
            cur_cursor->latch = 0;
            //cur_cursor->row = (cur_cursor->row+1)%64;
            //ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
        }
    }
    if (cur_cursor->selection == instr) {
        for (int key = 0; key < 16; key++) {
            if (ImGui::IsKeyPressed(hex_keys[key]) && cur_cursor->do_record && !ctrl_pressed) {
                cur_cursor->already_dragged = false;
                cur_cursor->dragging = false;
                if (cur_cursor->latch) {
                    cur_pattern_rows[cur_cursor->row].instr <<= 4;
                    cur_pattern_rows[cur_cursor->row].instr |= key;
                    cur_cursor->latch = 0;
                    cur_cursor->row = (cur_cursor->row+1)%64;
                    ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
                } else {
                    cur_pattern_rows[cur_cursor->row].instr = key;
                    cur_cursor->latch = 1;
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !cur_cursor->already_dragged) {
            cur_pattern_rows[cur_cursor->row].instr = 0;
            cur_cursor->latch = 0;
        }
    }
    if (cur_cursor->selection == eff_type) {
        for (int key = 0; key < 16; key++) {
            if (ImGui::IsKeyPressed(hex_keys[key]) && cur_cursor->do_record && !ctrl_pressed) {
                cur_cursor->already_dragged = false;
                cur_cursor->dragging = false;
                cur_pattern_rows[cur_cursor->row].eff_type = key;
                if (last_eff_type == key) {
                    cur_pattern_rows[cur_cursor->row].eff_arg = last_eff_arg;
                } else {
                    last_eff_arg = 0;
                }
                last_eff_type = key;
                cur_cursor->latch = 0;
                cur_cursor->row = (cur_cursor->row+1)%64;
                ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !cur_cursor->already_dragged) {
            cur_pattern_rows[cur_cursor->row].eff_type = 0;
            cur_cursor->latch = 0;
        }
    }
    if (cur_cursor->selection == eff_arg) {
        for (int key = 0; key < 16; key++) {
            if (ImGui::IsKeyPressed(hex_keys[key]) && cur_cursor->do_record && !ctrl_pressed) {
                cur_cursor->already_dragged = false;
                cur_cursor->dragging = false;
                if (cur_cursor->latch) {
                    cur_pattern_rows[cur_cursor->row].eff_arg <<= 4;
                    cur_pattern_rows[cur_cursor->row].eff_arg |= key;
                    last_eff_arg = cur_pattern_rows[cur_cursor->row].eff_arg;
                    cur_cursor->latch = 0;
                    cur_cursor->row = (cur_cursor->row+1)%64;
                    ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
                } else {
                    last_eff_arg = cur_pattern_rows[cur_cursor->row].eff_arg = key;
                    cur_cursor->latch = 1;
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !cur_cursor->already_dragged) {
            cur_pattern_rows[cur_cursor->row].eff_arg = 0;
            cur_cursor->latch = 0;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        cur_cursor->playing ^= 1;
        cur_cursor->play_row = 0;
        cur_cursor->latch = 0;
        init_routine(song);
    }
}

void render_pat(song *song, cursor *cur_cursor, bool *enable) {
    // init window and table
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("Pattern", enable);

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    // only render the table if it's on screen...
    bool render_table = ImGui::BeginTable("patview",3+2,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoPadInnerX); 
    ImVec2 char_size_xy = ImGui::CalcTextSize("A");
    float char_size = char_size_xy.x; // from foiniss
    char_size_xy.y += io.FontGlobalScale+2;
    const float ch_row_len = 12.0; // C-4 69 420
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (!render_table) {
        // this is so the program doesn't crash when the pattern window is a hidden tab
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }

    ImGui::TableSetupColumn("row_pos",ImGuiTableColumnFlags_WidthFixed,4.0*char_size);
    for (int ch = 0; ch < 3; ch++) {
        char ch_id[16];
        snprintf(ch_id,16,"ch%d",ch);
        ImGui::TableSetupColumn(ch_id,ImGuiTableColumnFlags_WidthFixed,ch_row_len*char_size-1);
    }
    ImGui::TableSetupColumn("row_end",ImGuiTableColumnFlags_WidthFixed,char_size);

    // get the amount of dummy rows
    int dummy_row_cnt = (int)(ImGui::GetWindowSize().y/(2.0*char_size_xy.y));

    // draw row highlights
    ImVec2 c = ImGui::GetCursorScreenPos();
    c.x += char_size_xy.x*4.0;
    c.y += char_size_xy.y*dummy_row_cnt;
    ImGui::TableNextRow(0,char_size_xy.y);
    for (int row = 0; row < 64; row += 4) {
        draw_list->AddRectFilled(ImVec2(c.x, c.y), 
                                 ImVec2(c.x+char_size_xy.x*(ch_row_len*3.0),c.y+char_size_xy.y),
                                 IM_COL32(0x20,0x2c,0x35,0xff));
        c.y += char_size_xy.y*4.0;
    }

    if (cur_cursor->playing) {
        c = ImGui::GetCursorScreenPos();
        c.x += char_size_xy.x*4.0;
        c.y += char_size_xy.y*dummy_row_cnt;
        c.y += char_size_xy.y*cur_cursor->play_row;

        draw_list->AddRectFilled(ImVec2(c.x, c.y), 
                                 ImVec2(c.x+char_size_xy.x*(ch_row_len*3.0),c.y+char_size_xy.y),
                                 IM_COL32(0x40,0x4c,0x55,0xff));
    }

    if (cur_cursor->do_record) {
        c = ImGui::GetCursorScreenPos();
        c.x += char_size_xy.x*4.0;
        c.y += char_size_xy.y*dummy_row_cnt;
        c.y += char_size_xy.y*cur_cursor->row;

        draw_list->AddRectFilled(ImVec2(c.x, c.y), 
                                 ImVec2(c.x+char_size_xy.x*(ch_row_len*3.0),c.y+char_size_xy.y),
                                 IM_COL32(0x30,0x1b,0x1b,0xff));
    }

    if (cur_cursor->already_dragged) {
        ImVec2 c_start = ImGui::GetCursorScreenPos();
        c_start.x += char_size_xy.x*5.0; // offset it correctly
        c_start.x += get_select_offset(cur_cursor->drag_x_start_sel)*char_size_xy.x+1.0;
        c_start.x += floorf(cur_cursor->drag_x_start/4)*12*char_size_xy.x;
        c_start.y += cur_cursor->drag_y_start*char_size_xy.y;
        c_start.y += dummy_row_cnt*char_size_xy.y;
        if (cur_cursor->drag_x_start > cur_cursor->drag_x_end)
            c_start += ImVec2(get_select_width(cur_cursor->drag_x_start_sel)*char_size_xy.x, 0.0f);
        if (cur_cursor->drag_y_start > cur_cursor->drag_y_end)
            c_start += ImVec2(0.0f, char_size_xy.y);

        ImVec2 c_end = ImGui::GetCursorScreenPos();
        c_end.x += char_size_xy.x*5.0; // offset it correctly
        c_end.x += get_select_offset(cur_cursor->drag_x_end_sel)*char_size_xy.x+1.0;
        c_end.x += floorf(cur_cursor->drag_x_end/4)*12*char_size_xy.x;
        c_end.y += cur_cursor->drag_y_end*char_size_xy.y;
        c_end.y += dummy_row_cnt*char_size_xy.y;
        if (cur_cursor->drag_x_start <= cur_cursor->drag_x_end)
            c_end += ImVec2(get_select_width(cur_cursor->drag_x_end_sel)*char_size_xy.x, char_size_xy.y);
    
        draw_list->AddRectFilled(c_start, c_end,
                                 IM_COL32(0x66,0x66,0x72,0x67));
    }

    // get mouse
    if (ImGui::IsWindowFocused()) {
        // get the absolute coords OF THE WINDOW
        float x = ImGui::GetMousePos().x+ImGui::GetScrollX()+1.0;
        float y = ImGui::GetMousePos().y+ImGui::GetScrollY();
        x -= ImGui::GetWindowPos().x;
        y -= ImGui::GetWindowPos().y;
        x -= char_size_xy.x*5.0;
        int char_x = (int)floorf(x/char_size_xy.x);
        int char_y = (int)floorf(y/char_size_xy.y);
        char_y -= dummy_row_cnt;

        // get channel selection type
        int ch = char_x/12;
        enum channel_mode ch_select = nothing;
        switch (char_x%12) {
            case 0:
            case 1:
            case 2:
            case 3: {
                ch_select = note;
                break;
            }
            case 4:
            case 5: 
            case 6: {
                ch_select = instr;
                break;
            }
            case 7: {
                ch_select = eff_type;
                break;
            }
            case 8:
            case 9: 
            case 10: {
                ch_select = eff_arg;
            }
            default: break; // wtf
        }

        //printf("%02d %02d %d\n",ch,char_y,ch_select);
        if (ch >= 0 && ch < 3 && char_y >= 0 && char_y < 64
            && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
            if (ImGui::IsMouseClicked(0)) {
                cur_cursor->ch = ch;
                cur_cursor->row = char_y;
                cur_cursor->selection = ch_select;
                cur_cursor->latch = 0;
                cur_cursor->dragging = false;
                cur_cursor->already_dragged = false;
                cur_cursor->drag_x_start = ch*4+((int)ch_select);
                cur_cursor->drag_y_start = char_y;
                cur_cursor->drag_x_start_sel = ch_select;
                ImGui::SetScrollY(char_y*char_size_xy.y);
            }
        }
        // the mouse dragging code (eventually for copy+paste)
        if (ImGui::IsMouseDragging(0) && (!ImGui::IsMouseClicked(0))) {
            if (ch_select != nothing) {
                cur_cursor->dragging = true;
                cur_cursor->already_dragged = true;
                cur_cursor->drag_x_end = ch*4+((int)ch_select);
                cur_cursor->drag_y_end = char_y;
                cur_cursor->drag_x_end_sel = ch_select;  
                // clamp the drag coords
                if ((cur_cursor->drag_x_end/4) >= 3) {
                    cur_cursor->drag_x_end = 4*3-1;
                    cur_cursor->drag_x_end_sel = eff_arg;
                }
                if ((cur_cursor->drag_x_end/4) < 0) { // just in case
                    cur_cursor->drag_x_end = 0;
                    cur_cursor->drag_x_end_sel = note;
                }
                if (cur_cursor->drag_y_end >= 63) cur_cursor->drag_y_end = 63;
                if (cur_cursor->drag_y_end < 0) cur_cursor->drag_y_end = 0;
            }
            if ((ImGui::GetMousePos().y-ImGui::GetWindowPos().y) >= (ImGui::GetWindowHeight()-char_size_xy.y*1.5)
                && (fabs(io.MouseDelta.y) > 0 || fabs(io.MouseDelta.x) > 0)) {
                cur_cursor->latch = 0;
                if (cur_cursor->row < 63) cur_cursor->row++;
                ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
            }
            if ((ImGui::GetMousePos().y-ImGui::GetWindowPos().y) <= char_size_xy.y*1.5
                && (fabs(io.MouseDelta.y) > 0 || fabs(io.MouseDelta.x) > 0)) {
                cur_cursor->latch = 0;
                if (cur_cursor->row > 0) cur_cursor->row--;
                ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
            }
        }
        if (ImGui::IsMouseReleased(0) && cur_cursor->dragging) {
            cur_cursor->dragging = false;
            cur_cursor->ch = cur_cursor->drag_x_end/4;
            cur_cursor->selection = cur_cursor->drag_x_end_sel;
            cur_cursor->row = cur_cursor->drag_y_end;
        }
    }

    if (ImGui::IsWindowFocused()) do_pat_keyboard(song, cur_cursor);

    if (cur_cursor->ch >= 0 && cur_cursor->ch < 3) {
        // render SELECTED cursor
        ImVec2 c = ImGui::GetCursorScreenPos();
        c.x += char_size_xy.x*5.0; // offset it correctly
        c.x += get_select_offset(cur_cursor->selection)*char_size_xy.x+1.0;
        c.x += (cur_cursor->ch*12)*char_size_xy.x;
        c.y += cur_cursor->row*char_size_xy.y;
        c.y += dummy_row_cnt*char_size_xy.y;
        draw_list->AddRectFilled(ImVec2(c.x, c.y), 
                                 ImVec2(c.x+get_select_width(cur_cursor->selection)*char_size_xy.x,
                                        c.y+char_size_xy.y),
                                 IM_COL32(0x28,0x4c,0x7c,0xff));
    }

    for (int dummy_row = 0; dummy_row < dummy_row_cnt; dummy_row++) {
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        for (int ch = 0; ch < 3; ch++) {
            ImGui::TableNextColumn();
        }
        ImGui::TableNextRow(0,char_size_xy.y);
    }

    for (int row = 0; row < 64; row++) {
        ImGui::TableNextColumn();
        ImGui::Text(" %02X ", row);
        ImGui::TableNextColumn();
        for (int ch = 0; ch < 3; ch++) {
            // C-4 01 4xx
            char cur_note[11] = "... .. ...";
            uint8_t note     = song->pattern[song->order_table[ch][cur_cursor->order]].rows[row].note;
            uint8_t instr    = song->pattern[song->order_table[ch][cur_cursor->order]].rows[row].instr;
            uint8_t eff_type = song->pattern[song->order_table[ch][cur_cursor->order]].rows[row].eff_type;
            uint8_t eff_arg  = song->pattern[song->order_table[ch][cur_cursor->order]].rows[row].eff_arg;
            if (note != NOTE_EMPTY) {
                if (note != NOTE_OFF) {
                    memcpy(cur_note,note_str[note%12],2);
                    cur_note[2] = "0123456789ABCDEF"[(note-(note%12))/12];
                } else {
                    memcpy(cur_note,"OFF",3);
                }
            }
            if (instr != 0) {
                const char *hex_str = "0123456789ABCDEF";
                cur_note[4] = hex_str[instr>>4];
                cur_note[5] = hex_str[instr&0xf];
            }
            if (eff_type != 0) {
                const char *hex_str = "0123456789ABCDEF";
                cur_note[7] = hex_str[eff_type&0xf];
            }
            if (eff_arg != 0) {
                const char *hex_str = "0123456789ABCDEF";
                cur_note[8] = hex_str[eff_arg>>4];
                cur_note[9] = hex_str[eff_arg&0xf];
            }
            ImGui::Text(" %s ",cur_note);
            ImGui::TableNextColumn();
        }
        ImGui::TableNextRow(0,char_size_xy.y);
    }

    for (int dummy_row = 0; dummy_row < dummy_row_cnt; dummy_row++) {
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        for (int ch = 0; ch < 3; ch++) {
            ImGui::TableNextColumn();
        }
        ImGui::TableNextRow(0,char_size_xy.y);
    }

    ImGui::EndTable();
    ImGui::PopStyleVar();
    ImGui::End();
}
