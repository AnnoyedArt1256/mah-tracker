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

// Default layout for imgui.ini
const char *default_layout="[Window][DockSpace Demo]\n\
Pos=0,0\n\
Size=1470,820\n\
Collapsed=0\n\
\n\
[Window][Debug##Default]\n\
Pos=60,60\n\
Size=400,400\n\
Collapsed=0\n\
\n\
[Window][Dear ImGui Metrics/Debugger]\n\
Pos=1038,323\n\
Size=432,498\n\
Collapsed=0\n\
DockId=0x00000002,0\n\
\n\
[Window][Settings]\n\
Pos=974,26\n\
Size=496,295\n\
Collapsed=0\n\
DockId=0x0000000C,0\n\
\n\
[Window][Pattern]\n\
Pos=0,371\n\
Size=857,449\n\
Collapsed=0\n\
DockId=0x0000000E,0\n\
\n\
[Window][Controls]\n\
Pos=593,26\n\
Size=379,295\n\
Collapsed=0\n\
DockId=0x0000000B,0\n\
\n\
[Window][Orders]\n\
Pos=0,26\n\
Size=591,295\n\
Collapsed=0\n\
DockId=0x00000005,0\n\
\n\
[Window][Instrument Editor]\n\
Pos=859,323\n\
Size=611,497\n\
Collapsed=0\n\
DockId=0x00000008,0\n\
\n\
[Window][Register View]\n\
Pos=593,26\n\
Size=379,295\n\
Collapsed=0\n\
DockId=0x0000000B,1\n\
\n\
[Window][Filter Info]\n\
Pos=0,323\n\
Size=857,44\n\
Collapsed=0\n\
DockId=0x00000009,0\n\
\n\
[Window][Filter & Channel Info]\n\
Pos=0,323\n\
Size=857,46\n\
Collapsed=0\n\
DockId=0x0000000D,0\n\
\n\
[Docking][Data]\n\
DockSpace           ID=0xC0DFADC4 Window=0xD0388BC8 Pos=0,26 Size=1470,794 Split=Y\n\
  DockNode          ID=0x00000003 Parent=0xC0DFADC4 SizeRef=1470,295 Split=X Selected=0xA0C159B7\n\
    DockNode        ID=0x00000005 Parent=0x00000003 SizeRef=591,265 HiddenTabBar=1 Selected=0x575D137D\n\
    DockNode        ID=0x00000006 Parent=0x00000003 SizeRef=877,265 Split=X Selected=0x4746B4B8\n\
      DockNode      ID=0x0000000B Parent=0x00000006 SizeRef=379,265 Selected=0xA0C159B7\n\
      DockNode      ID=0x0000000C Parent=0x00000006 SizeRef=496,265 Selected=0x4746B4B8\n\
  DockNode          ID=0x00000004 Parent=0xC0DFADC4 SizeRef=1470,494 Split=X Selected=0xFD260491\n\
    DockNode        ID=0x00000001 Parent=0x00000004 SizeRef=1036,538 Split=X Selected=0xFD260491\n\
      DockNode      ID=0x00000007 Parent=0x00000001 SizeRef=857,536 Split=Y Selected=0xFD260491\n\
        DockNode    ID=0x00000009 Parent=0x00000007 SizeRef=850,44 HiddenTabBar=1 Selected=0x86B4F1EE\n\
        DockNode    ID=0x0000000A Parent=0x00000007 SizeRef=850,448 Split=Y Selected=0xFD260491\n\
          DockNode  ID=0x0000000D Parent=0x0000000A SizeRef=857,46 HiddenTabBar=1 Selected=0x48025F75\n\
          DockNode  ID=0x0000000E Parent=0x0000000A SizeRef=857,454 CentralNode=1 Selected=0xFD260491\n\
      DockNode      ID=0x00000008 Parent=0x00000001 SizeRef=611,536 Selected=0xE12C31DF\n\
    DockNode        ID=0x00000002 Parent=0x00000004 SizeRef=432,538 Selected=0xD9E076F4\n\
\n\n";

void reset_layout() {
    ImGui::LoadIniSettingsFromMemory(default_layout);   
}