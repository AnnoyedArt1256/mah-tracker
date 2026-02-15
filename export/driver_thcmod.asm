; !!! IMPORTANT !!!
; This 6502 driver and the accompanying Python converter are NOT licensed under the GPL.
; They are licensed under the public domain. 
; Use this driver and converter whenever and wherever you want, commercial or for fun!

; HR_MODE = 0: use the hard-restart method that the GUI editor uses
; HR_MODE = 1: use the hard-restart method that GT2 and other players use
HR_MODE = 0
HR_FRAME_LENGTH = 1

.zeropage
.org $fe
temp: .res 2

.segment "CODE"

.org    $080D
jmp $900
.res $0900-*
main:
 sei
 lda #$35
 sta $01

 lda #127
 sta $dc0d

 and $d011
 sta $d011

 lda $dc0d
 lda $dd0d

 lda #<irq
 sta $fffe
 lda #>irq
 sta $ffff

 lda #$1b
 sta $d011
 lda #$40
 sta $d012

 lda #1
 sta $d01a

 lda #0
 jsr $0d00

    cli
	jmp *

irq:
	pha
    txa
    pha
    tya
    pha

    inc $d020
	jsr $0d03
    dec $d020
    asl $d019

    pla
    tay
    pla
    tax
    pla
    rti

.res $0d00-*
    jmp init
play:
    dec tick
    lda tick
    beq :+
    jmp skipseq
:

    lda #0
    sta do_patend

    ldx #2
ch_loop:
    jsr do_ch
    dex
    bpl ch_loop

    lda do_patend
    beq :+
    inc order
    lda order
    jsr set_pat
:

    ldx tick_sel
    lda speed, x
    sta tick
    lda tick_sel
    eor #1
    sta tick_sel
skipseq:

    ldx #2
:
    jsr do_wav
    jsr do_pulse_sweep
    jsr do_eff
    .if HR_MODE = 1
        lda hr_delay, x
        cmp #$ff
        bne @skip_freq
    .endif
    cpx #2
    beq @skip_ch
    ldy sid_mul, x
    lda final_freq
    clc
    adc bend_lo, x
    sta $d400, y
    lda final_freq+1
    adc bend_hi, x
    sta $d401, y
    lda duty_lo, x
    sta $d402, y
    lda duty_hi, x
    sta $d403, y
    .if HR_MODE = 1
@skip_freq:
    .endif
@skip_ch:
    dex
    bpl :-

    ;jsr do_filt
    ;rts

do_filt:
    ldx filt_inst
    beq :+
    lda ins_filt_lo, x
    sta temp
    lda ins_filt_hi, x
    sta temp+1

@redo_wave:
    ldy filter_pos
    lda (temp), y ; filter mode
    cmp #$ff
    beq @do_jump

    ora #$0f
    sta $d418

    tya
    clc
    adc ins_filter_len, x
    tay
    lda (temp), y ; filter cutoff
    sta $d416
   

    inc filter_pos
:
    rts

@do_jump:
    ldy filt_inst
    lda filter_pos
    clc
    adc ins_filter_len, y
    tay
    lda (temp), y
    sta filter_pos
    jmp @redo_wave

do_pulse_sweep:
    ldy ins, x
    lda duty_lo, x
    clc
    adc duty_speed_lo, x
    sta duty_lo, x
    lda duty_hi, x
    adc duty_speed_hi, x
    sta duty_hi, x

    lda ins_duty_end_lo, y
    sec
    sbc duty_lo, x
    lda ins_duty_end_hi, y
    sbc duty_hi, x
    and #$80
    beq :+
    lda ins_duty_end_lo, y
    sta duty_lo, x
    lda ins_duty_end_hi, y
    sta duty_hi, x
    jsr flip_duty_speed
    rts
:

    lda duty_lo, x
    sec
    sbc ins_duty_start_lo, y
    lda duty_hi, x
    sbc ins_duty_start_hi, y
    and #$80
    beq :+
    lda ins_duty_start_lo, y
    sta duty_lo, x
    lda ins_duty_start_hi, y
    sta duty_hi, x
    jsr flip_duty_speed
:
    rts

flip_duty_speed:
    ; (duty_speed^0xffff)+1
    lda duty_speed_lo, x
    eor #$ff
    sta duty_speed_lo, x
    lda duty_speed_hi, x
    eor #$ff
    sta duty_speed_hi, x
    inc duty_speed_lo, x
    bne :+
    inc duty_speed_hi, x
:
    rts

set_adsr_note:
    cpx #2
    beq @skip
    ldy ins, x
    lda ins_ad, y
    sta temp
    lda ins_sr, y
    sta temp+1

    lda eff_type, x
    cmp #5
    bne :+
    lda eff_arg, x
    sta temp
:
    cmp #6
    bne :+
    lda eff_arg, x
    sta temp+1
:

    ldy sid_mul, x
    lda temp
    sta $d405, y
    lda temp+1
    sta $d406, y
@skip:
    rts

init_note_macros:
    jsr set_adsr_note

    ldy ins, x
    lda ins_duty_start_lo, y 
    sta duty_lo, x
    lda ins_duty_start_hi, y 
    sta duty_hi, x
    lda ins_duty_speed_lo, y 
    sta duty_speed_lo, x
    lda ins_duty_speed_hi, y 
    sta duty_speed_hi, x

    lda filt_resonance_temp
    and bit_mask_inv, x
    sta filt_resonance_temp
    and #7
    sta temp

    lda ins_filter_enable, y
    asl
    asl
    asl
    asl
    bcc :+
    ora bit_mask, x
    ora temp
    sta filt_resonance_temp
:
    lda filt_resonance_temp
    sta $d417

    lda ins_filter_len, y
    beq :+
    lda #0
    sta filter_pos
    sty filt_inst
:

    lda #$ff
    sta hr_delay, x
    rts

do_wav:
    lda ins, x
    bne :+
    rts
:
    lda hr_delay, x
    cmp #$ff
    beq @skip_to_wav
    inc hr_delay, x
    lda hr_delay, x
    .if HR_MODE = 1
        cmp #HR_FRAME_LENGTH
        bne :+
        jsr set_adsr_note
        lda #$09
        sta $d404, y
        rts
    :
        cmp #HR_FRAME_LENGTH+1 ; HR frames
    .else
        cmp #HR_FRAME_LENGTH+1
    .endif
    bne @skip_note_macro_init
    jsr init_note_macros
    jmp @skip_to_wav
@skip_note_macro_init:
    rts
@skip_to_wav:

    ldy ins, x
    lda ins_wav_lo, y
    sta temp
    lda ins_wav_hi, y
    sta temp+1
    
@redo_wave:
    ldy arp_pos, x
    lda (temp), y
    bpl :+
    cmp #$ff
    beq @do_jump
    and #$7f
    tay
    lda freq_lo, y
    sta final_freq
    lda freq_hi, y
    sta final_freq+1
    jmp :++
:
    clc
    adc transpose, x
    clc
    adc cur_note, x
    sec
    sbc #48
    tay
    lda freq_lo, y
    sta final_freq
    lda freq_hi, y
    sta final_freq+1
:

@cont_wave:

    ldy ins, x
    lda arp_pos, x
    clc
    adc ins_wave_len, y
    tay
    lda (temp), y
    and gate_mask, x
    ldy sid_mul, x
    cpx #2
    beq :+
    sta $d404, y
    .if HR_MODE = 1
        sta wave_temp, x
    .endif
:

    inc arp_pos, x
    rts

@do_jump:
    ldy ins, x
    lda arp_pos, x
    clc
    adc ins_wave_len, y
    tay
    lda (temp), y
    sta arp_pos, x
    jmp @redo_wave

do_ch:
    dec dur, x
    lda dur, x
    beq :+
    rts
:

    lda pat_ptr_lo, x
    sta temp
    lda pat_ptr_hi, x
    sta temp+1

@parse_rept:

    ldy #0
    lda (temp), y
    cmp #$40
    bcs :+
    sta dur, x
    jsr inc_pat
    jmp @end_parse
:
    cmp #$80
    bcs :+
    and #$3f
    sta ins, x
    jsr inc_pat
    lda eff_type, x
    cmp #3
    beq @parse_rept
    jsr reinit_note_inst
    jmp @parse_rept
:
    cmp #$e0
    bcs :+
    and #$7f
    sta cur_note, x
    jsr inc_pat
    lda eff_type, x
    cmp #3
    beq @parse_rept
    jsr reinit_note_inst
    jmp @parse_rept
:
    cmp #$e0
    bne :+
    lda #0
    sta eff_type, x
    sta eff_arg, x
    jsr inc_pat
    jmp @parse_rept
:
    cmp #$f0
    bcs :+
    and #$0f
    ldy eff_type, x
    sta eff_type, x
    sty last_eff
    ldy #1
    lda (temp), y
    sta eff_arg, x
    jsr inc_pat
    jsr inc_pat
    jmp @parse_rept
:
    cmp #$fe
    bne :+
    sta gate_mask, x
    jsr inc_pat
    jmp @parse_rept
:
    cmp #$ff
    bne :+
    lda #1
    sta do_patend
:
@end_parse:
    lda last_eff
    cmp eff_type, x
    beq :+
    lda #0
    sta vib_tim, x
:

    lda eff_type, x
    cmp #$0e
    bne :+
    lda eff_arg, x
    and #$0f
    sta speed+1
    lda eff_arg, x
    lsr
    lsr
    lsr
    lsr
    sta speed
:

    lda eff_type, x
    cmp #$0f
    bne :+
    lda eff_arg, x
    sta speed
    sta speed+1
:

    lda eff_type, x
    cmp #$0c
    bne :+
    lda eff_arg, x
    sta transpose, x
:

    lda temp
    sta pat_ptr_lo, x
    lda temp+1
    sta pat_ptr_hi, x
ch_skip_parse:
    rts

inc_pat:
    inc temp
    bne :+
    inc temp+1
:
    rts

reinit_note_inst:
    lda #0
    sta hr_delay, x
    sta arp_pos, x
    sta vib_tim, x
    sta bend_lo, x
    sta bend_hi, x
    lda #$ff
    sta gate_mask, x
    cpx #2
    beq :+
    .if HR_MODE = 1
        ldy sid_mul, x
        lda #$00
        sta $d405, y
        lda #$00
        sta $d406, y
        lda wave_temp, x
        and #$fe
        sta $d404, y
    .else
        lda #0
        ldy sid_mul, x
        sta $d405, y
        sta $d406, y
        lda #$08
        sta $d404, y
    .endif
:
    rts

sid_mul: .byte 0, 7, 14
bit_mask: .byte 1, 2, 4
bit_mask_inv: .byte 1^$ff, 2^$ff, 4^$ff

init:
    ldx #$18
    lda #0
:
    sta $d400, x
    dex
    bpl :-
    ldx #(vars_end-vars_start)-1
:
    sta vars_start, x
    dex
    bpl :-
    lda #$0f
    sta $d418
    lda init_speed
    sta speed
    sta speed+1
    lda #1
    sta tick
    lda #0
    sta order
    sta ins
    sta ins+1
    sta ins+2
    ;jsr set_pat
    ;rts

; Y = pattern
set_pat:
    sta @set_pat_smc+1
    ldx #2
@loop:
    lda order_lo, x
    sta temp
    lda order_hi, x
    sta temp+1
@set_pat_smc:
    ldy #0
    lda (temp), y
    cmp #$ff
    bne :+
    lda #0
    sta order
    jmp set_pat
:
    tay
    lda pat_lo, y
    sta pat_ptr_lo, x
    lda pat_hi, y
    sta pat_ptr_hi, x
    dex
    bpl @loop

    lda #1
    sta dur
    sta dur+1
    sta dur+2
    rts

do_eff:
    lda eff_type, x
    cmp #4
    beq :+
    cmp #1
    bne @skip1
    lda eff_arg, x
    sta temp
    jmp vib_add
@skip1:
    cmp #2
    bne @skip2
    lda eff_arg, x
    sta temp
    jmp vib_sub
@skip2:
    rts
:

    lda eff_arg, x
    asl
    asl
    asl
    asl
    sta temp
    lda eff_arg, x
    lsr
    lsr
    lsr
    lsr
    eor #$0f
    sta temp+1
    lda vib_tim, x
    bmi :+
    cmp temp+1
    bcc :+
    eor #$ff
:
    clc
    adc #2
    sta vib_tim, x
    and #1
    beq vib_add
    ;bne vib_sub
vib_sub:
    lda bend_lo, x
    sec
    sbc temp
    sta bend_lo, x
    lda bend_hi, x
    sbc #0
    sta bend_hi, x
    rts

vib_add:
    lda bend_lo, x
    clc
    adc temp
    sta bend_lo, x
    lda bend_hi, x
    adc #0
    sta bend_hi, x
    rts

order_lo:
    .lobytes order_ch0, order_ch1, order_ch2

order_hi:
    .hibytes order_ch0, order_ch1, order_ch2

vars_start:
speed: .byte 0, 0
tick: .byte 0
tick_sel: .byte 0
dur: .res 3, 0
ins: .res 3, 0
gate_mask: .res 3,0
pat_ptr_lo: .res 3, 0
pat_ptr_hi: .res 3, 0
eff_type: .res 3, 0
eff_arg: .res 3, 0
cur_note: .res 3, 0
arp_pos: .res 3, 0
arp_addr_lo: .res 3, 0
arp_addr_hi: .res 3, 0
hr_delay: .res 3, 0
do_patend: .byte 0
order: .byte 0
final_freq: .word 0
duty_lo: .res 3, 0
duty_hi: .res 3, 0
duty_speed_lo: .res 3, 0
duty_speed_hi: .res 3, 0
filt_resonance_temp: .byte 0
filt_inst: .byte 0
filter_pos: .byte 0
vib_tim: .res 3, 0
bend_lo: .res 3, 0
bend_hi: .res 3, 0
last_eff: .byte 0
transpose: .res 3, 0
.if HR_MODE = 1
    wave_temp: .res 3, 0
.endif
vars_end:

freq_lo:
    .incbin "note_lo.bin"

freq_hi:
    .incbin "note_hi.bin"

.include "music.asm"