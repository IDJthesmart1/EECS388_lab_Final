# ATmega328P Timer Control Reference

**Scope:** Timer0 modes 2 and 7, Timer1 modes 12 and 14, Timer2 modes 2 and 7.
**Source:** Atmel ATmega328P datasheet (7810D–AVR–01/15), chapters 14 (Timer0), 15 (Timer1), 16 (prescalers) and 17 (Timer2).
**Naming convention:** registers are written as `SURNAME` (e.g. TCCR1A). Fields are written as `FORENAME (SURNAME[Z:A])`, where `SURNAME` is the register that holds the field and `[Z:A]` are its bits, MSB `Z` down to LSB `A` (e.g. COM1A (TCCR1A[7:6]); a single bit is `[N]`, e.g. WGM11 (TCCR1A[1])). Mode codes that span two registers are written `WGMn[Z:A]`, e.g. WGM1[3:0] = 1100. Full register tables are in Section 2.

---

## 1. The modes at a glance

| Timer | Mode | WGM bits | Name | TOP | OCRx update | TOV flag set at | Pin-control style |
|---|---|---|---|---|---|---|---|
| Timer0 | 2 | WGM0[2:0] = `010` | CTC | OCR0A | Immediate | MAX (0xFF) | Set / clear / toggle on match |
| Timer0 | 7 | WGM0[2:0] = `111` | Fast PWM | OCR0A | BOTTOM (double buffered) | TOP | PWM on OC0B; OC0A toggle only |
| Timer1 | 12 | WGM1[3:0] = `1100` | CTC | ICR1 | Immediate | MAX (0xFFFF) | Set / clear / toggle on match, both channels |
| Timer1 | 14 | WGM1[3:0] = `1110` | Fast PWM | ICR1 | BOTTOM (double buffered) | TOP | PWM on both OC1A and OC1B |
| Timer2 | 2 | WGM2[2:0] = `010` | CTC | OCR2A | Immediate | MAX (0xFF) | Set / clear / toggle on match |
| Timer2 | 7 | WGM2[2:0] = `111` | Fast PWM | OCR2A | BOTTOM (double buffered) | TOP | PWM on OC2B; OC2A toggle only |

### Output pins

| Output | Port pin | DDR bit that must be set to 1 | Arduino Uno pin |
|---|---|---|---|
| OC0A | PD6 | DDD6 (DDRD[6]) | D6 |
| OC0B | PD5 | DDD5 (DDRD[5]) | D5 |
| OC1A | PB1 | DDB1 (DDRB[1]) | D9 |
| OC1B | PB2 | DDB2 (DDRB[2]) (shared with SS) | D10 |
| OC2A | PB3 | DDB3 (DDRB[3]) (shared with MOSI) | D11 |
| OC2B | PD3 | DDD3 (DDRD[3]) | D3 |

---

## 2. Register map with bit indices

**Reading these tables:** each row names the register that holds the bits. The Field column gives `FORENAME (SURNAME[Z:A])`, with the MSB `Z` first and the LSB `A` last; the Bits column repeats the same range. R/W: `R/W` read-write, `R` read-only, `W` write-only strobe. Flag bits clear by writing a 1.

### 2.1 Timer0 registers (8-bit)

All bits reset to 0.

| Register (address) | Bits | Field | R/W | Role |
|---|---|---|---|---|
| TCCR0A 0x24 (0x44) | [7:6] | COM0A (TCCR0A[7:6]) | R/W | Compare output mode for OC0A. Any nonzero value connects OC0A to its pin (DDD6 (DDRD[6]) must be 1). COM0A1 (TCCR0A[7]) is the high bit, COM0A0 (TCCR0A[6]) the low bit. `00` disconnected (normal port operation) · `01` mode 2: toggle on match; mode 7: toggle on match with OCR0A (= TOP) · `10` mode 2: clear on match; mode 7: non-inverting PWM (on OC0A/OC2A this gives a constant level because OCRnA is TOP, see 5.2 and 5.6) · `11` mode 2: set on match; mode 7: inverting PWM. |
| TCCR0A 0x24 (0x44) | [5:4] | COM0B (TCCR0A[5:4]) | R/W | Compare output mode for OC0B. Any nonzero value connects OC0B to its pin (DDD5 (DDRD[5]) must be 1). COM0B1 (TCCR0A[5]) is the high bit, COM0B0 (TCCR0A[4]) the low bit. `00` disconnected · `01` mode 2: toggle on match; mode 7: **reserved** · `10` mode 2: clear on match; mode 7: non-inverting PWM · `11` mode 2: set on match; mode 7: inverting PWM. |
| TCCR0A 0x24 (0x44) | [3:2] | Reserved (TCCR0A[3:2]) | R | Reads 0, write 0. |
| TCCR0A 0x24 (0x44) | [1] | WGM01 (TCCR0A[1]) | R/W | Middle bit of the 3-bit mode code WGM0[2:0] (the top bit WGM02 is TCCR0B[3]). Mode 2 needs `1`, mode 7 needs `1`. |
| TCCR0A 0x24 (0x44) | [0] | WGM00 (TCCR0A[0]) | R/W | Low bit of the 3-bit mode code WGM0[2:0]. Mode 2 needs `0`, mode 7 needs `1`. |
| TCCR0B 0x25 (0x45) | [7] | FOC0A (TCCR0B[7]) | W | Force Output Compare A: writing `1` forces an immediate compare match on OC0A using the current COM0A (TCCR0A[7:6]) action. Non-PWM only (mode 2 yes; **mode 7: write 0**). No flag, no interrupt, no timer clear. Reads 0. |
| TCCR0B 0x25 (0x45) | [6] | FOC0B (TCCR0B[6]) | W | Force Output Compare B: same as FOC0A (TCCR0B[7]) for OC0B / COM0B (TCCR0A[5:4]). |
| TCCR0B 0x25 (0x45) | [5:4] | Reserved (TCCR0B[5:4]) | R | Reads 0, write 0. |
| TCCR0B 0x25 (0x45) | [3] | WGM02 (TCCR0B[3]) | R/W | Top bit of the 3-bit mode code WGM0[2:0] (the low two bits are WGM01 (TCCR0A[1]) and WGM00 (TCCR0A[0])). Mode 2 needs `0`; **mode 7 needs `1`**. |
| TCCR0B 0x25 (0x45) | [2:0] | CS0 (TCCR0B[2:0]) | R/W | Clock select, 3-bit code (CS02 (TCCR0B[2]), weight 4; CS01 (TCCR0B[1]), weight 2; CS00 (TCCR0B[0]), weight 1). `000` stops the timer; decode in Section 3. |
| TCNT0 0x26 (0x46) | [7:0] | TCNT0 (TCNT0[7:0]) | R/W | Counter value. Writing blocks the next compare match. |
| OCR0A 0x27 (0x47) | [7:0] | OCR0A (OCR0A[7:0]) | R/W | Compare A; also TOP in modes 2 and 7. Immediate update in mode 2, double-buffered (latched at BOTTOM) in mode 7. |
| OCR0B 0x28 (0x48) | [7:0] | OCR0B (OCR0B[7:0]) | R/W | Compare B (duty cycle of OC0B in mode 7). Immediate in mode 2, double-buffered in mode 7. |
| TIMSK0 (0x6E) | [7:3] | Reserved (TIMSK0[7:3]) | R | Reads 0, write 0. |
| TIMSK0 (0x6E) | [2] | OCIE0B (TIMSK0[2]) | R/W | Compare B interrupt enable (TIMER0_COMPB; also needs the I-bit, SREG[7]). |
| TIMSK0 (0x6E) | [1] | OCIE0A (TIMSK0[1]) | R/W | Compare A interrupt enable (TIMER0_COMPA). In modes 2 and 7 this fires at TOP. |
| TIMSK0 (0x6E) | [0] | TOIE0 (TIMSK0[0]) | R/W | Overflow interrupt enable (TIMER0_OVF). Mode 2: fires on wrap from 0xFF; mode 7: fires at TOP. |
| TIFR0 0x15 (0x35) | [7:3] | Reserved (TIFR0[7:3]) | R | Reads 0, write 0. |
| TIFR0 0x15 (0x35) | [2] | OCF0B (TIFR0[2]) | R/W | Compare B match flag. Cleared by running the ISR or by writing `1`. |
| TIFR0 0x15 (0x35) | [1] | OCF0A (TIFR0[1]) | R/W | Compare A match flag (set at TOP in modes 2 and 7). Clear by writing `1`. |
| TIFR0 0x15 (0x35) | [0] | TOV0 (TIFR0[0]) | R/W | Overflow flag. Mode 2: set on wrap from 0xFF; mode 7: set at TOP. Clear by writing `1`. |

### 2.2 Timer1 registers (16-bit counter, 8-bit registers)

All bits reset to 0. The 16-bit values are split into H and L registers; write the high byte first, read the low byte first.

| Register (address) | Bits | Field | R/W | Role |
|---|---|---|---|---|
| TCCR1A (0x80) | [7:6] | COM1A (TCCR1A[7:6]) | R/W | Compare output mode for OC1A. Any nonzero value connects OC1A to its pin (DDB1 (DDRB[1]) must be 1). COM1A1 (TCCR1A[7]) is the high bit, COM1A0 (TCCR1A[6]) the low bit. `00` disconnected (normal port operation) · `01` mode 12: toggle on match; mode 14: toggle on match (50 % wave, see 5.4) · `10` mode 12: clear on match; mode 14: non-inverting PWM · `11` mode 12: set on match; mode 14: inverting PWM. |
| TCCR1A (0x80) | [5:4] | COM1B (TCCR1A[5:4]) | R/W | Compare output mode for OC1B. Any nonzero value connects OC1B to its pin (DDB2 (DDRB[2]) must be 1). COM1B1 (TCCR1A[5]) is the high bit, COM1B0 (TCCR1A[4]) the low bit. `00` disconnected · `01` mode 12: toggle on match; mode 14: disconnected (normal port operation) · `10` mode 12: clear on match; mode 14: non-inverting PWM · `11` mode 12: set on match; mode 14: inverting PWM. |
| TCCR1A (0x80) | [3:2] | Reserved (TCCR1A[3:2]) | R | Reads 0, write 0. |
| TCCR1A (0x80) | [1] | WGM11 (TCCR1A[1]) | R/W | Bit 1 of the 4-bit mode code WGM1[3:0] (WGM13 and WGM12 are TCCR1B[4] and TCCR1B[3]). Mode 12 needs `0`, mode 14 needs `1`. |
| TCCR1A (0x80) | [0] | WGM10 (TCCR1A[0]) | R/W | Bit 0 (LSB) of the 4-bit mode code WGM1[3:0]. Modes 12 and 14 both need `0`. |
| TCCR1B (0x81) | [7] | ICNC1 (TCCR1B[7]) | R/W | Input capture noise canceler. Irrelevant in modes 12 and 14 (ICP1 is disconnected when ICR1 is TOP). |
| TCCR1B (0x81) | [6] | ICES1 (TCCR1B[6]) | R/W | Input capture edge select. Irrelevant in modes 12 and 14. |
| TCCR1B (0x81) | [5] | Reserved (TCCR1B[5]) | R | Reads 0, write 0. |
| TCCR1B (0x81) | [4] | WGM13 (TCCR1B[4]) | R/W | Top bit (MSB) of the 4-bit mode code WGM1[3:0]. **Modes 12 and 14 both need `1`.** |
| TCCR1B (0x81) | [3] | WGM12 (TCCR1B[3]) | R/W | Bit 2 of the 4-bit mode code WGM1[3:0]. **Modes 12 and 14 both need `1`.** |
| TCCR1B (0x81) | [2:0] | CS1 (TCCR1B[2:0]) | R/W | Clock select, 3-bit code (CS12 (TCCR1B[2]), weight 4; CS11 (TCCR1B[1]), weight 2; CS10 (TCCR1B[0]), weight 1). `000` stops the timer; decode in Section 3. |
| TCCR1C (0x82) | [7] | FOC1A (TCCR1C[7]) | R/W | Force Output Compare A: `1` forces an immediate compare match on OC1A using COM1A (TCCR1A[7:6]). Non-PWM only (mode 12 yes; **mode 14: write 0**). No flag, no interrupt, no timer clear. Reads 0. |
| TCCR1C (0x82) | [6] | FOC1B (TCCR1C[6]) | R/W | Force Output Compare B: same as FOC1A (TCCR1C[7]) for OC1B / COM1B (TCCR1A[5:4]). |
| TCCR1C (0x82) | [5:0] | Reserved (TCCR1C[5:0]) | R | Reads 0, write 0. |
| TCNT1H (0x85) | [7:0] | TCNT1 (TCNT1H[7:0]) | R/W | Counter, high byte. |
| TCNT1L (0x84) | [7:0] | TCNT1 (TCNT1L[7:0]) | R/W | Counter, low byte. |
| OCR1AH (0x89) | [7:0] | OCR1A (OCR1AH[7:0]) | R/W | Compare A, high byte. |
| OCR1AL (0x88) | [7:0] | OCR1A (OCR1AL[7:0]) | R/W | Compare A, low byte. Immediate update in mode 12, double-buffered in mode 14. |
| OCR1BH (0x8B) | [7:0] | OCR1B (OCR1BH[7:0]) | R/W | Compare B, high byte. |
| OCR1BL (0x8A) | [7:0] | OCR1B (OCR1BL[7:0]) | R/W | Compare B, low byte. Immediate update in mode 12, double-buffered in mode 14. |
| ICR1H (0x87) | [7:0] | ICR1 (ICR1H[7:0]) | R/W | **TOP** in modes 12 and 14, high byte. |
| ICR1L (0x86) | [7:0] | ICR1 (ICR1L[7:0]) | R/W | **TOP** in modes 12 and 14, low byte. Not double-buffered. |
| TIMSK1 (0x6F) | [7:6] | Reserved (TIMSK1[7:6]) | R | Reads 0, write 0. |
| TIMSK1 (0x6F) | [5] | ICIE1 (TIMSK1[5]) | R/W | Input capture interrupt enable (TIMER1_CAPT). In modes 12 and 14 this is the "counter reached TOP" interrupt, because ICF1 (TIFR1[5]) is set at TOP. |
| TIMSK1 (0x6F) | [4:3] | Reserved (TIMSK1[4:3]) | R | Reads 0, write 0. |
| TIMSK1 (0x6F) | [2] | OCIE1B (TIMSK1[2]) | R/W | Compare B interrupt enable (TIMER1_COMPB). |
| TIMSK1 (0x6F) | [1] | OCIE1A (TIMSK1[1]) | R/W | Compare A interrupt enable (TIMER1_COMPA). |
| TIMSK1 (0x6F) | [0] | TOIE1 (TIMSK1[0]) | R/W | Overflow interrupt enable (TIMER1_OVF). Mode 12: fires on wrap from 0xFFFF; mode 14: fires at TOP. |
| TIFR1 0x16 (0x36) | [7:6] | Reserved (TIFR1[7:6]) | R | Reads 0, write 0. |
| TIFR1 0x16 (0x36) | [5] | ICF1 (TIFR1[5]) | R/W | Input capture flag. In modes 12 and 14 it is set when the counter reaches TOP. Clear by writing `1`. |
| TIFR1 0x16 (0x36) | [4:3] | Reserved (TIFR1[4:3]) | R | Reads 0, write 0. |
| TIFR1 0x16 (0x36) | [2] | OCF1B (TIFR1[2]) | R/W | Compare B match flag. FOC1B (TCCR1C[6]) does not set it. Clear by writing `1`. |
| TIFR1 0x16 (0x36) | [1] | OCF1A (TIFR1[1]) | R/W | Compare A match flag. FOC1A (TCCR1C[7]) does not set it. Clear by writing `1`. |
| TIFR1 0x16 (0x36) | [0] | TOV1 (TIFR1[0]) | R/W | Overflow flag. Mode 12: set on wrap from 0xFFFF; mode 14: set at TOP. Clear by writing `1`. |

### 2.3 Timer2 registers (8-bit)

All bits reset to 0.

| Register (address) | Bits | Field | R/W | Role |
|---|---|---|---|---|
| TCCR2A (0xB0) | [7:6] | COM2A (TCCR2A[7:6]) | R/W | Compare output mode for OC2A. Any nonzero value connects OC2A to its pin (DDB3 (DDRB[3]) must be 1). COM2A1 (TCCR2A[7]) is the high bit, COM2A0 (TCCR2A[6]) the low bit. `00` disconnected (normal port operation) · `01` mode 2: toggle on match; mode 7: toggle on match with OCR2A (= TOP) · `10` mode 2: clear on match; mode 7: non-inverting PWM (on OC0A/OC2A this gives a constant level because OCRnA is TOP, see 5.2 and 5.6) · `11` mode 2: set on match; mode 7: inverting PWM. |
| TCCR2A (0xB0) | [5:4] | COM2B (TCCR2A[5:4]) | R/W | Compare output mode for OC2B. Any nonzero value connects OC2B to its pin (DDD3 (DDRD[3]) must be 1). COM2B1 (TCCR2A[5]) is the high bit, COM2B0 (TCCR2A[4]) the low bit. `00` disconnected · `01` mode 2: toggle on match; mode 7: **reserved** · `10` mode 2: clear on match; mode 7: non-inverting PWM · `11` mode 2: set on match; mode 7: inverting PWM. |
| TCCR2A (0xB0) | [3:2] | Reserved (TCCR2A[3:2]) | R | Reads 0, write 0. |
| TCCR2A (0xB0) | [1] | WGM21 (TCCR2A[1]) | R/W | Middle bit of the 3-bit mode code WGM2[2:0] (the top bit WGM22 is TCCR2B[3]). Mode 2 needs `1`, mode 7 needs `1`. |
| TCCR2A (0xB0) | [0] | WGM20 (TCCR2A[0]) | R/W | Low bit of the 3-bit mode code WGM2[2:0]. Mode 2 needs `0`, mode 7 needs `1`. |
| TCCR2B (0xB1) | [7] | FOC2A (TCCR2B[7]) | W | Force Output Compare A: writing `1` forces an immediate compare match on OC2A using the current COM2A (TCCR2A[7:6]) action. Non-PWM only (mode 2 yes; **mode 7: write 0**). No flag, no interrupt, no timer clear. Reads 0. |
| TCCR2B (0xB1) | [6] | FOC2B (TCCR2B[6]) | W | Force Output Compare B: same as FOC2A (TCCR2B[7]) for OC2B / COM2B (TCCR2A[5:4]). |
| TCCR2B (0xB1) | [5:4] | Reserved (TCCR2B[5:4]) | R | Reads 0, write 0. |
| TCCR2B (0xB1) | [3] | WGM22 (TCCR2B[3]) | R/W | Top bit of the 3-bit mode code WGM2[2:0] (the low two bits are WGM21 (TCCR2A[1]) and WGM20 (TCCR2A[0])). Mode 2 needs `0`; **mode 7 needs `1`**. |
| TCCR2B (0xB1) | [2:0] | CS2 (TCCR2B[2:0]) | R/W | Clock select, 3-bit code (CS22 (TCCR2B[2]), weight 4; CS21 (TCCR2B[1]), weight 2; CS20 (TCCR2B[0]), weight 1). `000` stops the timer; decode in Section 3. |
| TCNT2 (0xB2) | [7:0] | TCNT2 (TCNT2[7:0]) | R/W | Counter value. Writing blocks the next compare match. |
| OCR2A (0xB3) | [7:0] | OCR2A (OCR2A[7:0]) | R/W | Compare A; also TOP in modes 2 and 7. Immediate update in mode 2, double-buffered (latched at BOTTOM) in mode 7. |
| OCR2B (0xB4) | [7:0] | OCR2B (OCR2B[7:0]) | R/W | Compare B (duty cycle of OC2B in mode 7). Immediate in mode 2, double-buffered in mode 7. |
| TIMSK2 (0x70) | [7:3] | Reserved (TIMSK2[7:3]) | R | Reads 0, write 0. |
| TIMSK2 (0x70) | [2] | OCIE2B (TIMSK2[2]) | R/W | Compare B interrupt enable (TIMER2_COMPB; also needs the I-bit, SREG[7]). |
| TIMSK2 (0x70) | [1] | OCIE2A (TIMSK2[1]) | R/W | Compare A interrupt enable (TIMER2_COMPA). In modes 2 and 7 this fires at TOP. |
| TIMSK2 (0x70) | [0] | TOIE2 (TIMSK2[0]) | R/W | Overflow interrupt enable (TIMER2_OVF). Mode 2: fires on wrap from 0xFF; mode 7: fires at TOP. |
| TIFR2 0x17 (0x37) | [7:3] | Reserved (TIFR2[7:3]) | R | Reads 0, write 0. |
| TIFR2 0x17 (0x37) | [2] | OCF2B (TIFR2[2]) | R/W | Compare B match flag. Cleared by running the ISR or by writing `1`. |
| TIFR2 0x17 (0x37) | [1] | OCF2A (TIFR2[1]) | R/W | Compare A match flag (set at TOP in modes 2 and 7). Clear by writing `1`. |
| TIFR2 0x17 (0x37) | [0] | TOV2 (TIFR2[0]) | R/W | Overflow flag. Mode 2: set on wrap from 0xFF; mode 7: set at TOP. Clear by writing `1`. |

### 2.4 Prescaler reset register (GTCCR)

All bits reset to 0.

| Register (address) | Bits | Field | R/W | Role |
|---|---|---|---|---|
| GTCCR 0x23 (0x43) | [7] | TSM (GTCCR[7]) | R/W | Timer/Counter Synchronization Mode: `1` holds the prescaler-reset bits so timers can be configured and started together. |
| GTCCR 0x23 (0x43) | [6:2] | Reserved (GTCCR[6:2]) | R | Reads 0, write 0. |
| GTCCR 0x23 (0x43) | [1] | PSRASY (GTCCR[1]) | R/W | Resets the Timer2 prescaler. Hardware clears it (unless TSM (GTCCR[7]) = 1). |
| GTCCR 0x23 (0x43) | [0] | PSRSYNC (GTCCR[0]) | R/W | Resets the prescaler shared by Timer0 and Timer1 (affects both). Hardware clears it (unless TSM (GTCCR[7]) = 1). |

### 2.5 Waveform generation modes covered, with the register and bit that holds each WGM bit

**Timer0 and Timer2** (n = 0 or 2):

| Mode | WGMn2 (TCCRnB[3]) | WGMn1 (TCCRnA[1]) | WGMn0 (TCCRnA[0]) | Name | TOP | OCRnx update | TOVn set at |
|---|---|---|---|---|---|---|---|
| 2 | 0 | 1 | 0 | CTC | OCRnA | Immediate | MAX (0xFF) |
| 7 | 1 | 1 | 1 | Fast PWM | OCRnA | BOTTOM | TOP |

**Timer1:**

| Mode | WGM13 (TCCR1B[4]) | WGM12 (TCCR1B[3]) | WGM11 (TCCR1A[1]) | WGM10 (TCCR1A[0]) | Name | TOP | OCR1x update | TOV1 set at |
|---|---|---|---|---|---|---|---|---|
| 12 | 1 | 1 | 0 | 0 | CTC | ICR1 | Immediate | MAX (0xFFFF) |
| 14 | 1 | 1 | 1 | 0 | Fast PWM | ICR1 | BOTTOM | TOP |

The WGM bits have no standalone meaning; the combined code selects the mode.

---

## 3. Clock select (CSn[2:0]), the "start the timer" bits

The timer is **stopped** while CSn[2:0] = 000. Setting a nonzero value starts it. Write the WGM and COM bits first, then set CS last. CSn2 is TCCRnB[2], CSn1 is TCCRnB[1], CSn0 is TCCRnB[0].

### Timer0 (CS0 (TCCR0B[2:0])) and Timer1 (CS1 (TCCR1B[2:0]))

Here n = 0 or 1.

| CSn2 (TCCRnB[2]) | CSn1 (TCCRnB[1]) | CSn0 (TCCRnB[0]) | Clock source |
|---|---|---|---|
| 0 | 0 | 0 | Stopped |
| 0 | 0 | 1 | clkI/O / 1 |
| 0 | 1 | 0 | clkI/O / 8 |
| 0 | 1 | 1 | clkI/O / 64 |
| 1 | 0 | 0 | clkI/O / 256 |
| 1 | 0 | 1 | clkI/O / 1024 |

Codes `110` and `111` select an external clock pin and are outside the scope of this document. Timer0 and Timer1 share one prescaler but can use different divisors.

### Timer2 (CS2 (TCCR2B[2:0]))

| CS22 (TCCR2B[2]) | CS21 (TCCR2B[1]) | CS20 (TCCR2B[0]) | Clock source |
|---|---|---|---|
| 0 | 0 | 0 | Stopped |
| 0 | 0 | 1 | clkI/O / 1 |
| 0 | 1 | 0 | clkI/O / 8 |
| 0 | 1 | 1 | clkI/O / 32 |
| 1 | 0 | 0 | clkI/O / 64 |
| 1 | 0 | 1 | clkI/O / 128 |
| 1 | 1 | 0 | clkI/O / 256 |
| 1 | 1 | 1 | clkI/O / 1024 |

Timer2's prescaler has different divisors from Timer0/1 (32 and 128 are extra). It runs from clkI/O at reset.

---

## 4. Direct pin control: how COMnx works

### 4.1 The mechanism (same for all three timers)

In this section COMnx stands for the compare output mode field of channel x of Timer n: COMnA (TCCRnA[7:6]) for channel A and COMnB (TCCRnA[5:4]) for channel B. COMnx1 is its high bit (TCCRnA[7] for A, TCCRnA[5] for B) and COMnx0 its low bit (TCCRnA[6] for A, TCCRnA[4] for B).

Each compare channel has a **waveform generator** that drives an internal OCnx flip-flop. The COMnx bits do two jobs:

1. **Connect or disconnect** the pin. If either COMnx bit is 1, the OCnx output overrides the normal PORT function of the pin.
2. **Select the action** the waveform generator performs on the OCnx flip-flop. The meaning of the bits **depends on the WGM mode**.

Rules that apply everywhere:

- **The DDR bit must be 1.** COM bits connect the output, but the direction register still decides whether the driver is on. With the DDR bit (see Output pins in Section 1) = 0 you will see nothing on the pin.
- **COMnx = 00** always means normal port operation (PORT/DDR control the pin). No action on the OCnx flip-flop.
- **COM bits are not double-buffered.** A change takes effect at the *next compare match*, not at the next period boundary (except that FOC can force it immediately, see 4.5).
- **Set the initial OCnx state before enabling the output driver.** The OCnx flip-flop resets to 0 and keeps its value across mode changes, so the easiest way to preload it is a FOC strobe in a non-PWM mode, *then* set the DDR bit.
- **Writing TCNTn blocks the next compare match.** Don't write TCNTn to a value equal to OCRnx when using the output unit, or that match is missed.
- **The counting sequence is controlled only by WGM.** COM bits never change when or how the counter counts.

### 4.2 CTC modes (Timer0 mode 2, Timer1 mode 12, Timer2 mode 2): non-PWM table

Datasheet Tables 14-2 / 14-5 (Timer0), 15-2 (Timer1), 17-2 / 17-5 (Timer2). Identical for channel A and B.

| COMnx1 (TCCRnA[7] for A, TCCRnA[5] for B) | COMnx0 (TCCRnA[6] for A, TCCRnA[4] for B) | Action on compare match (TCNTn == OCRnx) |
|---|---|---|
| 0 | 0 | OCnx disconnected, normal port operation |
| 0 | 1 | **Toggle** OCnx |
| 1 | 0 | **Clear** OCnx (drive low) |
| 1 | 1 | **Set** OCnx (drive high) |

Use cases:

- **Toggle (01)** produces a square wave with exactly 50 % duty cycle. This is the main use of CTC pin control.
- **Clear (10) / Set (11)** are one-shot style: the pin changes once at the match and then *stays there*, because no further action undoes it. On their own they do not make a repeating waveform. They are useful for edge-triggering an external device at a precise time, and for combining with software or a second compare channel (e.g. Set on match A, then Clear on match B in software).
- In CTC the counter is cleared at TOP, but **OCnx is not forced to any level at TOP**. Only compare matches act on the pin.

**TOP register address**

| Timer | TOP register | Channel that fires every period | The other channel |
|---|---|---|---|
| Timer0 mode 2 | OCR0A | OC0A (toggles at TOP) | OC0B compares against OCR0B; it only matches if OCR0B ≤ OCR0A |
| Timer2 mode 2 | OCR2A | OC2A | OC2B, same rule |
| Timer1 mode 12 | **ICR1** | *Neither*. Both OC1A and OC1B are free, ordinary compare channels | n/a |

Timer1 mode 12 is the notable one: because TOP comes from ICR1, **both** OC1A and OC1B can be toggled/set/cleared independently at their own compare values, and the input capture function on ICP1 is disabled while ICR1 is used as TOP.

**Frequency, toggle mode.**

Datasheet formula (TOP = OCRnA):

```
f_OCnA = f_clkI/O / ( 2 · N · (1 + OCRnA) )
```

N is the prescaler divisor. Maximum is f_clkI/O / 2 with OCRnA = 0.

- Timer1 mode 12: each channel toggles once per counter period, so `f_OC1x = f_clkI/O / ( 2 · N · (1 + ICR1) )`, provided OCR1x ≤ ICR1. The OCR1x value sets the *phase* of the edges, not the frequency.
- Timer0/2 OCnB toggle: same formula as OCnA, with OCRnA as TOP.

**CTC interrupts and flags.** OCFnA (TIFRn[1]) is set at TOP (Timer0/2). In Timer1 mode 12, **ICF1 (TIFR1[5])** is set at TOP (enable with ICIE1 (TIMSK1[5])), while OCF1A (TIFR1[1]) and OCF1B (TIFR1[2]) fire at their own matches. TOVn (TIFRn[0]) is set only when the counter wraps from MAX, which in CTC usually means "TOP was missed" and is not a normal event.

**CTC caution (no double buffering).** If you write a new TOP lower than the current TCNTn, that compare is missed and the counter runs to MAX (0xFF or 0xFFFF), wraps, and only then matches. Update TOP from the compare ISR, or with a large margin.

### 4.3 Fast PWM modes (Timer0 mode 7, Timer1 mode 14, Timer2 mode 7): PWM table

Datasheet Tables 14-3 / 14-6, 15-3, 17-3 / 17-6.

| COMnx1 (TCCRnA[7] for A, TCCRnA[5] for B) | COMnx0 (TCCRnA[6] for A, TCCRnA[4] for B) | Behavior |
|---|---|---|
| 0 | 0 | Disconnected, normal port operation |
| 0 | 1 | **Channel A only (OC0A / OC2A, mode 7 where TOP = OCRnA):** toggle on compare match. **Channel B (OC0B / OC2B): Reserved, do not use.** |
| 1 | 0 | **Non-inverting PWM:** clear on compare match, **set at BOTTOM** |
| 1 | 1 | **Inverting PWM:** set on compare match, **clear at BOTTOM** |

Timer1 variation (Table 15-3): in mode 14, COM1A (TCCR1A[7:6]) = `01` toggles OC1A on compare match with OCR1A; COM1B (TCCR1A[5:4]) = `01` leaves OC1B disconnected (normal port operation) rather than being "Reserved."

Key differences from CTC:

1. **Set/Clear are no longer one-shot.** The pin is driven to a defined level at *both* BOTTOM (by the counter restarting) and at the compare match. That pair of actions is what makes a periodic waveform, and COMnx = `10` vs `11` only selects the polarity.
2. **Duty cycle (non-inverting)** = (OCRnx + 1) / (TOP + 1). The pin is high from BOTTOM until the match.
3. **OCRnx is double-buffered**, so you can write it any time and it takes effect cleanly at BOTTOM. There are no glitches or odd-length pulses.
4. **TOP is a PWM period, not a compare point.** The frequency does not depend on OCRnx (on Timer0/2 mode 7, OCRnA is TOP and sets the frequency).
5. **Toggle (01) is a special case** available only on channel A (OC0A, OC1A, OC2A). It toggles once per timer period, giving a 50 % square wave at half the PWM frequency (see 5.2, 5.4 and 5.6).

**Extreme values (all fast-PWM modes):**

| OCRnx value | Non-inverting (`10`) | Inverting (`11`) |
|---|---|---|
| = BOTTOM (0) | Narrow high spike: one timer clock per period | Narrow low spike: one timer clock per period |
| = TOP | Compare match ignored, set at BOTTOM, so the output stays **constantly high** | Compare match ignored, clear at BOTTOM, so the output stays **constantly low** |
| Between | Normal PWM | Normal PWM, inverted |

The datasheet describes the OCRnx = TOP case as a compare match that is ignored while the set or clear still happens at BOTTOM, giving a constant level. Test your boundary case on hardware if you rely on 0 % or 100 %.

### 4.4 CTC vs Fast PWM

| Aspect | CTC (modes 2 / 12) | Fast PWM (modes 7 / 14) |
|---|---|---|
| Counter sequence | 0 → TOP, clear | 0 → TOP, clear (same shape) |
| What COMnx = `01` does | **Toggle** on match, on **A and B** channels | **Toggle on channel A only** (once per period, 50 % duty). B is Reserved on Timer0/2, disconnected on Timer1 |
| What COMnx = `10` does | **Clear** on match; stays low (one-shot) | **Non-inverting PWM:** clear on match **and** set at BOTTOM |
| What COMnx = `11` does | **Set** on match; stays high (one-shot) | **Inverting PWM:** set on match **and** clear at BOTTOM |
| Action at BOTTOM / TOP | None. The pin is only touched by matches | Pin forced to its "start" level at BOTTOM |
| OCRx buffering | **None** (immediate update) | **Double-buffered**, latched at BOTTOM |
| Typical output | 50 % square wave of precisely set frequency | Variable-duty PWM at fixed frequency |
| Frequency formula | `f = f_clk / (2·N·(1+TOP))` (toggle) | `f_PWM = f_clk / (N·(1+TOP))` |
| FOC strobe | Active | **Ignored.** Must be written 0 |
| TOV flag set at | MAX | TOP |
| Cost of changing TOP live | May miss the match | Timer0/2: TOP is OCRnA, which is buffered, so safe. Timer1: TOP is ICR1, **not** buffered, so not safe |

Note the factor of 2: the toggle waveform needs two matches per cycle, so for the same TOP and prescaler a CTC toggle output is **half** the frequency of a fast-PWM output.

Waveform sketch (TOP = 7, OCRx = 3, one OCnx pin):

```
TCNT:        0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 ...
CTC toggle (COM=01), toggles at TCNT==3 only (one edge per period):
             ______________|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|______________
Fast PWM non-inv (COM=10): high from BOTTOM, low after match at 3
             |¯¯¯¯¯¯¯¯|________|¯¯¯¯¯¯¯¯|________|¯¯¯¯¯
Fast PWM inv (COM=11):
             |________|¯¯¯¯¯¯¯¯|________|¯¯¯¯¯¯¯¯|_____
```

### 4.5 Force Output Compare (FOCnx)

- **Location:** FOC0A (TCCR0B[7]) / FOC0B (TCCR0B[6]); FOC2A (TCCR2B[7]) / FOC2B (TCCR2B[6]); FOC1A (TCCR1C[7]) / FOC1B (TCCR1C[6]) (note that Timer1 uses **TCCR1C**).
- **Active only in non-PWM modes** (so in modes 2 and 12, not in 7 or 14). In PWM modes write 0.
- It's a **strobe**: writing 1 acts as an immediate compare match, using whatever COMnx currently says (set/clear/toggle). It always reads 0.
- A strobe does **not** set OCFnx, does **not** raise an interrupt, and does **not** clear the timer even in CTC mode.
- Use it to preload the pin's starting level (e.g. COMnx = `10` + FOC to force low) before turning on the DDR bit, or to force an edge now.

---

## 5. Per-timer reference

### 5.1 Timer0 mode 2: CTC (WGM0[2:0] = 010)

**Registers to configure**

| Register | Setting |
|---|---|
| TCCR0A | `WGM01 = 1` (TCCR0A[1]), `WGM00 = 0` (TCCR0A[0]); COM0A (TCCR0A[7:6]) and COM0B (TCCR0A[5:4]) per the CTC table |
| TCCR0B | `WGM02 = 0` (TCCR0B[3]); CS0 (TCCR0B[2:0]) for prescaler; FOC0A (TCCR0B[7]) / FOC0B (TCCR0B[6]) optional strobes |
| OCR0A | TOP. Counter cycle = OCR0A + 1 timer clocks |
| OCR0B | Optional second compare point (only matches if ≤ OCR0A) |
| TIMSK0 | `OCIE0A` (TIMSK0[1]) for the TOP interrupt; `OCIE0B` (TIMSK0[2]) for the B interrupt; `TOIE0` (TIMSK0[0]) rarely useful |
| DDRD | `DDD6` (DDRD[6]) for OC0A and/or `DDD5` (DDRD[5]) for OC0B |

**All configuration options**

| Feature | Options |
|---|---|
| OC0A (PD6) | COM0A (TCCR0A[7:6]): 00 off · 01 toggle · 10 clear · 11 set (on match with OCR0A) |
| OC0B (PD5) | COM0B (TCCR0A[5:4]): 00 off · 01 toggle · 10 clear · 11 set (on match with OCR0B) |
| Clock | CS0 (TCCR0B[2:0]): Stopped, /1, /8, /64, /256, /1024 |
| Interrupts | OCF0A (TIFR0[1]) (at TOP), OCF0B (TIFR0[2]) (at OCR0B), TOV0 (TIFR0[0]) (only on wrap from 0xFF) |
| FOC0A (TCCR0B[7]) / FOC0B (TCCR0B[6]) | Immediate forced match |

**Frequency:** `f_OC0A = f_clk / (2·N·(1+OCR0A))` (toggle).

```c
// ~500 Hz square wave on OC0A (PD6) at 16 MHz: /64, OCR0A = 249
DDRD  |= (1 << DDD6);
TCCR0A = (1 << COM0A0) | (1 << WGM01);   // toggle OC0A, CTC
OCR0A  = 249;
TCCR0B = (1 << CS01) | (1 << CS00);      // /64, starts timer
// 16e6 / (2*64*250) = 500 Hz
```

### 5.2 Timer0 mode 7: Fast PWM, TOP = OCR0A (WGM0[2:0] = 111)

**Registers to configure**

| Register | Setting |
|---|---|
| TCCR0A | `WGM01 = 1` (TCCR0A[1]), `WGM00 = 1` (TCCR0A[0]); COM0A (TCCR0A[7:6]) / COM0B (TCCR0A[5:4]) per the fast-PWM table |
| TCCR0B | `WGM02 = 1` (TCCR0B[3]); CS0 (TCCR0B[2:0]); FOC0A (TCCR0B[7]) and FOC0B (TCCR0B[6]) = 0 |
| OCR0A | **TOP**, sets the PWM period. Also the toggle compare for OC0A |
| OCR0B | Duty cycle for OC0B |
| DDRD | `DDD5` (DDRD[5]) for OC0B PWM; `DDD6` (DDRD[6]) if using OC0A toggle |

**All configuration options**

| Pin | COM0x | Result in mode 7 |
|---|---|---|
| OC0A (PD6) | COM0A (TCCR0A[7:6]) = 00 | Disconnected |
| OC0A (PD6) | COM0A (TCCR0A[7:6]) = 01 | **Toggle** at OCR0A match. Gives a 50 % wave at `f = f_clk/(2·N·(1+OCR0A))` with double-buffered OCR0A |
| OC0A (PD6) | COM0A (TCCR0A[7:6]) = 10 / 11 | Because OCR0A *is* TOP, the match is ignored and the pin is held at its BOTTOM level: `10` gives a constant high, `11` a constant low. Not useful for PWM |
| OC0B (PD5) | COM0B (TCCR0A[5:4]) = 00 | Disconnected |
| OC0B (PD5) | COM0B (TCCR0A[5:4]) = 01 | **Reserved**, do not use |
| OC0B (PD5) | COM0B (TCCR0A[5:4]) = 10 | Non-inverting PWM: duty = (OCR0B+1)/(OCR0A+1) |
| OC0B (PD5) | COM0B (TCCR0A[5:4]) = 11 | Inverting PWM |

So **in mode 7, OC0B is the variable-duty PWM channel, and OC0A is the frequency channel** (use it as a toggle clock output, or leave it disconnected and free for GPIO).

**Frequency:** `f_PWM = f_clk / (N·(1+OCR0A))` from the datasheet's mode-3 formula `f_clk/(N·256)` by replacing 256 with OCR0A + 1). With TOP = OCR0A you can pick a PWM frequency freely, trading it for resolution.

**Flags:** TOV0 (TIFR0[0]) is set at TOP (= OCR0A), the same moment OCF0A (TIFR0[1]) is set and OCR0x gets updated from its buffer.

```c
// 1 kHz PWM on OC0B (PD5), 25 % duty, 16 MHz: /64, TOP = 249
DDRD  |= (1 << DDD5);
TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);   // non-inverting on OC0B
TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);       // mode 7, /64
OCR0A  = 249;                  // TOP -> 16e6/(64*250) = 1000 Hz
OCR0B  = 62;                   // (62+1)/250 = 25.2 %
```

### 5.3 Timer1 mode 12: CTC, TOP = ICR1 (WGM1[3:0] = 1100)

**Registers to configure**

| Register | Setting |
|---|---|
| TCCR1A | `WGM11 = 0` (TCCR1A[1]), `WGM10 = 0` (TCCR1A[0]); COM1A (TCCR1A[7:6]) and COM1B (TCCR1A[5:4]) per the CTC table |
| TCCR1B | `WGM13 = 1` (TCCR1B[4]), `WGM12 = 1` (TCCR1B[3]); CS1 (TCCR1B[2:0]) clock; ICNC1 (TCCR1B[7]) / ICES1 (TCCR1B[6]) irrelevant (ICP1 is disconnected) |
| TCCR1C | FOC1A (TCCR1C[7]) / FOC1B (TCCR1C[6]) optional strobes |
| ICR1 | **TOP** |
| OCR1A / OCR1B | Independent compare points (should be ≤ ICR1) |
| TIMSK1 | `ICIE1` (TIMSK1[5]) = interrupt at TOP; `OCIE1A` (TIMSK1[1]) / `OCIE1B` (TIMSK1[2]) = interrupts at the compare points; `TOIE1` (TIMSK1[0]) |
| DDRB | `DDB1` (DDRB[1]) for OC1A and/or `DDB2` (DDRB[2]) for OC1B |

**All configuration options**

| Feature | Options |
|---|---|
| OC1A (PB1) | COM1A (TCCR1A[7:6]): 00 off · 01 toggle · 10 clear · 11 set (on match with OCR1A) |
| OC1B (PB2) | COM1B (TCCR1A[5:4]): 00 off · 01 toggle · 10 clear · 11 set (on match with OCR1B) |
| Clock | CS1 (TCCR1B[2:0]): Stopped, /1, /8, /64, /256, /1024 |
| Interrupts | ICF1 (TIFR1[5]) = TOP reached; OCF1A (TIFR1[1]); OCF1B (TIFR1[2]); TOV1 (TIFR1[0]) (only on wrap from 0xFFFF) |
| FOC1A (TCCR1C[7]) / FOC1B (TCCR1C[6]) | Forced match |

In mode 12, OCR1A and OCR1B are both ordinary compare registers, so both output channels work independently. The cost is that ICR1 is not double-buffered, and the capture pin ICP1 is no longer usable.

**Frequency:** `f_OC1x = f_clk / (2·N·(1+ICR1))` for a toggle output. Resolution of the "period" is a full 16 bits.

```c
// 0.5 Hz on OC1A (PB1) and a phase-shifted 0.5 Hz on OC1B (PB2), 16 MHz, /1024
DDRB  |= (1 << DDB1) | (1 << DDB2);
TCCR1A = (1 << COM1A0) | (1 << COM1B0);          // toggle both
TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10);  // mode 12, /1024
ICR1   = 15624;                                  // period 1 s -> toggle every 1 s
OCR1A  = 0;                                      // toggles at start of each period
OCR1B  = 7812;                                   // toggles half a period later
```

### 5.4 Timer1 mode 14: Fast PWM, TOP = ICR1 (WGM1[3:0] = 1110)

**Registers to configure**

| Register | Setting |
|---|---|
| TCCR1A | `WGM11 = 1` (TCCR1A[1]), `WGM10 = 0` (TCCR1A[0]); COM1A (TCCR1A[7:6]) / COM1B (TCCR1A[5:4]) per the fast-PWM table |
| TCCR1B | `WGM13 = 1` (TCCR1B[4]), `WGM12 = 1` (TCCR1B[3]); CS1 (TCCR1B[2:0]) |
| TCCR1C | FOC1A (TCCR1C[7]) and FOC1B (TCCR1C[6]) must be 0 |
| ICR1 | **TOP**, sets frequency and resolution |
| OCR1A / OCR1B | Duty cycles. **Both channels are full PWM** because TOP is ICR1, not one of the compare registers |
| TIMSK1 | `ICIE1` (TIMSK1[5]) for TOP; `OCIE1A` (TIMSK1[1]) / `OCIE1B` (TIMSK1[2]) for the match points; `TOIE1` (TIMSK1[0]) |
| DDRB | `DDB1` (DDRB[1]) and `DDB2` (DDRB[2]) |

**All configuration options**

| Pin | COM1x | Result in mode 14 |
|---|---|---|
| OC1A (PB1) | COM1A (TCCR1A[7:6]) = 00 | Disconnected |
| OC1A (PB1) | COM1A (TCCR1A[7:6]) = 01 | Toggle on compare match with OCR1A (50 % square wave, see below) |
| OC1A (PB1) | COM1A (TCCR1A[7:6]) = 10 | Non-inverting PWM |
| OC1A (PB1) | COM1A (TCCR1A[7:6]) = 11 | Inverting PWM |
| OC1B (PB2) | COM1B (TCCR1A[5:4]) = 00 | Disconnected |
| OC1B (PB2) | COM1B (TCCR1A[5:4]) = 01 | Disconnected (normal port operation) |
| OC1B (PB2) | COM1B (TCCR1A[5:4]) = 10 | Non-inverting PWM |
| OC1B (PB2) | COM1B (TCCR1A[5:4]) = 11 | Inverting PWM |

**OC1A toggle (COM1A (TCCR1A[7:6]) = 01) in mode 14.** OC1A toggles every time TCNT1 matches OCR1A. TCNT1 passes OCR1A once per timer period (0 → ICR1), so OC1A toggles once per period and one full output cycle takes two timer periods. High time and low time are therefore both ICR1 + 1 timer ticks, a 50 % duty cycle **whatever the value of OCR1A** (as long as OCR1A ≤ ICR1); OCR1A only sets the *phase* of the edges within the period, and it is double-buffered. `f_OC1A = f_clk / ( 2 · N · (1 + ICR1) )`, half the PWM frequency. OC1B can still run PWM while OC1A toggles, but OC1B has no toggle option (`01` leaves it disconnected).

**Frequency and resolution**

```
f_PWM  = f_clk / ( N · (1 + ICR1) )
R_bits = log2( ICR1 + 1 )          (min ICR1 = 3 → 2-bit; max 0xFFFF → 16-bit)
Duty (non-inverting) = (OCR1x + 1) / (ICR1 + 1)
```

**Rules:**

- **TOP must be ≥ every OCR1x.** If TOP is lower than an OCR1x value, that channel never matches.
- **OCR1x is double-buffered** (updated at BOTTOM, together with TOV1 (TIFR1[0])), **ICR1 is not.** Lowering ICR1 below the current TCNT1 on the fly misses the TOP match, and the counter runs to 0xFFFF before restarting. Set ICR1 once at startup.
- At TOP, TOV1 (TIFR1[0]) **and** ICF1 (TIFR1[5]) are set together (ICF1 acts as a "TOP reached" flag since ICR1 is TOP).
- Edge values: OCR1x = 0 gives a one-clock spike per period; OCR1x = ICR1 gives a constant level (high for `10`, low for `11`).

Section 15.9.3 says COM1x = 2 gives inverted and COM1x = 3 gives non-inverted PWM. This is a known error. Table 15-3 and Section 15.9.3's own first paragraph are correct: **COM1x = 10 is non-inverting, COM1x = 11 is inverting.**

```c
// 50 Hz servo PWM on OC1A (PB1) and OC1B (PB2), 16 MHz, /8
DDRB  |= (1 << DDB1) | (1 << DDB2);
TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);          // non-inverting, mode 14 low bits
TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);             // mode 14, /8
ICR1   = 39999;        // 16e6 / (8 * 40000) = 50 Hz
OCR1A  = 3000;         // 1.5 ms pulse (tick = 0.5 us)
OCR1B  = 2000;         // 1.0 ms pulse
```

### 5.5 Timer2 mode 2: CTC (WGM2[2:0] = 010)

Same behavior as Timer0 mode 2, with Timer2's own registers and prescaler set.

**Registers to configure**

| Register | Setting |
|---|---|
| TCCR2A | `WGM21 = 1` (TCCR2A[1]), `WGM20 = 0` (TCCR2A[0]); COM2A (TCCR2A[7:6]) / COM2B (TCCR2A[5:4]) per the CTC table |
| TCCR2B | `WGM22 = 0` (TCCR2B[3]); CS2 (TCCR2B[2:0]); FOC2A (TCCR2B[7]) / FOC2B (TCCR2B[6]) optional |
| OCR2A | TOP |
| OCR2B | Second compare (only matches if ≤ OCR2A) |
| TIMSK2 | `OCIE2A` (TIMSK2[1]), `OCIE2B` (TIMSK2[2]), `TOIE2` (TIMSK2[0]) |
| DDRB / DDRD | `DDB3` (DDRB[3]) for OC2A; `DDD3` (DDRD[3]) for OC2B |

**All configuration options**

| Feature | Options |
|---|---|
| OC2A (PB3) | COM2A (TCCR2A[7:6]): 00 off · 01 toggle · 10 clear · 11 set |
| OC2B (PD3) | COM2B (TCCR2A[5:4]): 00 off · 01 toggle · 10 clear · 11 set |
| Clock | CS2 (TCCR2B[2:0]): Stopped, /1, /8, /32, /64, /128, /256, /1024 |
| Interrupts | OCF2A (TIFR2[1]) (at TOP), OCF2B (TIFR2[2]), TOV2 (TIFR2[0]) (wrap from 0xFF) |

**Frequency:** `f_OC2A = f_clkI/O / (2·N·(1+OCR2A))` with N ∈ {1, 8, 32, 64, 128, 256, 1024}.

```c
// 1 kHz square wave on OC2A (PB3) at 16 MHz: /64, OCR2A = 124
DDRB  |= (1 << DDB3);
TCCR2A = (1 << COM2A0) | (1 << WGM21);       // toggle OC2A, CTC
OCR2A  = 124;
TCCR2B = (1 << CS22);                        // /64 (CS2 (TCCR2B[2:0]) = 100), starts timer
// 16e6 / (2*64*125) = 1000 Hz
```

### 5.6 Timer2 mode 7: Fast PWM, TOP = OCR2A (WGM2[2:0] = 111)

Same behavior as Timer0 mode 7.

**Registers to configure**

| Register | Setting |
|---|---|
| TCCR2A | `WGM21 = 1` (TCCR2A[1]), `WGM20 = 1` (TCCR2A[0]); COM2A (TCCR2A[7:6]) / COM2B (TCCR2A[5:4]) per the fast-PWM table |
| TCCR2B | `WGM22 = 1` (TCCR2B[3]); CS2 (TCCR2B[2:0]); FOC2A (TCCR2B[7]) and FOC2B (TCCR2B[6]) = 0 |
| OCR2A | **TOP** (and toggle compare for OC2A) |
| OCR2B | Duty for OC2B |
| DDRD / DDRB | `DDD3` (DDRD[3]) for OC2B PWM; `DDB3` (DDRB[3]) for OC2A toggle |

**All configuration options**

| Pin | COM2x | Result in mode 7 |
|---|---|---|
| OC2A (PB3) | COM2A (TCCR2A[7:6]) = 00 | Disconnected |
| OC2A (PB3) | COM2A (TCCR2A[7:6]) = 01 | Toggle at OCR2A match (50 % wave, `f = f_clk/(2·N·(1+OCR2A))`) |
| OC2A (PB3) | COM2A (TCCR2A[7:6]) = 10 / 11 | Constant level (OCR2A = TOP, match ignored): `10` constant high, `11` constant low |
| OC2B (PD3) | COM2B (TCCR2A[5:4]) = 00 | Disconnected |
| OC2B (PD3) | COM2B (TCCR2A[5:4]) = 01 | **Reserved** |
| OC2B (PD3) | COM2B (TCCR2A[5:4]) = 10 | Non-inverting PWM: duty = (OCR2B+1)/(OCR2A+1) |
| OC2B (PD3) | COM2B (TCCR2A[5:4]) = 11 | Inverting PWM |

**Frequency:** `f_PWM = f_clkI/O / (N·(1+OCR2A))`.

```c
// 8 kHz PWM on OC2B (PD3), 50 % duty, 16 MHz, /8, TOP = 249
DDRD  |= (1 << DDD3);
TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);   // non-inverting on OC2B
TCCR2B = (1 << WGM22) | (1 << CS21);                      // mode 7, /8
OCR2A  = 249;                  // 16e6 / (8 * 250) = 8000 Hz
OCR2B  = 124;                  // (124+1)/250 = 50 %
```

---

## 6. Interrupt vectors (avr-libc names)

| Source | Vector | Enable bit | Flag |
|---|---|---|---|
| Timer0 compare A | `TIMER0_COMPA_vect` | OCIE0A (TIMSK0[1]) | OCF0A (TIFR0[1]) |
| Timer0 compare B | `TIMER0_COMPB_vect` | OCIE0B (TIMSK0[2]) | OCF0B (TIFR0[2]) |
| Timer0 overflow | `TIMER0_OVF_vect` | TOIE0 (TIMSK0[0]) | TOV0 (TIFR0[0]) |
| Timer1 capture (also "TOP" in mode 12/14) | `TIMER1_CAPT_vect` | ICIE1 (TIMSK1[5]) | ICF1 (TIFR1[5]) |
| Timer1 compare A | `TIMER1_COMPA_vect` | OCIE1A (TIMSK1[1]) | OCF1A (TIFR1[1]) |
| Timer1 compare B | `TIMER1_COMPB_vect` | OCIE1B (TIMSK1[2]) | OCF1B (TIFR1[2]) |
| Timer1 overflow | `TIMER1_OVF_vect` | TOIE1 (TIMSK1[0]) | TOV1 (TIFR1[0]) |
| Timer2 compare A | `TIMER2_COMPA_vect` | OCIE2A (TIMSK2[1]) | OCF2A (TIFR2[1]) |
| Timer2 compare B | `TIMER2_COMPB_vect` | OCIE2B (TIMSK2[2]) | OCF2B (TIFR2[2]) |
| Timer2 overflow | `TIMER2_OVF_vect` | TOIE2 (TIMSK2[0]) | TOV2 (TIFR2[0]) |

All interrupts also need the global enable (`sei()`, the I-bit, SREG[7]). Flags clear automatically when the ISR runs, or by writing a 1 to the flag bit. FOC strobes never set OCFnx.

---

## 7. Quick checklist for pin-output setup

1. Pick the mode and write the **WGM bits** (split across TCCRnA and TCCRnB, e.g. WGM01 (TCCR0A[1]), WGM00 (TCCR0A[0]), WGM02 (TCCR0B[3]); four bits for Timer1: WGM13 (TCCR1B[4]), WGM12 (TCCR1B[3]), WGM11 (TCCR1A[1]), WGM10 (TCCR1A[0])).
2. Load the **TOP register** (OCRnA, or ICR1 for Timer1 modes 12 and 14) and the compare/duty register(s).
3. Write the **COM bits**: COMnA (TCCRnA[7:6]) and COMnB (TCCRnA[5:4]). Meaning depends on whether you're in CTC or Fast PWM (sections 4.2 and 4.3).
4. Optionally preload the pin level with a **FOC strobe** (CTC modes only).
5. Set the **DDR bit** to make the pin an output.
6. Set **CSn (TCCRnB[2:0])** last, which starts the timer.
7. Enable interrupts (TIMSKn, `sei()`) if needed.

## 8. Common mistakes

- **Missing a match:** writing TCNTn, or lowering a non-buffered TOP (OCRnA in CTC, ICR1 in mode 12/14) below TCNTn, skips a compare.
- **PB2 / PB3 are SPI pins** (SS, MOSI). OC1B and OC2A share them.
- **Timer1 mode 12/14 disable input capture** on ICP1, since ICR1 is occupied as TOP.
- **Timer0 and Timer1 share a prescaler.** A prescaler reset (PSRSYNC (GTCCR[0])) affects both.
- **Fast-PWM TOP changes** are safe on OCR0A / OCR2A (double-buffered), but not on ICR1.