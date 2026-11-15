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
#include <SDL2/SDL.h>
#include "defines.h"

SDL_AudioDeviceID dev;
int16_t audio_buffer[BUFFER_SIZE*4];
int16_t audio_buffer_temp[BUFFER_SIZE];
int audio_buffer_read, audio_buffer_write;
uint64_t time_cur;
bool audio_paused;

extern int advance_sample(song *song, cursor *cur_cursor, int16_t *buffer, int buffer_length, float volume); // player.cpp

void audio_callback(void *udata, uint8_t *stream, int len) {
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

// Reset audio buffer
void reset_audio_buffer() {
    audio_buffer_read = 0;
    audio_buffer_write = BUFFER_SIZE;
    time_cur = SDL_GetTicks64();
}

void init_audio() {
    SDL_AudioSpec wanted;
    wanted.freq = SAMPLE_RATE;
    wanted.format = AUDIO_S16;
    wanted.channels = 1;
    wanted.samples = BUFFER_SIZE;
    wanted.callback = audio_callback;
    wanted.userdata = NULL;

    dev = SDL_OpenAudio(&wanted, NULL);
    SDL_PauseAudio(0);
    reset_audio_buffer();
}

void advance_audio(song *song, cursor *cur_cursor) {
    advance_sample(song, cur_cursor, audio_buffer_temp, BUFFER_SIZE, visible_windows.audio_volume);
    memcpy(&audio_buffer[audio_buffer_write], audio_buffer_temp, BUFFER_SIZE*sizeof(short));
            
    audio_buffer_write += BUFFER_SIZE;
            
    if (audio_buffer_write >= (BUFFER_SIZE*4))
        audio_buffer_write -= (BUFFER_SIZE*4);
            
    uint64_t now = SDL_GetTicks64();
    if (time_cur > now) {
        now = time_cur - now;
        SDL_Delay(now);
    }
    time_cur += 1000/50;
}

void free_audio() {
    SDL_PauseAudioDevice(dev, 1);
    SDL_CloseAudioDevice(dev);
}