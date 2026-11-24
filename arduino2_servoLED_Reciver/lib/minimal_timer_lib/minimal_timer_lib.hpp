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
  void ctcm_apply(uint8_t timer_id, uint32_t period_us, bool pin, bool interrupt);
  void fpwm_apply(uint8_t timer_id, uint32_t period_us, bool pin, bool interrupt, uint8_t duty_cycle);
}

//------------------------------------------------------------------------------
// Public functions
//------------------------------------------------------------------------------

/******************************************************************************
 *   Function: ctcm_init<uint8_t timer_id, uint32_t period_us, bool pin, bool interrupt>()
 *      Arguments:
 *          timer_id - Timer ID to use (0, 1, or 2)
 *          period_us - Period in microseconds (us)
 *          pin - Enable the GPIO pin to use for output
 *            channel B (GPIO_3, GPIO_5, or GPIO_10) is used for timers 0, 2, and 1 respectively
 *            pin will toggle each period, so effective waveform is 50% duty cycle over two periods
 *          interrupt - Enable interrupt on compare match; happens at the same time as GPIO pin toggle (twice per period)
 *******************************************************************************/
template <uint8_t timer_id, uint32_t period_us, bool pin, bool interrupt>
inline void ctcm_init() {
  // Compile-time checks:
  static_assert(timer_id <= 2, "Error: Timer ID must be 0, 1, or 2.");
  static_assert(!(timer_id != 1 && period_us > 16384), "Error: 8-bit Timers (0 and 2) cannot exceed 16,384 us.");
  static_assert(period_us >= 1, "Error: Period must be at least 1 us.");
  static_assert(period_us < 4194304, "Error: 16-bit Timer (1) cannot exceed 4,194,304 us.");

  timer_detail::ctcm_apply(timer_id, period_us, pin, interrupt);
}

/******************************************************************************
 *   Function: fpwm_init<uint8_t timer_id, uint32_t period_us, bool pin, bool interrupt, uint8_t duty_cycle>()
 *      Arguments:
 *          timer_id - Timer ID to use (0, 1, or 2)
 *          period_us - Period in microseconds (us)
 *          pin - Enable the GPIO pin to use for output
 *            channel B (GPIO_3, GPIO_5, or GPIO_10) is used for timers 0, 2, and 1 respectively
 *            pin will toggle each period, so effective waveform is 50% duty cycle over two periods
 *          interrupt - Enable interrupt on TOP; happens at the same time as timer resets to 0 (once per period)
 *          duty_cycle - The duty cycle of the PWM signal, half percent precision (0-200 = 0%-100%)
 *******************************************************************************/
template <uint8_t timer_id, uint32_t period_us, bool pin, bool interrupt, uint8_t duty_cycle>
inline void fpwm_init() {
  // Compile-time checks:
  static_assert(timer_id <= 2, "Error: Timer ID must be 0, 1, or 2.");
  static_assert(!(timer_id != 1 && period_us > 16384), "Error: 8-bit Timers (0 and 2) cannot exceed 16,384 us.");
  static_assert(period_us >= 1, "Error: Period must be at least 1 us.");
  static_assert(period_us < 4194304, "Error: 16-bit Timer (1) cannot exceed 4,194,304 us.");
  static_assert(duty_cycle <= 200, "Error: Duty cycle must be between 0 and 200.");

  timer_detail::fpwm_apply(timer_id, period_us, pin, interrupt, duty_cycle);
}

void fpwm_update(uint8_t timer_id, uint32_t period_us, uint8_t duty_cycle);