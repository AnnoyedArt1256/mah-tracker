# mah-tracker
AArt1256's custom SID chiptune tracker meant to be easy to use for newcomers and experts alike!

### effects (in the editor, just in case)
- 1xx: pitch up
- 2xx: pitch down
- 3.. (or 300): tie note
- 3xx (where xx is a **non-zero** value): glissando/pitch glide
- 4xy: vibrato (x: speed, y: depth)
- 5xy: set attack and decay to XY
- 6xy: set sustain and release to XY
- 9xx: set filter cutoff to XX
- Cxx: set transpose for channel to XX
- Dxx: jump to next pattern
- Exy: set funk tempo (aka two alternating speeds) to be x and y for each speed value
- Fxx: set speed to xx

### shortcuts
### pattern editor:
- up/down/left/right: move the cursor in that direction
- space: switch between read-only ("Jam") and read-write ("Record") modes. this can also be done through the Jam/Record button in the Controls window.
- enter: play the song beginning from the current pattern
- **1**: put a note cut/off in the selected channel's row (IF the cursor is over a note)
<br><br>
- ctrl+c: **copy** note/selection
- ctrl+x: **cut** note/selection
- ctrl+v: **paste** note/selection
- ctrl+z: **undo** note/selection
- ctrl+y: **redo** note/selection
<br><br>
- backspace/delete: delete current note/selection
<br><br>
- ctrl/cmd+f1: transpose note/selection by +1 semitone
- ctrl/cmd+f2: transpose note/selection by -1 semitone
- ctrl/cmd+f3: transpose note/selection by +1 octave (+12 semitones)
- ctrl/cmd+f4: transpose note/selection by -1 octave (-12 semitones)

## legal
The **tracker GUI** in C++ is distributed under the GPLv2 license as shown in [LICENSE](LICENSE)

**HOWEVER** the [SID export player](export/driver.asm)  and its [accompanying converter](export/convert.py) is **NOT** included within the GPL license, it's in the public domain so use it whenever and wherever you want to!