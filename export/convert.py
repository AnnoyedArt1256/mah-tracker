"""
!!! IMPORTANT !!!
This converter and the accompanying 6502 driver are NOT licensed under the GPL.
They are licensed under the public domain. 
Use this driver and converter whenever and wherever you want, commercial or for fun!
"""

import sys

def convert(filename):
    file = open(filename,"rb")
    out = "; blah\n"

    magic = file.read(8).decode("utf-8")
    if magic != 'MAHTRACK':
        return

    init_speed = file.read(1)[0]
    out += f"init_speed: .byte {init_speed}\n"

    # reserved bytes
    version = file.read(1)[0]
    version |= file.read(1)[0]<<8
    file.read(5)

    file.read(32)
    file.read(32)
    file.read(32)

    file.read(16) # MOAR PADDING MOAR

    # TODO: what to do for patterns $FF?
    order_len = file.read(1)[0]
    pats_used = []
    for ch in range(3):
        order_line = f"order_ch{ch}: .byte "
        for ord in range(order_len):
            pat = file.read(1)[0]
            if pat not in pats_used: pats_used.append(pat)
            order_line += f"{pat}"
            if ord == order_len-1: order_line += ",$ff\n"
            else: order_line += ","
        out += order_line

    max_pat = file.read(1)[0]

    out += f"pat_lo: .lobytes"
    for pat in range(max_pat+1):
        out += f" pattern{pat}"
        if pat == max_pat: out += "\n"
        else: out += ","

    out += f"pat_hi: .hibytes"
    for pat in range(max_pat+1):
        out += f" pattern{pat}"
        if pat == max_pat: out += "\n"
        else: out += ","

    max_ins = 0
    for pat in range(max_pat+1):
        cur_eff_type = 0xff
        cur_eff_arg = 0
        pattern_data = []
        last_write = 0
        cur_ins = 0
        write_buffer = []
        is_dxx = False
        for row in range(64):
            note, instr, eff_type, eff_arg = file.read(4)
            if version == 0 and eff_type == 3: # workaround for tie notes in older modules
                eff_arg = 0

            write_buffer = []

            if is_dxx:
                continue

            is_dxx |= eff_type == 0x0D

            if eff_type > 0 and not is_dxx and (eff_type != cur_eff_type or eff_arg != cur_eff_arg):
                cur_eff_type, cur_eff_arg = eff_type, eff_arg
                write_buffer.extend([0xE0|(eff_type&0xf), eff_arg]) # do effect
            elif eff_type == 0 and eff_type != cur_eff_type:
                cur_eff_type = 0
                write_buffer.append(0xE0) # stop effects

            if instr != 0 and (cur_ins != instr or row == 0):
                cur_ins = instr
                if instr >= max_ins: max_ins = instr
                if instr < 0x40: 
                    write_buffer.append(instr+0x40)
                else:
                    write_buffer.extend([0xFD, instr])

            if note != 0xFF: # if note is NOT empty
                if note == 0xFE: # note OFF
                    write_buffer.append(0xFE)
                else:
                    write_buffer.append(note|0x80)
                
            if last_write >= 63 or len(write_buffer) > 0:
                if last_write > 0: pattern_data.append(last_write)
                pattern_data.extend(write_buffer)
                last_write = 0

            if not is_dxx and row != 63: last_write += 1

        # write the remaining waits
        if last_write > 0: pattern_data.append(last_write)
        last_write = 0

        pattern_data.append(0xFF)

        if pat not in pats_used: pattern_data = [0xFF] # unused pattern

        out += f"pattern{pat}: .byte {str(pattern_data)[1:-1]}\n"

    ins_properties = []
    ins_wave_dict = {}
    for ins in range(max_ins+1):
        ins_prop = []
        a, d, s, r = file.read(4) # get adsr
        ins_prop.append(a<<4|d)
        ins_prop.append(s<<4|r)

        # get wavetable/arp macros
        wav_len = file.read(1)[0]
        wav_loop = file.read(1)[0]

        ins_prop.append(wav_len)

        wav = []
        arp = []
        for _ in range(wav_len): arp.append(file.read(1)[0])
        for _ in range(wav_len): wav.append(file.read(1)[0])
        arp.append(0xFF)
        wav.append(wav_len-1 if wav_loop == 0xFF else wav_loop)
        ins_prop.append(arp)
        ins_prop.append(wav)

        # duty sweep properties
        ins_prop.append((file.read(1)[0])|(file.read(1)[0]<<8))
        ins_prop.append((file.read(1)[0])|(file.read(1)[0]<<8))
        ins_prop.append((file.read(1)[0])|(file.read(1)[0]<<8))

        # duty reset (VERSION >= 2)
        if version >= 2:
            ins_prop.append(file.read(1)[0])        
        else:
            # stub to always reset on older modules
            ins_prop.append(1)

        filter_res_enable = file.read(1)[0]
        ins_prop.append(filter_res_enable)

        filter_len = file.read(1)[0]
        ins_prop.append(filter_len)

        filter_loop = file.read(1)[0]

        cutoff = []
        filt_mode = []
        for _ in range(filter_len): cutoff.append(file.read(1)[0])
        for _ in range(filter_len): filt_mode.append(file.read(1)[0])
        if filter_len > 0:
            filt_mode.append(0xFF)
            cutoff.append(max(filter_len-1,0) if filter_loop == 0xFF else filter_loop)
        ins_prop.append(filt_mode)
        ins_prop.append(cutoff)

        ins_properties.append(ins_prop)

    out += f"ins_wav_lo: .lobytes"
    for pat in range(max_ins+1):
        out += f" ins_wav{pat}"
        if pat == max_ins: out += "\n"
        else: out += ","
    out += f"ins_wav_hi: .hibytes"
    for pat in range(max_ins+1):
        out += f" ins_wav{pat}"
        if pat == max_ins: out += "\n"
        else: out += ","
    out += f"ins_filt_lo: .lobytes"
    for pat in range(max_ins+1):
        out += f" ins_filt{pat}"
        if pat == max_ins: out += "\n"
        else: out += ","
    out += f"ins_filt_hi: .hibytes"
    for pat in range(max_ins+1):
        out += f" ins_filt{pat}"
        if pat == max_ins: out += "\n"
        else: out += ","
        
    out += f"ins_ad: .byte {str([x[0] for x in ins_properties])[1:-1]}\n"
    out += f"ins_sr: .byte {str([x[1] for x in ins_properties])[1:-1]}\n"
    out += f"ins_wave_len: .byte {str([x[2]+1 for x in ins_properties])[1:-1]}\n"
    for ins in range(max_ins+1):
        # deduplicate the wave/arp tables
        wavearp_table = str(ins_properties[ins][3])[1:-1]
        wavearp_table += "\n"+str(ins_properties[ins][4])[1:-1]
        if wavearp_table in ins_wave_dict:
            out += f"ins_wav{ins} = {ins_wave_dict[wavearp_table]}\n"
        else:
            out += f"ins_wav{ins}:\n"
            out += f".byte {str(ins_properties[ins][3])[1:-1]}\n"
            out += f".byte {str(ins_properties[ins][4])[1:-1]}\n"
            ins_wave_dict[wavearp_table] = f"ins_wav{ins}"
    out += f"ins_duty_start_lo: .lobytes {str([x[5] for x in ins_properties])[1:-1]}\n"
    out += f"ins_duty_start_hi: .hibytes {str([x[5] for x in ins_properties])[1:-1]}\n"

    out += f"ins_duty_end_lo: .lobytes {str([x[6] for x in ins_properties])[1:-1]}\n"
    out += f"ins_duty_end_hi: .hibytes {str([x[6] for x in ins_properties])[1:-1]}\n"

    out += f"ins_duty_speed_lo: .lobytes {str([x[7] for x in ins_properties])[1:-1]}\n"
    out += f"ins_duty_speed_hi: .hibytes {str([x[7] for x in ins_properties])[1:-1]}\n"

    out += f"ins_duty_reset: .lobytes {str([x[8] for x in ins_properties])[1:-1]}\n"

    out += f"ins_filter_enable: .byte {str([x[9] for x in ins_properties])[1:-1]}\n\n"
    out += f"ins_filter_len: .byte {str([0 if x[10] == 0 else x[10]+1 for x in ins_properties])[1:-1]}\n"
    for ins in range(max_ins+1):
        out += f"ins_filt{ins}:\n"
        if ins_properties[ins][10] > 0:
            out += f".byte {str(ins_properties[ins][11])[1:-1]}\n"
            out += f".byte {str(ins_properties[ins][12])[1:-1]}\n"

    file.close()
    file = open("music.asm","w")
    file.write(out)
    file.close()

if len(sys.argv) >= 2:
    convert(sys.argv[1])

    # frequency calculation code taken from
    # https://codebase64.org/doku.php?id=base:how_to_calculate_your_own_sid_frequency_table

    tuning = 440
    f = open("note_lo.bin","wb")
    for i in range(96):
        hz = tuning * (2**(float(i-57)/12.0))
        cnst = (256**3)/985248.0 # PAL frequency
        freq = min(max(hz*cnst,0),0xffff)
        f.write(bytearray([int(freq)&0xff]))
    f.close()
    f = open("note_hi.bin","wb")
    for i in range(96):
        hz = tuning * (2**(float(i-57)/12.0))
        cnst = (256**3)/985248.0 # PAL frequency
        freq = min(max(hz*cnst,0),0xffff)
        f.write(bytearray([(int(freq)>>8)&0xff]))
    f.close()
else:
    print("not enough arguments!\n")
    print("run the python converter like this")
    print(f"    python3 {sys.argv[0]} example_module.mah\n")
    print("OR run the provided shell file like this")
    print(f"    sh compile.sh example_module.mah")
    sys.exit(1)