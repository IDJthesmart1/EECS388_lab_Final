#pragma once

extern "C" {
#include <stdint.h>
}

/*
/ Implemented mode combinations:
/ timer_id: mode(s): period_register
/ 0: 2(CTC), 7(FPWM): OCR0A
/ 1: 12(CTC), 14(FPWM): ICR1
/ 2: 2(CTC), 7(FPWM): OCR2A
*/

//------------------------------------------------------------------------------
// Internal linkage points (NOT part of the public API)
//------------------------------------------------------------------------------
// Templates must be fully visible to every translation unit that instantiates
// them, so they live in this header. These two functions are the ONLY symbols
// the templates need from the .cpp file. Every register helper, lookup table,
// and struct stays private to minimal_timer_lib.cpp (anonymous namespace).
namespace timer_detail {
  void ctcm_apply(uint8_t timer_id, uint32_t period_us, uint8_t channel);
  void fpwm_apply(uint8_t timer_id, uint32_t period_us, uint8_t channel);
}

//------------------------------------------------------------------------------
// Public functions
//------------------------------------------------------------------------------

// Pass TimerID, PeriodUS, and Channel inside < > instead of ( )
template <uint8_t timer_id, uint32_t period_us, uint8_t channel>
inline void ctcm_init() {
  // Compile-time checks:
  static_assert(timer_id <= 2, "Error: Timer ID must be 0, 1, or 2.");
  static_assert(!(timer_id != 1 && period_us > 32768), "Error: 8-bit Timers (0 and 2) cannot exceed 32,768 us.");
  static_assert(channel <= 3, "Error: Channel must be none, A, B, or both (0, 1, 2, 3).");
  static_assert(period_us >= 2, "Error: CTC period must be at least 2 us (half-period must be nonzero).");
  static_assert(period_us < 8388608, "Error: 16-bit Timer (1) cannot exceed 8,388,608 us.");

  timer_detail::ctcm_apply(timer_id, period_us, channel);
}

// Pass TimerID, PeriodUS, and Channel inside < > instead of ( )
template <uint8_t timer_id, uint32_t period_us, uint8_t channel>
inline void fpwm_init() {
  // Compile-time checks:
  static_assert(timer_id <= 2, "Error: Timer ID must be 0, 1, or 2.");
  static_assert(!(timer_id != 1 && period_us > 16384), "Error: 8-bit Timers (0 and 2) cannot exceed 16,384 us.");
  static_assert(channel <= 3, "Error: Channel must be none, A, B, or both (0, 1, 2, 3).");
  static_assert(period_us >= 1, "Error: Period must be at least 1 us.");
  static_assert(period_us < 4194304, "Error: 16-bit Timer (1) cannot exceed 4,194,304 us.");
  static_assert(!(timer_id != 1 && (channel == 1 || channel == 3)), "Error: Only Timer 1 supports channel A PWM.");

  timer_detail::fpwm_apply(timer_id, period_us, channel);
}
