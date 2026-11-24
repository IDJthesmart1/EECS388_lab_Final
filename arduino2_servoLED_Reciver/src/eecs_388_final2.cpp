/******************************************************************************
 *   Header Includes
 *******************************************************************************/

extern "C" {
#include <stdint.h>
}

#include <Arduino.h>
#include "eecs_388_lib.hpp"
#include "minimal_timer_lib.hpp"

//------------------------------------------------------------------------------
// Constant definitions
//------------------------------------------------------------------------------

/*ServoMotor constants*/
#define SERVO_PULSE_MAX (2400)    /* 2400 us (2.4ms) */
#define SERVO_PULSE_MIN (544)     /* 544 us (0.544ms) */
#define SERVO_PERIOD    (20000)   /* 20000 us (20ms) */
#define MIN_ANGLE       (0)       /* degrees */
#define MAX_ANGLE       (180)     /* degrees */
// Speed assumes no load and slowest case of 4.8V
#define SERVO_SPEED     (2000)    /* 2000 us/deg (0.12 seconds per 60 degrees) */

// Simulated code for the angles from the lab sheet
const int ANGLE_VALUES[] = { 10, 25, 75, 45, 100, 40, 125, 15, 150, 50, 170 };
//------------------------------------------------------------------------------
// Application specific definitions
//------------------------------------------------------------------------------
struct LED {
  uint8_t pin;
  uint8_t state; // 0 = off, 1 = on, 2 = flashing
};

LED red = { GPIO_11, false };
LED green = { GPIO_12, false };

struct Servo {
  uint8_t pin;
  int angle_state;
};

Servo steering = { GPIO_9, MIN_ANGLE };

//-------------------------------------------------------------------------------

void turnOn(LED& led) {
  gpio_write(led.pin, ON);
  led.state = 1;
}

void turnOff(LED& led) {
  gpio_write(led.pin, OFF);
  led.state = 0;
}

void toggle(LED& led) {
  if (led.state == 0) {
    turnOn(led);
  } else {
    turnOff(led);
  }
}

void flash(LED& led) {
  ctcm_init<2, 14286, true, false>(); // 100ms period divided into 7 (16ms max), GPIO_11
  led.state = 2;
}

const unsigned long pulse_range = SERVO_PULSE_MAX - SERVO_PULSE_MIN;
const unsigned long angle_range = MAX_ANGLE - MIN_ANGLE;


/******************************************************************************
 *   Function: setup() - Initializes the Arduino System
 *      Pre condition:
 *          Hardware must be properly connected (BMP180 sensors, etc.)
 *      Post condition:
 *          Runs initialization calls one time on power up
 *          UART is initialized for ser_printf()
 *          I2C is initialized for communication with BMP180
 *          calib_data is filled with calibration data from BMP180
 *******************************************************************************/
void setup() {
  uart_init();     // Initialize UART for serial communication

  sei();
  fpwm_init<1, 20000, true, true, 0>();

  //Setup Auto-break LEDs for Distance
  gpio_mode(GPIO_12, GPIO_OUTPUT); //GREEN
  gpio_mode(GPIO_11, GPIO_OUTPUT); //RED
}

int8_t acceleration = 0; // global variable to track acceleration, measured in 0.2 mph per 1/50 of a second (1 mph per second)
// 1 = accelerating / no breaking, -1 = brake lightly, -2 = brake hard, -3 = must stop
/******************************************************************************
 *   Function: ISR(USART_RX_vect) - UART Receive Data Interrupt Service Routine
 *      Pre condition:
 *          uart_setup() has been executed and system is initialized
 *      Post condition:
 *          Reads the UART data register if and only if data has been received,
 *          and sets the LED states based on the received distance value
 *******************************************************************************/
ISR(USART_RX_vect) {
  // distance is now a value between 0 and 255 where each unit represents 2 cm
  // Valid range of 30 - 540 cm (technically, anything above 200 cm is considered "safe to proceed")
  uint8_t distance_received = ser_read();
  if (distance_received > 200) {
    turnOn(green);
    turnOff(red);
    acceleration = 1;
  } else if (distance_received > 100) {
    turnOn(red);
    turnOn(green);
    acceleration = -1;
  } else if (distance_received > 60) {
    turnOn(red);
    turnOff(green);
    acceleration = -2;
  } else {
    flash(red);
    turnOff(green);
    acceleration = -3;
  }

}

#define TOP_SPEED 500 // 100 mph in 0.2 mph increments
uint32_t speed = 0; // global variable to track speed, measured in 0.2mph increments

#define NUM_ANGLES 11
volatile uint32_t current_angle_f = ANGLE_VALUES[0];
volatile uint32_t angle_index = 0;

// Acceleration: no breaking: 0 to 60 in 6 seconds, brake lightly: 60 to 0 in 6 seconds
//               brake hard: 60 to 0 in 3 seconds, must stop: 60 to 0 in 3 seconds
/******************************************************************************
 *   Function: ISR(TIMER1_CAPT_vect) - Servo PWM Interrupt Service Routine
 *      Description:
 *          This function is called every 20ms exactly (50 Hz),
 *          and updates the servo's duty cycle to the new calculated angle value
 *          The changes take effect exactly one period delayed
 *      Pre condition:
 *          timer 1 is initialized
 *      Post condition:
 *          Updates the speed variable based on the current acceleration value
 *******************************************************************************/
ISR(TIMER1_CAPT_vect) {
  // Only possible values for acceleration are 1, -1, -2, and -3
  // Speed is anything from 0 to 500
  if (acceleration == 1 && speed < TOP_SPEED) {
    speed++; // Prevent speed from overflowing
  } else if (speed > 3) {
    speed = static_cast<uint32_t>(speed + acceleration); // Update speed based on the acceleration
  } else if (speed <= 3) {
    // snap to zero if speed is less than or equal to 3 (0.6 mph)
    speed = 0; // Prevent speed from underflowing
  }

  // no speed do nothing.
  if (speed == 0) {
    return;
  }

  // 10 deg/tick at speed=1, down to ~1 deg/tick at speed=500
  float step_deg = 10.0f - ((float)speed / (float)TOP_SPEED) * 9.0f;
  if (step_deg < 1.0f) step_deg = 1.0f;

  float target = (float) ANGLE_VALUES[angle_index];
  float diff = target - current_angle_f;

  if (diff > step_deg) {
    current_angle_f += step_deg;
  } else if (diff < -step_deg) {
    current_angle_f -= step_deg;
  } else {
    current_angle_f = target;
    angle_index = (angle_index + 1) % NUM_ANGLES;
  }
  
  uint32_t deg = (uint32_t) current_angle_f;
  if (deg > MAX_ANGLE) deg = MAX_ANGLE;

  uint32_t period = SERVO_PULSE_MIN + (uint32_t)(((uint64_t)deg * (SERVO_PULSE_MAX - SERVO_PULSE_MIN)) / MAX_ANGLE);
  fpwm_update(1, 20, (period*1000)/20);
}

/******************************************************************************
 *   Function: loop() - Main execution loop
 *      Pre condition:
 *          setup() has been executed and system is initialized
 *      Post condition:
 *          Performs a single iteration of the system's function
 *          Repeates indefinetely unless the board is reset or powered off
 *******************************************************************************/
void loop() {
  /*
  / 15.9.2 Clear Timer on Compare Match Mode, and 15.9.3 Fast PWM Mode are available
  /
  / Plan:
  /
  / UART communication: LiDAR reading will be triggered by receive complete interrupt,
  / from the LiDAR sensor to Arduino1, and then from Arduino1 to Arduino2.
  /
  / Break control: The LED state will be set in the same ISR that reads the data line,
  / so the state changes as soon as possible and the data never has to be stored in memory.
  / A CTCM Timer will control the red pin directly when flashing,
  / but loses control in any other state.
  / The rest of the time changes will be standard GPIO writes.
  /
  / Servo control: CTC Timer with 20ms period will control the pin directly,
  / an ISR will only trigger at the end of the period,
  / and update the compare match register with the new duty cycle every 20ms.
  /
  / Speed simulation: A global int will track "speed."
  / We need to determine reasonable acceleration and deceleration rates to hardcode.
  / Once that is done a CTCM timer can run a ISR at consistant intervals to update the speed variable,
  / according to the amount of breaking being applied. The speed will also scale the steering speed,
  / so that the servo will not turn as fast at high speeds.
  /
  /
  / LED flashing timer: Mode: CTC, Period: 100ms, Duty cycle: 50%
  / Servo PWM timer: Mode: Fast PWM, Period: 20ms, Duty cycle: set to the calculated angle, ISR: Update the duty cycle every iteration.
  /
  / Speed will scale the servo speed so that the steering will be faster at low speeds and slower at high speeds.
  / The servo angle represents the absolute angle of the car, and it takes longer to turn at high speeds. - Approved by TA
  */

}