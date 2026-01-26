# mah-tracker
AArt1256's custom SID chiptune tracker meant to be easy to use for newcomers and experts alike!

### effects (in the editor, just in case)
- 1xx: pitch up
- 2xx: pitch down
- 3..: tie note (TBA: glissando)
- 5xy: set attack and decay to XY
- 6xy: set sustain and release to XY
- Cxx: set transpose for channel to XX
- Dxx: jump to next pattern
- Fxx: set speed to xx

## legal
The **tracker GUI** in C++ is distributed under the GPLv2 license as shown in [LICENSE](LICENSE)

**HOWEVER** the [SID export player](export/driver.asm)  and its [accompanying converter](export/convert.py) is **NOT** included within the GPL license, it's in the public domain so use it whenever and wherever you want to!