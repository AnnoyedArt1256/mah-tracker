# mah-tracker module format

## INFO
unless noted otherwise, each byte/word is treated as an **unsigned** value and is **little endian**...

## Header (128 bytes)
- 8 byte magic string: `MAHTRACK` (\0 is null terminator or $00 in hex)
- 1 byte: initial tick speed
- 7 bytes reserved (currently only zeros)
- 32 bytes reserved for title (in the future)
- 32 bytes reserved for author (in the future)
- 32 bytes reserved for copyright/year (in the future)
- 16 bytes reserved (currently only zeros)

## Orders
- 1 bytes: global order table length

- N*3 bytes: order table (NON-interleaved)
    - in the file, the first channel's order table is laid out first, then the second's and then the third's

## Patterns
- 1 byte: amount of patterns
    - NOTE: the same pattern can be shared across multiple channels, so eg. if channel 1 and channel 2 use pattern $02 then they will play the same pattern and notes
- for each pattern:
  - there are 64 rows and for each row
    - there are 4 bytes
        - 1st byte: note (0 = C-0, ..., 95 = B-7, OFF = 254, EMPTY = 255)
        - 2nd byte: instrument 
            - if non-zero, then trigger an instrument or else it is considered an empty instrument row
        - 3rd byte: effect type (only the least signifcant 4 bits are used for this)  
            - if non-zero, then trigger an effect for that specific row or else it is considered an empty effect row and no effects are run in the driver
        - 4th byte: effect argument:
            - the usage of this byte changes based off the effect type (3rd byte)


## Instruments
- there are 128 instruments in a module (no more, no less)
- for each instrument the byte structure is as follows
    - 1 byte: attack
    - 1 byte: decay
    - 1 byte: sustain
    - 1 byte: release

    - 1 byte: wavetable/arp table length (0 = there's no arp/wav table)
    - 1 byte: wavetable/arp table loop position
        - if the loop position is 0xFF (255), then the wavetable/arp table DOES NOT LOOP!
    - N bytes: **arpeggio** part of the wave/arp table
        - for each byte:
            - if the MSB (bit 7, byte&0x80) is set
                - then it is an **absolute arpeggio** and the other least significant bits are the note value directly
            - if the MSB (bit 7, byte&0x80) is NOT set
                - then it is a **relative arpeggio** and the other least significant bits are the relative arpeggio value where the minimum value (-48) is represented with a **0** and the maximum value (47) is represented with **95** 

    - N bytes: **wave** part of the wave/arp table
        - each byte is just a raw SID waveform value, no conversion is needed

    - 2 bytes: duty sweep start
    - 2 bytes: duty sweep end
    - 2 bytes: duty sweep speed (signed 16-bit!!)
