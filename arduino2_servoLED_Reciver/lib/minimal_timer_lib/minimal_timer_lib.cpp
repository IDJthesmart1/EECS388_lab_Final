extern "C" {
#include <stdint.h>
}

#include <avr/io.h>

#include "minimal_timer_lib.hpp"

// Everything in this anonymous namespace has internal linkage: it cannot be
// referenced, declared, or linked against from any other translation unit.
namespace {

//------------------------------------------------------------------------------
// Register configuration functions (no input validation, for internal use only)
//------------------------------------------------------------------------------

// Set the waveform generation mode (WGM)
  void config_mode(uint8_t timer_id, uint8_t mode) {
    if (timer_id == 0) {
      TCCR0A = (TCCR0A & ~0b11) | (mode & 0b11);
      TCCR0B = (TCCR0B & ~0b1000) | ((mode & 0b100) << 1);
    } else if (timer_id == 1) {
      TCCR1A = (TCCR1A & ~0b11) | (mode & 0b11);
      TCCR1B = (TCCR1B & ~0b11000) | ((mode & 0b1100) << 1);
    } else {
      TCCR2A = (TCCR2A & ~0b11) | (mode & 0b11);
      TCCR2B = (TCCR2B & ~0b1000) | ((mode & 0b100) << 1);
    }
  }

  // Set the clock TOP register determining the ticks per period
  void config_top(uint8_t timer_id, uint16_t top) {
    if (timer_id == 0) {
      OCR0A = static_cast<uint8_t>(top);
    } else if (timer_id == 1) {
      OCR1A = top;
    } else {
      OCR2A = static_cast<uint8_t>(top);
    }
  }

  void config_compare(uint8_t timer_id, uint16_t top, uint8_t duty_cycle) {
    const uint16_t compare = duty_cycle * (top / 200);
    if (timer_id == 0) {
      OCR0B = static_cast<uint8_t>(compare);
    } else if (timer_id == 1) {
      OCR1B = compare;
    } else {
      OCR2B = static_cast<uint8_t>(compare);
    }
  }

  // Set the channel/pin configuration
  void config_channel(uint8_t timer_id, uint8_t ctrl) {
    // ctrl: 0bXXXX first two bits = channel A, second two bits = channel B
    //       0b00 = none, 0b01 = CTC toggle each period, 0b11 = PWM
    if (timer_id == 0) {
      TCCR0A = (TCCR0A & ~0b00110000) | (ctrl << 4);
      DDRD |= (1 << PD5); // Set GPIO_5 as output
    } else if (timer_id == 1) {
      TCCR1A = (TCCR1A & ~0b00110000) | (ctrl << 4);
      DDRB |= (1 << PB2); // Set GPIO_10 as output
    } else {
      TCCR2A = (TCCR2A & ~0b00110000) | (ctrl << 4);
      DDRD |= (1 << PD3); // Set GPIO_3 as output
    }
  }

  // Set the prescaler and immediately start the timer
  void config_scaler(uint8_t timer_id, uint8_t scaler) {
    if (timer_id == 0) {
      TCCR0B = (TCCR0B & ~0b111) | (scaler);
    } else if (timer_id == 1) {
      TCCR1B = (TCCR1B & ~0b111) | (scaler);
    } else {
      TCCR2B = (TCCR2B & ~0b111) | (scaler);
    }
  }

  void config_interrupt(uint8_t timer_id, bool enable) {
    if (timer_id == 0) {
      if (enable) {
        TIMSK0 |= (1 << OCIE0A);
      } else {
        TIMSK0 &= ~(1 << OCIE0A);
      }
    } else if (timer_id == 1) {
      if (enable) {
        TIMSK1 |= (1 << OCIE1A);
      } else {
        TIMSK1 &= ~(1 << OCIE1A);
      }
    } else {
      if (enable) {
        TIMSK2 |= (1 << OCIE2A);
      } else {
        TIMSK2 &= ~(1 << OCIE2A);
      }
    }
  }

  //------------------------------------------------------------------------------
  // Constants and helper functions
  //------------------------------------------------------------------------------

  struct Config {
    uint16_t ticks;
    uint8_t scaler;
    uint16_t rounding_error_us;
  };

  struct PrescalerEntry {
    uint32_t max_ticks;       // Exclusive upper bound
    uint8_t scaler_shift;     // log2(prescaler) for fast division/modulo
    uint8_t code;             // Register bitmask
  };

  // Timer 0 (8-bit: max 256 ticks and 5 prescalers)
  constexpr PrescalerEntry TIMER0_LUT[] = {
    { 256,        0,  0b001 }, // prescaler 1
    { 2048,       3,  0b010 }, // prescaler 8
    { 16384,      6,  0b011 }, // prescaler 64
    { 65536,      8,  0b100 }, // prescaler 256
    { UINT32_MAX, 10, 0b101 }  // prescaler 1024
  };

  // Timer 1 (16-bit: max 65536 ticks and 5 prescalers)
  constexpr PrescalerEntry TIMER1_LUT[] = {
    { 65536,      0,  0b001 }, // prescaler 1
    { 524288,     3,  0b010 }, // prescaler 8
    { 4194304,    6,  0b011 }, // prescaler 64
    { 16777216,   8,  0b100 }, // prescaler 256
    { UINT32_MAX, 10, 0b101 }  // prescaler 1024
  };

  // Timer 2 (8-bit: max 256 ticks and 7 prescalers)
  constexpr PrescalerEntry TIMER2_LUT[] = {
    { 256,        0,  0b001 }, // prescaler 1
    { 2048,       3,  0b010 }, // prescaler 8
    { 8192,       5,  0b011 }, // prescaler 32
    { 16384,      6,  0b100 }, // prescaler 64
    { 32768,      7,  0b101 }, // prescaler 128
    { 65536,      8,  0b110 }, // prescaler 256
    { UINT32_MAX, 10, 0b111 }  // prescaler 1024
  };

  Config lookup_config(uint8_t timer_id, uint32_t period_us) {
    const PrescalerEntry* lut;
    if (timer_id == 0) {
      lut = TIMER0_LUT;
    } else if (timer_id == 1) {
      lut = TIMER1_LUT;
    } else {
      lut = TIMER2_LUT;
    }

    const uint32_t period_ticks = period_us * 16;

    for (uint8_t i = 0; ; ++i) {
      if (period_ticks < lut[i].max_ticks) {
        const uint8_t shift = lut[i].scaler_shift;
        const uint16_t lost_bits = (period_ticks & (((uint32_t)1 << shift) - 1)) / 16;

        Config c;
        c.ticks = static_cast<uint16_t>((period_ticks >> shift) - 1);
        c.scaler = lut[i].code;
        c.rounding_error_us = lost_bits;
        return c;
      }
    }
  }

} // anonymous namespace

//------------------------------------------------------------------------------
// Entry points used by the header templates (the library's only external symbols)
//------------------------------------------------------------------------------

namespace timer_detail {

  void ctcm_apply(uint8_t timer_id, uint32_t period_us, bool channel, bool interrupt) {
    if (timer_id != 1) {
      config_mode(timer_id, 2);  // CTC mode for 8-bit timers
    } else {
      config_mode(timer_id, 12); // CTC mode for 16-bit timer
    }

    // Output toggles each compare match, so the compare period is half the output period
    Config config = lookup_config(timer_id, period_us);

    config_top(timer_id, config.ticks);
    config_compare(timer_id, config.ticks, 200); // compare = TOP

    if (channel) {
      config_channel(timer_id, 0b01);
    } else {
      config_channel(timer_id, 0b00);
    }

    config_scaler(timer_id, config.scaler);
  }

  void fpwm_apply(uint8_t timer_id, uint32_t period_us, bool channel, bool interrupt, uint8_t duty_cycle) {
    if (timer_id != 1) {
      config_mode(timer_id, 7);  // Fast PWM mode for 8-bit timers
    } else {
      config_mode(timer_id, 14); // Fast PWM mode for 16-bit timer
    }

    Config config = lookup_config(timer_id, period_us);

    config_top(timer_id, config.ticks);
    config_compare(timer_id, config.ticks, duty_cycle);

    if (channel) {
      config_channel(timer_id, 0b10);
    } else {
      config_channel(timer_id, 0b00);
    }

    config_interrupt(timer_id, interrupt); // Disable compare match interrupt

    config_scaler(timer_id, config.scaler);
  }

} // namespace timer_detail

void fpwm_update(uint8_t timer_id, uint32_t period_us, uint8_t duty_cycle) {
  Config config = lookup_config(timer_id, period_us);
  config_compare(timer_id, config.ticks, duty_cycle); // Update compare value for PWM duty cycle
}