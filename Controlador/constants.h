/**
 * constants.h
 * spaCEinvaders - Raspberry Pi Pico Controller
 * Constantes globales del control fisico.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define BTN_MOVE_PIN 16
#define BTN_FIRE_PIN 13

#define DOUBLE_TAP_WINDOW_MS 180
#define MOVE_HOLD_THRESHOLD_MS 180
#define MOVE_REPEAT_MS 90
#define DEBOUNCE_MS 50
#define USB_WAIT_ATTEMPTS 200
#define USB_WAIT_STEP_MS 10
#define USB_STARTUP_STABILIZE_MS 200
#define MAIN_LOOP_SLEEP_MS 10

#define CMD_LEFT 'L'
#define CMD_RIGHT 'R'
#define CMD_FIRE 'F'

#define LED_PIN 25
#define LED_BLINK_MS 80

#endif /* CONSTANTS_H */