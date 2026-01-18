#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "defines.h"

const char* note_str[12] = {
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

const ImGuiKey piano_keys[] = {
    ImGuiKey_Z, ImGuiKey_S, ImGuiKey_X, ImGuiKey_D, ImGuiKey_C, ImGuiKey_V,
    ImGuiKey_G, ImGuiKey_B, ImGuiKey_H, ImGuiKey_N, ImGuiKey_J, ImGuiKey_M,
    ImGuiKey_Q, ImGuiKey_2, ImGuiKey_W, ImGuiKey_3, ImGuiKey_E, ImGuiKey_R,
    ImGuiKey_5, ImGuiKey_T, ImGuiKey_6, ImGuiKey_Y, ImGuiKey_7, ImGuiKey_U, 
    ImGuiKey_I, ImGuiKey_9, ImGuiKey_O, ImGuiKey_0, ImGuiKey_P
};

const ImGuiKey hex_keys[16] = {
    ImGuiKey_0, ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, 
    ImGuiKey_4, ImGuiKey_5, ImGuiKey_6, ImGuiKey_7, 
    ImGuiKey_8, ImGuiKey_9, ImGuiKey_A, ImGuiKey_B, 
    ImGuiKey_C, ImGuiKey_D, ImGuiKey_E, ImGuiKey_F, 
};


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

void do_pat_keyboard(song *song, cursor *cur_cursor) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 char_size_xy = ImGui::CalcTextSize("A");
    char_size_xy.y += io.FontGlobalScale+2;
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        cur_cursor->latch = 0;
        cur_cursor->row = (cur_cursor->row+1)%32;
        ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        cur_cursor->latch = 0;
        cur_cursor->row--;
        if (cur_cursor->row < 0) cur_cursor->row = 32-1;
        ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        cur_cursor->latch = 0;
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
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        cur_cursor->latch = 0;
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
    if (cur_cursor->selection == note) {
        for (int key_ind = 0; key_ind < sizeof(piano_keys)/sizeof(ImGuiKey); key_ind++) {
            if (ImGui::IsKeyPressed(piano_keys[key_ind])) {
                song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].note = 
                    cur_cursor->octave*12+key_ind;
                cur_cursor->row = (cur_cursor->row+1)%32;
                ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
                cur_cursor->latch = 0;
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
            song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].note = 
                NOTE_EMPTY;
            cur_cursor->latch = 0;
            //cur_cursor->row = (cur_cursor->row+1)%32;
            //ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
        }
    }
    if (cur_cursor->selection == instr) {
        for (int key = 0; key < 16; key++) {
            if (ImGui::IsKeyPressed(hex_keys[key])) {
                if (cur_cursor->latch) {
                    song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].instr <<= 4;
                    song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].instr |= key;
                    cur_cursor->latch = 0;
                    cur_cursor->row = (cur_cursor->row+1)%32;
                    ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
                } else {
                    song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].instr = key;
                    cur_cursor->latch = 1;
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
            song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].instr = 0;
            cur_cursor->latch = 0;
        }
    }
    if (cur_cursor->selection == eff_type) {
        for (int key = 0; key < 16; key++) {
            if (ImGui::IsKeyPressed(hex_keys[key])) {
                song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].eff_type = key;
                cur_cursor->latch = 0;
                cur_cursor->row = (cur_cursor->row+1)%32;
                ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
            song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].eff_type = 0;
            cur_cursor->latch = 0;
        }
    }
    if (cur_cursor->selection == eff_arg) {
        for (int key = 1; key < 16; key++) {
            if (ImGui::IsKeyPressed(hex_keys[key])) {
                if (cur_cursor->latch) {
                    song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].eff_arg <<= 4;
                    song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].eff_arg |= key;
                    cur_cursor->latch = 0;
                    cur_cursor->row = (cur_cursor->row+1)%32;
                    ImGui::SetScrollY(cur_cursor->row*char_size_xy.y);
                } else {
                    song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].eff_arg = key;
                    cur_cursor->latch = 1;
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
            song->pattern[song->order_table[cur_cursor->ch][cur_cursor->order]].rows[cur_cursor->row].eff_arg = 0;
            cur_cursor->latch = 0;
        }
    }
}

void render_pat(song *song, cursor *cur_cursor, bool *enable) {
    // init window and table
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("Pattern", enable);

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    ImGui::BeginTable("patview",3+2,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoPadInnerX);

    ImVec2 char_size_xy = ImGui::CalcTextSize("A");
    float char_size = char_size_xy.x; // from foiniss
    char_size_xy.y += io.FontGlobalScale+2;
    const float ch_row_len = 12.0; // C-4 69 420
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

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
    for (int row = 0; row < 32; row += 4) {
        draw_list->AddRectFilled(ImVec2(c.x, c.y), 
                                 ImVec2(c.x+char_size_xy.x*(ch_row_len*3.0),c.y+char_size_xy.y),
                                 IM_COL32(0x20,0x2c,0x35,0xff));
        c.y += char_size_xy.y*4.0;
    }

    // get mouse
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsWindowFocused()) {
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

        printf("%02d %02d %d\n",ch,char_y,ch_select);
        if (ch >= 0 && ch < 3 && char_y >= 0 && char_y < 32) {
            if (ImGui::IsMouseClicked(0)) {
                cur_cursor->ch = ch;
                cur_cursor->row = char_y;
                cur_cursor->selection = ch_select;
                cur_cursor->latch = 0;
                ImGui::SetScrollY(char_y*char_size_xy.y);
            }
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

    for (int row = 0; row < 32; row++) {
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
                memcpy(cur_note,note_str[note%12],2);
                cur_note[2] = "0123456789ABCDEF"[(note-(note%12))/12];
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
