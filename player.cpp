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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "defines.h"
#include "siddefs-fp.h"
#include "SID.h"

SDL_AudioDeviceID dev;
#define SAMPLE_RATE 48000
#define BUFFER_SIZE (960) // 44100/60
reSIDfp::SID* sid_fp;

int16_t audio_buffer[BUFFER_SIZE*4];
int16_t audio_buffer_temp[BUFFER_SIZE];
int audio_buffer_read, audio_buffer_ind, audio_buffer_write;
int frame_cnt;
uint32_t time_cur;
#define CLAMP(x,y,z) ((x)>(z)?(z):((x)<(y)?(y):(x)))

// Frequency table LOW
unsigned char freqtbllo[] = {
  0x17,0x27,0x39,0x4b,0x5f,0x74,0x8a,0xa1,0xba,0xd4,0xf0,0x0e,
  0x2d,0x4e,0x71,0x96,0xbe,0xe8,0x14,0x43,0x74,0xa9,0xe1,0x1c,
  0x5a,0x9c,0xe2,0x2d,0x7c,0xcf,0x28,0x85,0xe8,0x52,0xc1,0x37,
  0xb4,0x39,0xc5,0x5a,0xf7,0x9e,0x4f,0x0a,0xd1,0xa3,0x82,0x6e,
  0x68,0x71,0x8a,0xb3,0xee,0x3c,0x9e,0x15,0xa2,0x46,0x04,0xdc,
  0xd0,0xe2,0x14,0x67,0xdd,0x79,0x3c,0x29,0x44,0x8d,0x08,0xb8,
  0xa1,0xc5,0x28,0xcd,0xba,0xf1,0x78,0x53,0x87,0x1a,0x10,0x71,
  0x42,0x89,0x4f,0x9b,0x74,0xe2,0xf0,0xa6,0x0e,0x33,0x20,0xff,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

// Frequency table HIGH
unsigned char freqtblhi[] = {
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,
  0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x04,
  0x04,0x04,0x04,0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,0x08,
  0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0c,0x0d,0x0d,0x0e,0x0f,0x10,
  0x11,0x12,0x13,0x14,0x15,0x17,0x18,0x1a,0x1b,0x1d,0x1f,0x20,
  0x22,0x24,0x27,0x29,0x2b,0x2e,0x31,0x34,0x37,0x3a,0x3e,0x41,
  0x45,0x49,0x4e,0x52,0x57,0x5c,0x62,0x68,0x6e,0x75,0x7c,0x83,
  0x8b,0x93,0x9c,0xa5,0xaf,0xb9,0xc4,0xd0,0xdd,0xea,0xf8,0xff,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

// Reset audio buffer
void reset_audio_buffer() {
    audio_buffer_read = 0;
    audio_buffer_write = BUFFER_SIZE;
    time_cur = SDL_GetTicks();
}

bool audio_paused;

void callback(void *udata, uint8_t *stream, int len) {
    SDL_memset(stream, 0, len);

    if (audio_paused) return;

    if (len > ((BUFFER_SIZE)*sizeof(short))) len = (BUFFER_SIZE)*sizeof(short); // clamp length

    SDL_MixAudioFormat(
        stream, 
        (const Uint8 *)(&audio_buffer[audio_buffer_read]), 
        AUDIO_S16SYS, 
        len, 
        SDL_MIX_MAXVOLUME
    );

    audio_buffer_read += len>>1;
    if (audio_buffer_read >= (BUFFER_SIZE*4))
        audio_buffer_read -= (BUFFER_SIZE*4);
}

int audio_cycles = 0;

// for the rest of the player to interface with residfp...
void init_sid() {
    frame_cnt = 0;
    audio_cycles = 0;

    sid_fp=new reSIDfp::SID;
    sid_fp->setChipModel(reSIDfp::MOS8580);
    sid_fp->setSamplingParameters(985248.0,reSIDfp::DECIMATE,985248.0,0.0);
    sid_fp->reset();
    sid_fp->clockSilent(30000);

    SDL_AudioSpec wanted;
    wanted.freq = SAMPLE_RATE;
    wanted.format = AUDIO_S16;
    wanted.channels = 1;
    wanted.samples = BUFFER_SIZE;
    wanted.callback = callback;
    wanted.userdata = NULL;

    dev = SDL_OpenAudio(&wanted, NULL);
    SDL_PauseAudio(0);
    reset_audio_buffer();
}

uint8_t sid_regs[0x20];

// SID write
void write_sid(uint8_t addr, uint8_t val) {
    sid_regs[addr&0x1f] = val;
    sid_fp->write(addr,val);
}

// SID read
uint8_t read_sid(uint8_t addr) {
    return sid_fp->read(addr);
}

void free_sid() {
    SDL_PauseAudioDevice(dev, 1);
    SDL_CloseAudioDevice(dev);
    if (sid_fp!=NULL) delete sid_fp;
}

short sid_buf[8];
void SID_advance_clock() {
    sid_fp->clock(1,sid_buf);
}

short SID_advance_sample() {
    return sid_buf[0];
}

void SID_set_chip(bool mode) {
    sid_fp->setChipModel(mode?reSIDfp::MOS8580:reSIDfp::MOS6581);
}

void advance_frame(song *song, cursor *cur_cursor);
extern float get_volume();

void advance_audio(song *song, cursor *cur_cursor) {
    const static int sid_rate_inc = ((int)(((double)(985248)/(SAMPLE_RATE))*256));
    while (1) {
        SID_advance_clock();
        audio_cycles += 256;
        if (audio_cycles >= sid_rate_inc) {
            audio_cycles -= sid_rate_inc;
            if (++frame_cnt == (SAMPLE_RATE/50)) {
                frame_cnt = 0;
                advance_frame(song, cur_cursor);
            }
            audio_buffer_temp[audio_buffer_ind++] = CLAMP(SID_advance_sample()*get_volume(),-32767,32767);
            if (audio_buffer_ind == BUFFER_SIZE) {
                audio_buffer_ind = 0;
                memcpy(&audio_buffer[audio_buffer_write], 
                    audio_buffer_temp,
                    BUFFER_SIZE*sizeof(short));
            
                audio_buffer_write += BUFFER_SIZE;
            
                if (audio_buffer_write >= (BUFFER_SIZE*4))
                audio_buffer_write -= (BUFFER_SIZE*4);
            
                int now = SDL_GetTicks();
                if (time_cur > now) {
                    now = time_cur - now;
                    SDL_Delay(now);
                }
                time_cur += 1000/50;
                return;
            }
        }
    } 
}

struct pvars {
    uint8_t tick;
    uint8_t speed;
    uint8_t hr_delay[3];
    uint8_t cur_note[3];
    uint8_t inst[3];
    uint8_t cur_arpwave_pos[3];
    uint16_t freq[3];
    uint16_t bend[3];
    uint16_t bend_delta[3];
    uint16_t pw[3];
    uint16_t pw_speed[3];
    uint8_t cur_filter_pos;
    uint8_t filter_inst;
    uint8_t filter_advance;
    uint8_t resonance_ch_enable;
    uint8_t cutoff;
    uint8_t filt_mode;
    uint8_t vol;
    uint8_t gate_mask[3];
    uint8_t vib_arg[3];
    uint8_t vib_tim[3];
    uint8_t last_eff[3];
    uint8_t last_arg[3];
};

pvars player_vars;

// SID init routine
void init_routine(song *song) {
    memset((void *)&player_vars,0,sizeof(pvars));
    player_vars.speed = song->init_speed;
    player_vars.tick = player_vars.speed;
    memset(&player_vars.hr_delay,0xFF,3);
    memset(&player_vars.inst,0,3);
    memset(&player_vars.gate_mask,0xFF,3);
    for (int i = 0; i <= 0x18; i++) write_sid(i,0);
    write_sid(0x18, 0x0F);
    player_vars.vol = 0x0F;
    player_vars.filt_mode = 0x00;
    reset_audio_buffer();
}



// Live note playback
void play_note_live(song *song, uint8_t ch, uint8_t note, uint8_t instr) {
    player_vars.inst[ch] = instr;
    player_vars.cur_note[ch] = note;
    player_vars.cur_arpwave_pos[ch] = 0;
    player_vars.hr_delay[ch] = 0;
    player_vars.pw[ch] = song->instr[player_vars.inst[ch]].duty_start;
    player_vars.pw_speed[ch] = song->instr[player_vars.inst[ch]].duty_speed;
    write_sid(ch*7+5, 0x00);
    write_sid(ch*7+6, 0x00);
    write_sid(ch*7+4, 0x08);
}

void advance_frame(song *song, cursor *cur_cursor) {
    if (cur_cursor->playing) {
        if (--player_vars.tick == 0) {
            bool do_order_skip = false;
            bool did_order_skip = false;
            player_vars.tick = player_vars.speed;
            uint8_t row = cur_cursor->play_row;
            for (int ch = 0; ch < 3; ch++) {
                uint8_t note     = song->pattern[song->order_table[ch][cur_cursor->order]].rows[row].note;
                uint8_t instr    = song->pattern[song->order_table[ch][cur_cursor->order]].rows[row].instr;
                uint8_t eff_type = song->pattern[song->order_table[ch][cur_cursor->order]].rows[row].eff_type;
                uint8_t eff_arg  = song->pattern[song->order_table[ch][cur_cursor->order]].rows[row].eff_arg;
                if (note != NOTE_EMPTY) {
                    if (note == NOTE_OFF) {
                        player_vars.gate_mask[ch] = 0xFE; // gate off
                    } else {
                        player_vars.cur_note[ch] = note;
                        if (eff_type != 3) {
                            player_vars.cur_arpwave_pos[ch] = 0;
                            player_vars.hr_delay[ch] = 0;
                            player_vars.pw[ch] = song->instr[player_vars.inst[ch]].duty_start;
                            player_vars.pw_speed[ch] = song->instr[player_vars.inst[ch]].duty_speed;
                            player_vars.bend[ch] = 0;
                            player_vars.vib_tim[ch] = 0;
                            write_sid(ch*7+5, 0x00);
                            write_sid(ch*7+6, 0x00);
                            write_sid(ch*7+4, 0x08);
                        }
                    }
                }
                if (instr != 0) {
                    player_vars.inst[ch] = instr;
                    if (eff_type != 3) {
                        player_vars.cur_arpwave_pos[ch] = 0;
                        player_vars.hr_delay[ch] = 0;
                        player_vars.pw[ch] = song->instr[player_vars.inst[ch]].duty_start;
                        player_vars.pw_speed[ch] = song->instr[player_vars.inst[ch]].duty_speed;
                        write_sid(ch*7+5, 0x00);
                        write_sid(ch*7+6, 0x00);
                        write_sid(ch*7+4, 0x08);
                    }
                }
                player_vars.bend_delta[ch] = 0;
                player_vars.vib_arg[ch] = 0;
                if (eff_type) {
                    switch (eff_type) {
                        case 0x1: {
                            player_vars.bend_delta[ch] = eff_arg;
                            player_vars.vib_tim[ch] = 0;
                            break;
                        }
                        case 0x2: {
                            player_vars.bend_delta[ch] = -eff_arg;
                            player_vars.vib_tim[ch] = 0;
                            break;
                        }
                        case 0x4: {
                            if (player_vars.last_eff[ch] != eff_type)
                                player_vars.vib_tim[ch] = 0;
                            player_vars.vib_arg[ch] = eff_arg;
                            break;
                        }
                        case 0xF: {
                            player_vars.speed = eff_arg&0x7f;
                            player_vars.tick = player_vars.speed;
                            break;
                        }
                        case 0xD: {
                            do_order_skip = true;
                            break;
                        }
                        default: break;
                    }
                }
                player_vars.last_eff[ch] = eff_type;
                player_vars.last_arg[ch] = eff_arg;
            }
            cur_cursor->play_row++;
            if (cur_cursor->play_row == 64) {
                cur_cursor->play_row = 0;
                if (!cur_cursor->loop) {
                    cur_cursor->order++;
                    if (cur_cursor->order >= song->order_len) {
                        cur_cursor->order = 0;
                    }
                    did_order_skip = true;
                }
            }
            if (do_order_skip && (!did_order_skip)) {
                cur_cursor->play_row = 0;
                if (!cur_cursor->loop) {
                    cur_cursor->order++;
                    if (cur_cursor->order >= song->order_len) {
                        cur_cursor->order = 0;
                    }
                }
            }
        }
    }
    for (int ch = 0; ch < 3; ch++) {
        uint8_t inst = player_vars.inst[ch];
        if (inst == 0) continue;
        if (player_vars.hr_delay[ch] != 0xFF) {
            if (player_vars.hr_delay[ch]++ == 1) {
                player_vars.hr_delay[ch] = 0xFF;
                player_vars.gate_mask[ch] = 0xFF; // gate on
                if (player_vars.last_eff[ch] == 0x05)
                    write_sid(ch*7+5, player_vars.last_arg[ch]);
                else
                    write_sid(ch*7+5, (song->instr[inst].a<<4)|song->instr[inst].d);

                if (player_vars.last_eff[ch] == 0x06)
                    write_sid(ch*7+6, player_vars.last_arg[ch]);
                else
                    write_sid(ch*7+6, (song->instr[inst].s<<4)|song->instr[inst].r);

                if (song->instr[inst].filter_enable) {
                    uint8_t resonance = song->instr[inst].filter_res;
                    player_vars.resonance_ch_enable = (player_vars.resonance_ch_enable&0x0f)|(resonance<<4);
                    player_vars.resonance_ch_enable |= 1<<ch;
                    if (song->instr[inst].filter_len != 0) {
                        player_vars.filter_inst = inst;
                        player_vars.cur_filter_pos = 0;
                        player_vars.filter_advance = true;
                    }
                } else {
                    player_vars.resonance_ch_enable &= ~(1<<ch);
                }
            }
        }
        if (player_vars.hr_delay[ch] == 0xFF) {
            uint8_t arp_pos = player_vars.cur_arpwave_pos[ch];
            uint8_t arp_val = song->instr[inst].arp[arp_pos];
            if (arp_val&0x80) arp_val &= 0x7f; // abs
            else arp_val = player_vars.cur_note[ch] + (arp_val-48); // rel
            player_vars.freq[ch] = freqtbllo[arp_val&127]|(freqtblhi[arp_val&127]<<8);
            write_sid(ch*7+4,song->instr[inst].wav[arp_pos] & player_vars.gate_mask[ch]);
            player_vars.cur_arpwave_pos[ch]++;
            if (player_vars.cur_arpwave_pos[ch] >= song->instr[inst].wav_len) {
                uint8_t loop = song->instr[inst].wav_loop;
                player_vars.cur_arpwave_pos[ch] = loop == INS_NO_LOOP ? song->instr[inst].wav_len-1 : loop;
            }
            player_vars.pw[ch] += player_vars.pw_speed[ch];
            if (player_vars.pw[ch] >= song->instr[inst].duty_end) {
                player_vars.pw[ch] = song->instr[inst].duty_end;
                player_vars.pw_speed[ch] *= -1;
            }
            if (player_vars.pw[ch] <= song->instr[inst].duty_start) {
                player_vars.pw[ch] = song->instr[inst].duty_start;
                player_vars.pw_speed[ch] *= -1;
            }
        }
        if (player_vars.vib_arg[ch]) {
            // thanks goattracker! :meatjob:
            uint8_t vib_depth = (player_vars.vib_arg[ch]<<4)&0xf0;
            uint8_t vib_speed = (player_vars.vib_arg[ch]>>4)^0x0f;
            if ((player_vars.vib_tim[ch] < 0x80) && (player_vars.vib_tim[ch] >= vib_speed)) {
                player_vars.vib_tim[ch] ^= 0xff;
            }
            player_vars.vib_tim[ch] += 2;
            if (player_vars.vib_tim[ch] & 1) player_vars.bend[ch] -= vib_depth;
            else player_vars.bend[ch] += vib_depth;
        }
        player_vars.bend[ch] += player_vars.bend_delta[ch];
        uint16_t final_freq = player_vars.freq[ch]+player_vars.bend[ch];
        write_sid(ch*7+0,final_freq&0xff);
        write_sid(ch*7+1,final_freq>>8);
        write_sid(ch*7+2,player_vars.pw[ch]&0xff);
        write_sid(ch*7+3,player_vars.pw[ch]>>8);
    }
    if (player_vars.filter_advance) {
        // advance filter table
        uint8_t inst = player_vars.filter_inst;
        player_vars.cutoff = song->instr[inst].filter[player_vars.cur_filter_pos];
        player_vars.filt_mode = song->instr[inst].filter_mode[player_vars.cur_filter_pos]&0x70;
        player_vars.cur_filter_pos++;
        if (player_vars.cur_filter_pos >= song->instr[inst].filter_len) {
            uint8_t loop_pos = song->instr[inst].filter_loop;
            if (loop_pos == 0xFF) player_vars.filter_advance = false;
            else player_vars.cur_filter_pos = loop_pos;
        }
    }
    write_sid(0x16,player_vars.cutoff);
    write_sid(0x17,player_vars.resonance_ch_enable);
    write_sid(0x18,player_vars.filt_mode|player_vars.vol);
}

void register_view(bool *open) {
    ImGui::Begin("Register View", open);
    for (int i = 0; i <= 0x18; i++) {
        ImGui::Text("%02x",sid_regs[i]);
        if ((i%7) != 6) ImGui::SameLine();
    }
    ImGui::End();
}