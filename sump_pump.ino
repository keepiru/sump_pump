/*  Simple pump controller
 *
 *  This is an Arduino sketch which senses water level and controls a pump.
 *  The purpose is to empty the condensate out of the bottom of my air
 *  conditioner before it spills on the floor.
 *
 *  Water is sensed by leaving a ground wire and a sense wire in the bottom of
 *  the condensate pan.  When they get covered with water the pump turns on to
 *  empty it out.  My pump (a $10 miniature peristaltic pump from Amazon) is
 *  safe to run dry, so this program keeps it running for a few minutes to
 *  finish slurping the last of the water out of the pan and purge the lines
 *  with air.
 *
 *  I'm using an Adafruit motor shield v1.2 to power the pump.  When using 5v
 *  power, the H-bridge drops too much voltage if I connect to both sides of
 *  output.  Instead, I connect the pump to one side of the output and ground.
 *  I also have to feed power directly into the 5v rail because the USB port
 *  drops too much juice.
 *
 *  Alternatively you could use a separate slightly-higher-voltage supply for
 *  the motor shield, or replace the whole shield with discrete transistor and
 *  flyback diode, or use a v2 motor shield which has better H-bridges with
 *  less voltage drop.
 *
 *  Copyright (C) 2016  Chris "Kai" Frederick
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <AFMotor.h> // Adafruit library for the original (v1) motor shield.

/* This pin is connected to a wire dangling over the water next to a ground
 * wire.  We will apply a pullup so it's at 5V when in air; when wet it will get
 * pulled down toward ground. */
#define WATER_SENSE_PIN A0

/* 800 ADC counts is about 3.9 volts.  If the internal pullup is 40k ohm, this
 * means we will trigger when the resistance falls below 142k ohm.  800 is
 * chosen to reject most noise while still triggering reliably even in fairly
 * pure (highly insulating) water.  Adjust it higher to detect higher purity
 * water or greater distance between the leads; adjust it lower to reduce the
 * chance of accidental triggers. */
#define WET_THRESHOLD 800

/* 13 is the default arduino on-board LED pin.  We'll turn it on whenever water
 * is detected. */
#define TRIGGERED_LED_PIN 13

/* How long to keep running the pump after the leads no longer detect water.
 * 180,000 milliseconds == 3 minutes  */
#define RUN_DRY_DURATION 180000   // milliseconds

/* 0..255 .   255 = full speed. */
#define PUMP_SPEED 255

/* How long to let things settle when the pump turns off.  Back EMF might
 * introduce a little noise and create a false trigger if the WET_TRESHOLD is
 * set very sensitive.  */
#define PUMP_SETTLE_DURATION 5000  // milliseconds
AF_DCMotor pump(4);                                        // output on the Adafruit motor shield board

void setup() {
  pinMode(WATER_SENSE_PIN, INPUT_PULLUP);                  // AVR pulls up the probe; water will pull it back down.
  pinMode(TRIGGERED_LED_PIN, OUTPUT);                      // LED is an output.
}

void loop() {
  static uint32_t last_trigger_time = 0;                   // milliseconds, time water was last detected
  uint16_t measurement;                                    // 0..1023 raw ADC value
  static bool pump_running = false;                        // true iff pump is currently running

  measurement = analogRead(WATER_SENSE_PIN);
  if (measurement < WET_THRESHOLD) {                       // Water pulls the measurement DOWN
    last_trigger_time = millis();
    digitalWrite(TRIGGERED_LED_PIN, 1);                    // Turn on LED
    pump.setSpeed(PUMP_SPEED);                             // And the pump
    pump.run(FORWARD);
    pump_running = true;
  } else {
    digitalWrite(TRIGGERED_LED_PIN, 0);                    // Turn LED off; the pump keeps running.
  }

  /*  Note that millis() is a uint32_t .  It will overflow every 49.7 days, so
   *  we need to be careful to hanlde this in our math as well.  If we trigger 4
   *  ms before the overflow, then:
   *    last_trigger_time == 2**32 - 5 == 4294967291
   *  4 ms later:
   *    millis() == 4294967295
   *    millis() - last_trigger_time == 4
   *  2 ms after that:
   *    millis() == 1
   *    millis() - last_trigger_time == 6
   *  ... because we're using uint32_t math which also overflows.  */
  if (pump_running && millis() - last_trigger_time > RUN_DRY_DURATION) {
    pump.run(RELEASE);
    pump_running = false;
    delay(PUMP_SETTLE_DURATION);
  }
}
