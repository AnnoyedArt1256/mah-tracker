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

#define GL_GLEXT_PROTOTYPES
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <SDL.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
// TODO: add glext include here
#else
#include <SDL_opengl.h>
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#define rightClickable if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetKeyboardFocusHere(-1);

void save_settings();
void load_settings();

struct window_bool {
    float audio_volume;
    bool settings;
    bool imgui_debugger;
    bool pattern;
    bool controls;
    bool orders;
    bool instr;
    bool reg_view;
};

struct window_bool visible_windows;

static bool opt_padding = false; // Is there padding (a blank space) between the window edge and the Dockspace?

void ShowExampleAppDockSpace(bool* p_open) {
    // Variables to configure the Dockspace example.
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // Config flags for the Dockspace

    // In this example, we're embedding the Dockspace into an invisible parent window to make it more configurable.
    // We set ImGuiWindowFlags_NoDocking to make sure the parent isn't dockable into because this is handled by the Dockspace.
    //
    // ImGuiWindowFlags_MenuBar is to show a menu bar with config options. This isn't necessary to the functionality of a
    // Dockspace, but it is here to provide a way to change the configuration flags interactively.
    // You can remove the MenuBar flag if you don't want it in your app, but also remember to remove the code which actually
    // renders the menu bar, found at the end of this function.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    // Is the example in Fullscreen mode?
    {
        // If so, get the main viewport:
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Set the parent window's position, size, and viewport to match that of the main viewport. This is so the parent window
        // completely covers the main viewport, giving it a "full-screen" feel.
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        // Set the parent window's styles to match that of the main viewport:
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // No corner rounding on the window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // No border around the window

        // Manipulate the window flags to make it inaccessible to the user (no titlebar, resize/move, or navigation)
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so the parent window should not have its own background:
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // If the padding option is disabled, set the parent window's padding size to 0 to effectively hide said padding.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::Begin("DockSpace Demo", p_open, window_flags);

    // Remove the padding configuration - we pushed it, now we pop it:
    if (!opt_padding)
        ImGui::PopStyleVar();

    // Pop the two style rules set in Fullscreen mode - the corner rounding and the border size.
    ImGui::PopStyleVar(2);

    // Check if Docking is enabled:
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        // If it is, draw the Dockspace with the DockSpace() function.
        // The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    else
    {
        // Docking is DISABLED - Show a warning message
    }

    // This is to show the menu bar that will change the config settings at runtime.
    // If you copied this demo function into your own code and removed ImGuiWindowFlags_MenuBar at the top of the function,
    // you should remove the below if-statement as well.
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("file")) {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            if (ImGui::MenuItem("open file")) {
                /*
                audio_paused = true;
                auto f = pfd::open_file("Choose a file to read", pfd::path::home(),
                            { "Mah-Tracker Files", "*.mah" },
                            pfd::opt::none).result();
                if (!f.empty()) {
                    load_file((char *)f[0].c_str());
                    audio_paused = false;
                    reset_audio_buffer_and_unpause();
                } else {
                    audio_paused = false;
                    reset_audio_buffer_and_unpause();
                }
                */
            }

            ImGui::EndMenu();
        }

        ImGui::MenuItem("settings", NULL, (bool *)&visible_windows.settings);

        if (ImGui::BeginMenu("windows")) {
            ImGui::MenuItem("ImGui Debugger", NULL, (bool *)&visible_windows.imgui_debugger);
            ImGui::MenuItem("Pattern", NULL, (bool *)&visible_windows.pattern);
            ImGui::MenuItem("Controls", NULL, (bool *)&visible_windows.controls);
            ImGui::MenuItem("Orders", NULL, (bool *)&visible_windows.orders);
            ImGui::MenuItem("Instrument Editor", NULL, (bool *)&visible_windows.instr);
            ImGui::MenuItem("Register View", NULL, (bool *)&visible_windows.reg_view);
            ImGui::EndMenu();
        }


        ImGui::EndMenuBar();
    }

    // End the parent window that contains the Dockspace:
    ImGui::End();
}

// save settings
void save_settings() {
    ImGuiIO& io = ImGui::GetIO();
    std::string filename = "mah-tracker.cfg";
    std::ofstream f(filename);
    if (!f) return;
    std::string value;
    f << io.FontGlobalScale << "\n";
    for (int i = 0; i < sizeof(visible_windows); i++) {
        f << (int)(((uint8_t*)&visible_windows)[i]) << "\n";
    }
}

// load settings
void load_settings() {
    ImGuiIO& io = ImGui::GetIO();
    std::string filename = "mah-tracker.cfg";
    std::ifstream f(filename);
    if (!f) return;
    std::string value;
    getline(f,value); if (!value.empty()) io.FontGlobalScale = std::stof(value);
    for (int i = 0; i < sizeof(visible_windows); i++) {
        getline(f,value); if (!value.empty()) ((uint8_t*)&visible_windows)[i] = std::stoi(value);
    }
}

#include "defines.h"

extern void init_sid(); // player.cpp
extern void advance_audio(song *song, cursor *cur_cursor); // player.cpp
extern void init_routine(song *song); // player.cpp
extern void register_view(bool *open);
cursor cur_cursor;
song c_song; // current song

float get_volume() { // for player.cpp
    return visible_windows.audio_volume;
}

// Main code
int main(int argc, char *argv[]) {
    visible_windows.imgui_debugger = false;
    
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI| SDL_WINDOW_MAXIMIZED);
    SDL_Window* window = SDL_CreateWindow("mah tracker baybeee!!11", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("IBMPlexMono-Regular.ttf", 25.0f); // FOINISS FONT OH MAI GAHHHH

    io.FontGlobalScale = 1.0f;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);

    load_settings();
    save_settings(); // for updating config files from older versions

    cur_cursor.ch = 0;
    cur_cursor.row = 0;
    cur_cursor.selection = note;
    cur_cursor.octave = 3;
    cur_cursor.latch = 0;
    cur_cursor.order = 0;
    cur_cursor.instr = 1;
    cur_cursor.playing = 0;
    cur_cursor.play_row = 0;

    // Main loop
    bool done = false;
    int cur_frame = 0;
    for (int pat = 0; pat < 256; pat++) {
        for (int row = 0; row < 32; row++) {
            c_song.pattern[pat].rows[row].note = NOTE_EMPTY;
            c_song.pattern[pat].rows[row].instr = 0;
            c_song.pattern[pat].rows[row].eff_type = 0;
            c_song.pattern[pat].rows[row].eff_arg = 0;
        }
        c_song.order_table[0][pat] = 0;
        c_song.order_table[1][pat] = 0;
        c_song.order_table[2][pat] = 0;
    }

    for (int ins = 0; ins < 128; ins++) {
        c_song.instr[ins].a = 0x0;
        c_song.instr[ins].d = 0x8;
        c_song.instr[ins].s = 0x0;
        c_song.instr[ins].r = 0x0;
        c_song.instr[ins].wav_len = 0x1;
        c_song.instr[ins].wav_loop = INS_NO_LOOP;
        memset(c_song.instr[ins].wav,0,128);
        memset(c_song.instr[ins].arp,48,128);
        c_song.instr[ins].wav[0] = 0x21;

        c_song.instr[ins].filter_len = 0;
        c_song.instr[ins].filter_loop = INS_NO_LOOP;
        memset(c_song.instr[ins].filter,0,128);
        memset(c_song.instr[ins].filter_mode,0,128);
    }

    //                      00 01
    // initial order table: 00 END
    c_song.order_table[0][0] = 0x00;
    c_song.order_table[1][0] = 0x01;
    c_song.order_table[2][0] = 0x02;
    c_song.order_len = 1;

    c_song.init_speed = 6;

    init_sid();
    init_routine(&c_song);
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    // Main loop
    while (!done) {
#endif
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
                case SDL_QUIT:
                    done = true;
                    break;
            }
        }
        advance_audio(&c_song,&cur_cursor);

        cur_frame++;
        if (cur_frame == (50*60)) {
            cur_frame = 0;
            save_settings();
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ShowExampleAppDockSpace((bool*)false);

        if (visible_windows.settings) {
            // Settings
            ImGui::Begin("Settings",(bool *)&visible_windows.settings);
            ImGui::SliderFloat("UI Scaling Factor", &io.FontGlobalScale, 0.5f, 3.0f); rightClickable
            ImGui::SliderFloat("Master Audio Volume", &visible_windows.audio_volume, 0.0f, 3.0f); rightClickable
            ImGui::Separator();
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();
            if (ImGui::MenuItem("Save Settings")) save_settings();
            if (ImGui::MenuItem("Load Settings")) load_settings();
            ImGui::Separator();
            ImGui::End();
        }

        if (visible_windows.pattern) {
            render_pat(&c_song,&cur_cursor,(bool *)&visible_windows.pattern);
        }

        if (visible_windows.controls) {
            ImGui::Begin("Controls", (bool *)&visible_windows.controls);
            //ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.2f);
            uint8_t one = 1;
            if (ImGui::InputInt("Octave",&cur_cursor.octave)) {
                if (cur_cursor.octave < 0) cur_cursor.octave = 0;
                else if (cur_cursor.octave > 7) cur_cursor.octave = 7;
            }
            if (ImGui::InputScalar("Speed",ImGuiDataType_U8,&c_song.init_speed,&one)) {
                if (c_song.init_speed < 1) c_song.init_speed = 1;
                else if (c_song.init_speed > 127) c_song.init_speed = 1277;
            }
            
            ImGui::End();
        }

        if (visible_windows.orders) {
            render_orders(&c_song,&cur_cursor,(bool *)&visible_windows.pattern);
        }

        if (visible_windows.instr) {
            render_instr(&c_song,&cur_cursor,(bool *)&visible_windows.instr);
        }

        if (visible_windows.reg_view) {
            register_view(&visible_windows.reg_view);
        }

        if (visible_windows.imgui_debugger) {
            ImGui::ShowMetricsWindow((bool *)&visible_windows.imgui_debugger);
        }

        ImGui::Render();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
