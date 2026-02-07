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

#define rightClickable if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetKeyboardFocusHere(-1);
#define CLAMP(x,y,z) ((x)>(z)?(z):((x)<(y)?(y):(x)))

extern const char* note_str[12];

// Waveform names
const char* wave_names[8] = {
    "Noise", "Pulse", "Saw", "Triangle",
    "Test Bit", "Ring Mod", "Hard Sync", "Gate Bit"
};

// Filter names
const char* filt_names[3] = {
    "High-pass", "Band-pass", "Low-pass"
};

extern void play_note_live(song *song, uint8_t ch, uint8_t note, uint8_t instr);
extern ImGuiKey piano_keys[29];

// Render instrument editor
void render_instr(song *song, cursor *cur_cursor, bool *enable) {
    static bool was_hovering = false;
    static bool was_dragging_wave[128][8];
    static bool click_wav_val = false;
    ImGuiIO& io = ImGui::GetIO();
    // For use in instrument copy & paste
    static instrument instr_copy = {};
    
    ImVec2 mouse_pos = io.MousePos;
    ImVec2 mouse_pos_prev = io.MousePosPrev;
    bool lmb_down = ImGui::IsMouseDown(0);
    bool lmb_clicked = ImGui::IsMouseClicked(0);

    ImGui::Begin("Instrument Editor", enable);

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        // live playing
        for (int key_ind = 0; key_ind < sizeof(piano_keys)/sizeof(ImGuiKey); key_ind++) {
            if (ImGui::IsKeyPressed(piano_keys[key_ind])) {
                play_note_live(song,0,cur_cursor->octave*12+key_ind,cur_cursor->instr);
            }
        }
    }

    char ins_name_preview[32];
    snprintf(ins_name_preview,32,"Insturment %02X",cur_cursor->instr);
    if (ImGui::BeginCombo("##ins_select",ins_name_preview)) {
        for (int n = 1; n < 128; n++) {
            char ins_name_select[32];
            snprintf(ins_name_select,32,"Insturment %02X",n);
            const bool is_selected = (cur_cursor->instr == n);
            if (ImGui::Selectable(ins_name_select, is_selected))
                cur_cursor->instr = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy")) {
        instr_copy = song->instr[cur_cursor->instr];
    }
    ImGui::SameLine();
    if (ImGui::Button("Paste")) {
        song->instr[cur_cursor->instr] = instr_copy;
    }

    if (ImGui::BeginTabBar("tabs_i_guess")) {
        if (ImGui::BeginTabItem("ADSR")) { 
            uint8_t zero = 0; // WHY
            uint8_t fifteen = 15; // WHYY!Y!Y!!Y
            ImGui::SliderScalar("Attack" ,ImGuiDataType_U8,&song->instr[cur_cursor->instr].a,&zero,&fifteen); rightClickable
            ImGui::SliderScalar("Decay"  ,ImGuiDataType_U8,&song->instr[cur_cursor->instr].d,&zero,&fifteen); rightClickable
            ImGui::SliderScalar("Sustain",ImGuiDataType_U8,&song->instr[cur_cursor->instr].s,&zero,&fifteen); rightClickable
            ImGui::SliderScalar("Release",ImGuiDataType_U8,&song->instr[cur_cursor->instr].r,&zero,&fifteen); rightClickable
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Arp/Waveform Macro")) {
            uint8_t one = 1; // ONE!!11!
            ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
            if (ImGui::InputScalar("Length",ImGuiDataType_U8,&song->instr[cur_cursor->instr].wav_len, &one, NULL, "%d", 0)) {
                if (song->instr[cur_cursor->instr].wav_len == 255) song->instr[cur_cursor->instr].wav_len = 0;
                if (song->instr[cur_cursor->instr].wav_len >= 128) song->instr[cur_cursor->instr].wav_len = 127;
            }
            bool is_loop = song->instr[cur_cursor->instr].wav_loop != INS_NO_LOOP;
            if (ImGui::Checkbox("Loop",&is_loop)) {
                if (is_loop) {
                    song->instr[cur_cursor->instr].wav_loop = 0;              
                } else {
                    song->instr[cur_cursor->instr].wav_loop = INS_NO_LOOP;
                }
            }
            if (is_loop) {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
                if (ImGui::InputScalar("Loop Position",ImGuiDataType_U8,&song->instr[cur_cursor->instr].wav_loop, &one, NULL, "%d", 0)) {
                    if (song->instr[cur_cursor->instr].wav_loop == 255) song->instr[cur_cursor->instr].wav_loop = 0;
                    if (song->instr[cur_cursor->instr].wav_loop >= 128) song->instr[cur_cursor->instr].wav_loop = 127;
                }  
            }
            

            // i know this is not my best friend imgui, i don't fucking care...
            float temp_ypos = ImGui::GetCursorPos().y;

            ImVec2 table_pos_start = ImGui::GetCursorScreenPos(); // these two vars are used
            ImVec2 table_pos_size = ImGui::GetContentRegionAvail(); // for clipping the label texts in the macros

            if (song->instr[cur_cursor->instr].wav_len != 0) {
                if (ImGui::BeginTable("insedit_arp",song->instr[cur_cursor->instr].wav_len,ImGuiTableFlags_ScrollX|ImGuiTableFlags_NoPadInnerX)) {    
                    for (int ch = 0; ch < song->instr[cur_cursor->instr].wav_len; ch++) {
                        ImVec2 slider_res = ImVec2(24.0f*io.FontGlobalScale,160.0f*io.FontGlobalScale);
                        char ch_id[16];
                        snprintf(ch_id,16,"arp%d",ch);
                        ImGui::TableSetupColumn(ch_id,ImGuiTableColumnFlags_WidthFixed,slider_res.x+4*io.FontGlobalScale);
                    }
                    ImGui::TableNextColumn();

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    for (int col = 0; col < song->instr[cur_cursor->instr].wav_len; col++) {
                        bool is_abs = song->instr[cur_cursor->instr].arp[col]&0x80;
                        int arp_val;
                        if (is_abs) arp_val = song->instr[cur_cursor->instr].arp[col]&127;
                        else arp_val = (song->instr[cur_cursor->instr].arp[col]&127)-48;
                        ImGui::PushID(col+512);
                        ImVec2 slider_res = ImVec2(24.0f*io.FontGlobalScale,160.0f*io.FontGlobalScale);
                        ImVec2 slider_drag_res = ImVec2(18.0f*io.FontGlobalScale,8.0f*io.FontGlobalScale);

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
                        if ((slider_bb.Contains(mouse_pos) || (was_hovering && is_in_x_boundary)) &&
                            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
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
                        if (is_abs) song->instr[cur_cursor->instr].arp[col] = CLAMP(arp_val,0,95)|0x80;
                        else song->instr[cur_cursor->instr].arp[col] = CLAMP(arp_val,-48,47)+48;

                        ImGui::SetCursorPosY(slider_res.y+io.FontGlobalScale*64.0);

                        int wav_val = song->instr[cur_cursor->instr].wav[col];
                        int row = 0;
                        for (int b = 7; b >= 0; b--) {
                            ImRect boundary = ImRect(ImVec4(ImGui::GetCursorScreenPos().x,ImGui::GetCursorScreenPos().y,
                                                            ImGui::GetCursorScreenPos().x+slider_res.x,
                                                            ImGui::GetCursorScreenPos().y+slider_res.x));

                            draw_list->AddRectFilled(boundary.Min, boundary.Max, // radio button base
                                                    IM_COL32(0x28,0x4c,0x7c,0xff));

                            if ((wav_val>>b)&1) { // radio button selection
                                draw_list->AddRectFilled(boundary.Min+ImVec2(io.FontGlobalScale*4.0,io.FontGlobalScale*4.0),
                                                        boundary.Max-ImVec2(io.FontGlobalScale*4.0,io.FontGlobalScale*4.0),
                                                        IM_COL32(0x78,0xac,0xcc,0xff));
                            }

                            if (col == 0) {
                                // add text over the radio buttons so the 
                                // user actually knows what the buttons do :meatjob:
                                ImGuiStyle& style = ImGui::GetStyle();
                                ImGui::PushClipRect(ImMax(boundary.Min,table_pos_start),
                                                    ImMin(boundary.Max+ImVec2(slider_res.x*4.0,0.0f),table_pos_start+table_pos_size-ImVec2(style.ScrollbarSize,0.0f)),false);
                                ImGui::Text("%s",wave_names[row]);
                                ImGui::SameLine();
                                ImGui::PopClipRect();
                            }

                            ImGui::Dummy(ImVec2(0.0f,slider_res.x+2+io.FontGlobalScale));
                            if (b == 4) ImGui::Dummy(ImVec2(0.0f,2+io.FontGlobalScale));
                            if (boundary.Contains(mouse_pos) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
                                ImGui::SetTooltip("%s", wave_names[row]);
                                if ((lmb_down && !boundary.Contains(mouse_pos_prev) && (((wav_val>>b)&1) == click_wav_val)) || lmb_clicked) { 
                                    if (lmb_clicked) click_wav_val = (wav_val>>b)&1;
                                    wav_val ^= (1<<b);
                                }
                            }
                            row++;
                        }
                        song->instr[cur_cursor->instr].wav[col] = wav_val;

                        ImGui::PopID();
                        ImGui::TableNextColumn();
                    }
                    ImGui::EndTable();
                }
            }
            //ImGui::SetCursorPosY(temp_ypos+io.FontGlobalScale*192.0);
            //ImGui::Separator();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Duty Macro")) {
            ImGui::SliderInt("Duty Start",&song->instr[cur_cursor->instr].duty_start,0,4095); rightClickable
            ImGui::SliderInt("Duty End",&song->instr[cur_cursor->instr].duty_end,0,4095); rightClickable
            ImGui::Separator();
            ImGui::SliderInt("Duty Speed",&song->instr[cur_cursor->instr].duty_speed,-2048,2047); rightClickable
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Filter Macro")) {
            uint8_t zero = 0; // WHY
            uint8_t one = 1; // ONE!!11!
            uint8_t fifteen = 15; // WHY
            ImGui::SliderScalar("Resonance",ImGuiDataType_U8,&song->instr[cur_cursor->instr].filter_res,&zero,&fifteen); rightClickable
            ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
            if (ImGui::InputScalar("Length",ImGuiDataType_U8,&song->instr[cur_cursor->instr].filter_len, &one, NULL, "%d", 0)) {
                if (song->instr[cur_cursor->instr].filter_len == 255) song->instr[cur_cursor->instr].filter_len = 0;
                if (song->instr[cur_cursor->instr].filter_len >= 128) song->instr[cur_cursor->instr].filter_len = 127;
            }
            ImGui::SameLine();
            ImGui::Checkbox("Enable Filter",&song->instr[cur_cursor->instr].filter_enable);

            bool is_loop = song->instr[cur_cursor->instr].filter_loop != INS_NO_LOOP;
            if (ImGui::Checkbox("Loop",&is_loop)) {
                if (is_loop) {
                    song->instr[cur_cursor->instr].filter_loop = 0;              
                } else {
                    song->instr[cur_cursor->instr].filter_loop = INS_NO_LOOP;
                }
            }
            if (is_loop) {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
                if (ImGui::InputScalar("Loop Position",ImGuiDataType_U8,&song->instr[cur_cursor->instr].filter_loop, &one, NULL, "%d", 0)) {
                    if (song->instr[cur_cursor->instr].filter_loop == 255) song->instr[cur_cursor->instr].filter_loop = 0;
                    if (song->instr[cur_cursor->instr].filter_loop >= 128) song->instr[cur_cursor->instr].filter_loop = 127;
                }  
            }


            float temp_ypos = ImGui::GetCursorPos().y;

            ImVec2 table_pos_start = ImGui::GetCursorScreenPos(); // these two vars are used
            ImVec2 table_pos_size = ImGui::GetContentRegionAvail(); // for clipping the label texts in the macros

            if (song->instr[cur_cursor->instr].filter_len != 0) {
                if (ImGui::BeginTable("insedit_filter",song->instr[cur_cursor->instr].filter_len,ImGuiTableFlags_ScrollX|ImGuiTableFlags_NoPadInnerX)) {
                    for (int ch = 0; ch < song->instr[cur_cursor->instr].filter_len; ch++) {
                        ImVec2 slider_res = ImVec2(24.0f*io.FontGlobalScale,160.0f*io.FontGlobalScale);
                        char ch_id[16];
                        snprintf(ch_id,16,"filt%d",ch);
                        ImGui::TableSetupColumn(ch_id,ImGuiTableColumnFlags_WidthFixed,slider_res.x+4*io.FontGlobalScale);
                    }
                    ImGui::TableNextColumn();

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    for (int col = 0; col < song->instr[cur_cursor->instr].filter_len; col++) {
                        int filt_val = song->instr[cur_cursor->instr].filter[col];
                        ImGui::PushID(col+512);
                        ImVec2 slider_res = ImVec2(24.0f*io.FontGlobalScale,160.0f*io.FontGlobalScale);
                        ImVec2 slider_drag_res = ImVec2(18.0f*io.FontGlobalScale,8.0f*io.FontGlobalScale);

                        ImGui::SetNextItemWidth(slider_res.x);
                        ImGui::InputInt("##filt_val",&filt_val,0,0);
                        
                        // draw filter bar slider

                        int v_min = 255;
                        int v_max = 0;

                        float padding = slider_res.y/64.0;
                        float filt_norm = (((float)filt_val)-v_min)/(v_max-v_min);
                        ImVec2 filt_off = ImVec2(0.0f,filt_norm*(slider_res.y-slider_drag_res.y+padding));
                        ImVec2 slider_bb_a = ImGui::GetCursorScreenPos();
                        ImVec2 slider_bb_b = ImGui::GetCursorScreenPos()+slider_res;
                        ImRect slider_bb = ImRect(slider_bb_a.x,slider_bb_a.y,slider_bb_b.x,slider_bb_b.y);
                        ImVec2 slider_a = ImGui::GetCursorScreenPos()+filt_off+ImVec2((slider_res.x-slider_drag_res.x)/2,padding);
                        ImVec2 slider_b = ImGui::GetCursorScreenPos()+ImVec2(slider_res.x-(slider_res.x-slider_drag_res.x)/2,slider_res.y-padding);
                        ImRect slider_rect = ImRect(ImVec4(slider_a.x,slider_a.y,slider_b.x,slider_b.y));

                        int x_rel = mouse_pos.x-slider_bb_a.x;
                        bool is_in_x_boundary = x_rel >= 0 && x_rel < slider_res.x;
                        if ((slider_bb.Contains(mouse_pos) || (was_hovering && is_in_x_boundary)) &&
                            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
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
                                filt_val = final_pos;
                            } else {
                                was_hovering = false;
                            }
                        }

                        draw_list->AddRectFilled(ImGui::GetCursorScreenPos(),
                                                ImGui::GetCursorScreenPos()+slider_res,
                                                IM_COL32(0x28,0x4c,0x7c,0xff));
                        if (filt_val) {
                            // HACK: make the filter slider NOT render at 0 cutoff
                            draw_list->AddRectFilled(slider_a,slider_b,
                                                    IM_COL32(0x78,0xac,0xcc,0xff));
                        }
                        song->instr[cur_cursor->instr].filter[col] = CLAMP(filt_val,0,255);

                        // draw the filter mode bitfields
                        ImGui::SetCursorPosY(slider_res.y+io.FontGlobalScale*32.0);
                        int mode_val = song->instr[cur_cursor->instr].filter_mode[col];
                        int row = 0;
                        for (int b = 2+4; b >= 0+4; b--) {
                            ImRect boundary = ImRect(ImVec4(ImGui::GetCursorScreenPos().x,ImGui::GetCursorScreenPos().y,
                                                            ImGui::GetCursorScreenPos().x+slider_res.x,
                                                            ImGui::GetCursorScreenPos().y+slider_res.x));

                            draw_list->AddRectFilled(boundary.Min, boundary.Max, // radio button base
                                                    IM_COL32(0x28,0x4c,0x7c,0xff));
                            if ((mode_val>>b)&1) { // radio button selection
                                draw_list->AddRectFilled(boundary.Min+ImVec2(io.FontGlobalScale*4.0,io.FontGlobalScale*4.0),
                                                        boundary.Max-ImVec2(io.FontGlobalScale*4.0,io.FontGlobalScale*4.0),
                                                        IM_COL32(0x78,0xac,0xcc,0xff));
                            }

                            if (col == 0) {
                                // add text over the radio buttons so the 
                                // user actually knows what the buttons do :meatjob:
                                ImGuiStyle& style = ImGui::GetStyle();
                                ImGui::PushClipRect(ImMax(boundary.Min,table_pos_start),
                                                    ImMin(boundary.Max+ImVec2(slider_res.x*4.0,0.0f),table_pos_start+table_pos_size-ImVec2(style.ScrollbarSize,0.0f)),false);
                                ImGui::Text("%s",filt_names[row]);
                                ImGui::SameLine();
                                ImGui::PopClipRect();
                            }

                            ImGui::Dummy(ImVec2(0.0f,slider_res.x+2+io.FontGlobalScale));
                            if (boundary.Contains(mouse_pos) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
                                ImGui::SetTooltip("%s", filt_names[row]);
                                if ((lmb_down && !boundary.Contains(mouse_pos_prev) && (((mode_val>>b)&1) == click_wav_val)) || lmb_clicked) { 
                                    if (lmb_clicked) click_wav_val = (mode_val>>b)&1;
                                    mode_val ^= (1<<b);
                                    printf("%02x\n",mode_val);
                                }
                            }
                            row++;
                        }
                        song->instr[cur_cursor->instr].filter_mode[col] = mode_val;
        
                        ImGui::PopID();
                        ImGui::TableNextColumn();
                    }
                    ImGui::EndTable();
                }
            }
            //ImGui::SetCursorPosY(temp_ypos+io.FontGlobalScale*192.0);
            //ImGui::Separator();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}