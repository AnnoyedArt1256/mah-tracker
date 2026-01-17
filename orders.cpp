#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "defines.h"

int get_unused_pattern(song *song) {
    int pat_val = 0;
    for (int i = 0; i < 256; i++) {
        bool used = false;
        for (int ch = 0; ch < 3; ch++) {
            for (int j = 0; j < song->order_len; j++) {
                if (song->order_table[ch][j] == i) {
                    used = true;
                    break;
                }
            }
        }
        if (!used) {
            pat_val = i;
            break;
        }
    }
    return pat_val;
}

void remove_pattern(song *song, int pat_ind) {
    for (int ch = 0; ch < 3; ch++) {
        int len = song->order_len-(pat_ind+1)-1;
        printf("%d\n",len);
        for (int pat = pat_ind; pat < song->order_len; pat++) {
            song->order_table[ch][pat] = song->order_table[ch][pat+1];
        }
    }
    song->order_len--;
}

void render_orders(song *song, cursor *cur_cursor, bool *enable) {
    // init window and table
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("Orders", enable);

    if (ImGui::Button("Add")) {
        if (song->order_len < 255) {
            int pat_ind = cur_cursor->order+1;
            for (int ch = 0; ch < 3; ch++) {
                for (int pat = song->order_len; pat > pat_ind; pat--) {
                    song->order_table[ch][pat] = song->order_table[ch][pat-1];
                }
                song->order_table[ch][pat_ind] = get_unused_pattern(song);
            }
            song->order_len++;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove")) {
        if (song->order_len > 1) remove_pattern(song, cur_cursor->order);
    }
    ImGui::SameLine();
    if (ImGui::Button("Duplicate")) {
        if (song->order_len < 255) {
            int pat_ind = cur_cursor->order+1;
            for (int ch = 0; ch < 3; ch++) {
                for (int pat = song->order_len; pat > pat_ind; pat--) {
                    song->order_table[ch][pat] = song->order_table[ch][pat-1];
                }
                song->order_table[ch][pat_ind] = song->order_table[ch][pat_ind-1];
            }
            song->order_len++;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Append")) {
        if (song->order_len < 255) {
            int order_ind = song->order_len++;
            for (int ch = 0; ch < 3; ch++) {
                song->order_table[ch][order_ind] = get_unused_pattern(song);
            }
        }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    ImGui::BeginTable("orderview",3+1,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoPadInnerX);

    ImVec2 char_size_xy = ImGui::CalcTextSize("A");
    float char_size = char_size_xy.x; // from foiniss
    char_size_xy.y += io.FontGlobalScale+2;
    float order_ch_size = (ImGui::GetWindowSize().x-(4.0*char_size))/3;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGui::TableSetupColumn("ord_pos",ImGuiTableColumnFlags_WidthFixed,4.0*char_size);
    for (int ch = 0; ch < 3; ch++) {
        char ch_id[16];
        snprintf(ch_id,16,"och%d",ch);
        ImGui::TableSetupColumn(ch_id,ImGuiTableColumnFlags_WidthFixed,order_ch_size-1);
    }

    { // render current order row
        ImVec2 c = ImGui::GetCursorScreenPos();
        c.y += char_size_xy.y*cur_cursor->order;
        draw_list->AddRectFilled(ImVec2(c.x, c.y), 
                                 ImVec2(c.x+ImGui::GetWindowSize().x,c.y+char_size_xy.y),
                                 IM_COL32(0x20,0x2c,0x45,0xff));
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsWindowFocused()) {
        // get the absolute coords OF THE WINDOW
        float x = ImGui::GetMousePos().x+ImGui::GetScrollX()+1.0;
        float y = ImGui::GetMousePos().y+ImGui::GetScrollY();
        x -= ImGui::GetWindowPos().x;
        y -= ImGui::GetWindowPos().y;
        x -= char_size_xy.x*4.0;
        int ch = (int)floorf(x/order_ch_size);
        int order_row = (int)floorf(y/char_size_xy.y);

        // render current hovering cell
        ImVec2 c = ImGui::GetCursorScreenPos();
        if (ch != -1) c.x += 4.0*char_size+order_ch_size*ch;
    
        // set cell width for rendering
        float cell_width = ch == -1 ? 4.0*char_size : order_ch_size;
        c.y += char_size_xy.y*order_row;
        if (order_row < song->order_len) {
            draw_list->AddRectFilled(ImVec2(c.x, c.y), 
                                     ImVec2(c.x+cell_width,c.y+char_size_xy.y),
                                     IM_COL32(0x40,0x4c,0x65,0xff));
        }
    
        if (ch == -1 && ImGui::IsMouseClicked(0) && order_row < song->order_len) {
            cur_cursor->order = order_row;
        } else if (ch >= 0 && ch < 3) {
            if (ImGui::IsMouseClicked(0) && song->order_table[ch][order_row] < 255)
                song->order_table[ch][order_row]++;
            if (ImGui::IsMouseClicked(1) && song->order_table[ch][order_row] > 0)
                song->order_table[ch][order_row]--;
        }
    }

    ImGui::TableNextRow(0,char_size_xy.y);

    for (int order = 0; order < song->order_len; order++) {
        ImGui::TableNextColumn();
        ImGui::Text(" %02X", order);
        ImGui::TableNextColumn();
        for (int ch = 0; ch < 3; ch++) {
            ImGui::SameLine(order_ch_size/2.0-char_size);
            ImGui::Text("%02X", song->order_table[ch][order]);
            ImGui::TableNextColumn();
        }
        ImGui::TableNextRow(0,char_size_xy.y);
    }

    ImGui::EndTable();
    ImGui::PopStyleVar();
    ImGui::End();
}