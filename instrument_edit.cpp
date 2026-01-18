#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "defines.h"

#define CLAMP(x,y,z) ((x)>(z)?(z):((x)<(y)?(y):(x)))

extern const char* note_str[12];

const char* wave_names[8] = {
    "Noise", "Pulse", "Saw", "Triangle",
    "Test-bit", "Ring-mod", "Hard-sync", "Gate-bit"
};

void render_instr(song *song, cursor *cur_cursor, bool *enable) {
    static bool was_hovering = false;
    static bool was_dragging_wave[128][8];
    static bool click_wav_val = false;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;
    ImVec2 mouse_pos_prev = io.MousePosPrev;
    bool lmb_down = ImGui::IsMouseDown(0);
    bool lmb_clicked = ImGui::IsMouseClicked(0);

    ImGui::Begin("Instrument Editor", enable);

    ImGui::Text("Arp/Wave Macro");

    uint8_t one = 1; // ONE!!11!
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    if (ImGui::InputScalar("##arp_length",ImGuiDataType_U8,&song->instr[0].wav_len, &one, NULL, "%d", 0)) {
        if (song->instr[0].wav_len == 255) song->instr[0].wav_len = 0;
        if (song->instr[0].wav_len >= 128) song->instr[0].wav_len = 127;
    }

    // i know this is not my best friend imgui, i don't fucking care...
    float temp_ypos = ImGui::GetCursorPos().y;

    if (song->instr[0].wav_len != 0) {
        ImGui::BeginTable("insedit_arp",song->instr[0].wav_len,ImGuiTableFlags_ScrollX|ImGuiTableFlags_NoPadInnerX);
        for (int ch = 0; ch < song->instr[0].wav_len; ch++) {
            ImVec2 slider_res = ImVec2(24.0f*io.FontGlobalScale,160.0f*io.FontGlobalScale);
            char ch_id[16];
            snprintf(ch_id,16,"arp%d",ch);
            ImGui::TableSetupColumn(ch_id,ImGuiTableColumnFlags_WidthFixed,slider_res.x+4*io.FontGlobalScale);
        }
        ImGui::TableNextColumn();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        for (int col = 0; col < song->instr[0].wav_len; col++) {
            bool is_abs = song->instr[0].arp[col]&0x80;
            int arp_val;
            if (is_abs) arp_val = song->instr[0].arp[col]&127;
            else arp_val = (song->instr[0].arp[col]&127)-48;
            ImGui::PushID(col+512);
            ImVec2 slider_res = ImVec2(24.0f*io.FontGlobalScale,160.0f*io.FontGlobalScale);
            ImVec2 slider_drag_res = ImVec2(18.0f*io.FontGlobalScale,8.0f*io.FontGlobalScale);

            // FIXME: when using an abs macro on the first column, the text is slightly higher than usual?

            /*
            if (ImGui::VSliderInt("##val",ImVec2(24.0f*io.FontGlobalScale,160.0f*io.FontGlobalScale),&arp_val,is_abs?0:-64,is_abs?127:63,"%d",ImGuislider)) {
                if (is_abs) song->instr[0].arp[col] = arp_val|0x80;
                else song->instr[0].arp[col] = arp_val+64;
            }
            */

            if (is_abs) {
                ImGui::Text("%s%d",note_str[arp_val%12],(arp_val-(arp_val%12))/12);
            } else {
                ImGui::SetNextItemWidth(slider_res.x);
                ImGui::InputInt("##arp_val",&arp_val,0,0);
            }
            
            // draw arp slider
            ImGui::SetCursorPosY(io.FontGlobalScale*24.0);

            int v_min = is_abs?95:47;
            int v_max = is_abs?0:-48;

            float padding = slider_res.y/64.0;
            float arp_norm = (((float)arp_val)-v_min)/(v_max-v_min);
            ImVec2 arp_off = ImVec2(0.0f,arp_norm*(slider_res.y-(slider_drag_res.y+padding*2)));
            ImVec2 slider_bb_a = ImGui::GetCursorScreenPos();
            ImVec2 slider_bb_b = ImGui::GetCursorScreenPos()+slider_res;
            ImRect slider_bb = ImRect(slider_bb_a.x,slider_bb_a.y,slider_bb_b.x,slider_bb_b.y);
            ImVec2 slider_a = ImGui::GetCursorScreenPos()+arp_off+ImVec2((slider_res.x-slider_drag_res.x)/2,padding);
            ImVec2 slider_b = ImGui::GetCursorScreenPos()+arp_off+slider_drag_res+ImVec2((slider_res.x-slider_drag_res.x)/2,padding);
            ImRect slider_rect = ImRect(ImVec4(slider_a.x,slider_a.y,slider_b.x,slider_b.y));

            int x_rel = mouse_pos.x-slider_bb_a.x;
            bool is_in_x_boundary = x_rel >= 0 && x_rel < slider_res.x;
            if (slider_bb.Contains(mouse_pos) || (was_hovering && is_in_x_boundary)) {
                float rel_pos = ((mouse_pos - ImGui::GetCursorScreenPos()).y)/slider_res.y;
                int final_pos = (int)((rel_pos*(v_max-v_min))+v_min);
                if (v_min > v_max) {
                    int temp = v_min;
                    v_min = v_max;
                    v_max = temp;
                }
                final_pos = CLAMP(final_pos,v_min,v_max);
                if (lmb_down) {
                    was_hovering = true;
                    arp_val = final_pos;
                } else {
                    was_hovering = false;
                }
            }

            draw_list->AddRectFilled(ImGui::GetCursorScreenPos(),
                                    ImGui::GetCursorScreenPos()+slider_res,
                                    IM_COL32(0x28,0x4c,0x7c,0xff));
            draw_list->AddRectFilled(slider_a,slider_b,
                                    IM_COL32(0x78,0xac,0xcc,0xff));

            // draw rel/abs checkbox
            ImGui::SetCursorPosY(slider_res.y+io.FontGlobalScale*32.0);
            ImGui::Checkbox("##arp_mode",(bool *)&is_abs);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", is_abs?"Absolute":"Relative");
            if (is_abs) song->instr[0].arp[col] = CLAMP(arp_val,0,95)|0x80;
            else song->instr[0].arp[col] = CLAMP(arp_val,-48,47)+48;

            ImGui::SetCursorPosY(slider_res.y+io.FontGlobalScale*64.0);

            int wav_val = song->instr[0].wav[col];
            int row = 0;
            for (int b = 7; b >= 0; b--) {
                ImRect boundary = ImRect(ImVec4(ImGui::GetCursorScreenPos().x,ImGui::GetCursorScreenPos().y,
                                                ImGui::GetCursorScreenPos().x+slider_res.x,
                                                ImGui::GetCursorScreenPos().y+slider_res.x));
                draw_list->AddRectFilled(boundary.Min, boundary.Max,
                                         IM_COL32(0x28,0x4c,0x7c,0xff));
                if ((wav_val>>b)&1) {
                    draw_list->AddRectFilled(boundary.Min+ImVec2(io.FontGlobalScale*4.0,io.FontGlobalScale*4.0),
                                             boundary.Max-ImVec2(io.FontGlobalScale*4.0,io.FontGlobalScale*4.0),
                                             IM_COL32(0x78,0xac,0xcc,0xff));
                }
                ImGui::Dummy(ImVec2(0.0f,slider_res.x+2+io.FontGlobalScale));
                if (b == 4) ImGui::Dummy(ImVec2(0.0f,2+io.FontGlobalScale));
                if (boundary.Contains(mouse_pos)) {
                    ImGui::SetTooltip("%s", wave_names[row]);
                    if ((lmb_down && !boundary.Contains(mouse_pos_prev) && (((wav_val>>b)&1) == click_wav_val)) || lmb_clicked) { 
                        if (lmb_clicked) click_wav_val = (wav_val>>b)&1;
                        wav_val ^= (1<<b);
                    }
                }
                row++;
            }
            song->instr[0].wav[col] = wav_val;

            ImGui::PopID();
            ImGui::TableNextColumn();
        }
        ImGui::EndTable();
    }
    //ImGui::SetCursorPosY(temp_ypos+io.FontGlobalScale*192.0);
    //ImGui::Separator();
    ImGui::End();
}