/**
 * controller.c
 * spaCEinvaders - Control fisico (Raspberry Pi Pico 2040)
 *
 * Boton MOVE (GP16): 1 toque = 'L', 2 toques = 'R'
 * Mantener pulsado repite el movimiento.
 * Boton FIRE (GP13): 1 toque = 'F'
 *
 * Comunicacion por USB CDC a 115200 8N1.
 * El cliente C en la PC lee el puerto COM y reenvia al servidor Java.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "constants.h"

typedef enum {
    MOVE_IDLE,
    MOVE_FIRST_PRESS,
    MOVE_WAIT_SECOND_TAP,
    MOVE_LEFT_REPEAT,
    MOVE_RIGHT_REPEAT
} MoveState;

static MoveState move_state = MOVE_IDLE;
static uint32_t first_press_time_ms = 0;
static uint32_t release_time_ms = 0;
static uint32_t last_repeat_time_ms = 0;
static bool move_btn_prev = false;
static bool fire_btn_prev = false;

static void gpio_setup(void);
static void send_command(char cmd);
static void led_blink(void);
static void handle_move_button(void);
static void handle_fire_button(void);

int main(void) {
    stdio_init_all();
    gpio_setup();

    for (int i = 0; i < 200 && !stdio_usb_connected(); i++) {
        sleep_ms(10);
    }

    sleep_ms(200);

    while (true) {
        handle_move_button();
        handle_fire_button();
        sleep_ms(10);
    }

    return 0;
}

static void gpio_setup(void) {
    gpio_init(BTN_MOVE_PIN);
    gpio_set_dir(BTN_MOVE_PIN, GPIO_IN);
    gpio_pull_up(BTN_MOVE_PIN);

    gpio_init(BTN_FIRE_PIN);
    gpio_set_dir(BTN_FIRE_PIN, GPIO_IN);
    gpio_pull_up(BTN_FIRE_PIN);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

static void send_command(char cmd) {
    printf("%c", cmd);
    fflush(stdout);
    led_blink();
}

static void led_blink(void) {
    gpio_put(LED_PIN, 1);
    sleep_ms(LED_BLINK_MS);
    gpio_put(LED_PIN, 0);
}

static void handle_move_button(void) {
    bool pressed = !gpio_get(BTN_MOVE_PIN);
    bool rising = pressed && !move_btn_prev;
    bool falling = !pressed && move_btn_prev;
    move_btn_prev = pressed;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (move_state) {
        case MOVE_IDLE:
            if (rising) {
                move_state = MOVE_FIRST_PRESS;
                first_press_time_ms = now;
            }
            break;

        case MOVE_FIRST_PRESS:
            if (pressed && (now - first_press_time_ms) >= MOVE_HOLD_THRESHOLD_MS) {
                send_command(CMD_LEFT);
                last_repeat_time_ms = now;
                move_state = MOVE_LEFT_REPEAT;
            } else if (falling) {
                release_time_ms = now;
                move_state = MOVE_WAIT_SECOND_TAP;
            }
            break;

        case MOVE_WAIT_SECOND_TAP:
            if (rising && (now - release_time_ms) <= DOUBLE_TAP_WINDOW_MS) {
                send_command(CMD_RIGHT);
                last_repeat_time_ms = now;
                move_state = MOVE_RIGHT_REPEAT;
            } else if (pressed && (now - release_time_ms) > DOUBLE_TAP_WINDOW_MS) {
                send_command(CMD_LEFT);
                last_repeat_time_ms = now;
                move_state = MOVE_LEFT_REPEAT;
            } else if (!pressed && (now - release_time_ms) > DOUBLE_TAP_WINDOW_MS) {
                send_command(CMD_LEFT);
                move_state = MOVE_IDLE;
            }
            break;

        case MOVE_LEFT_REPEAT:
            if (pressed) {
                if ((now - last_repeat_time_ms) >= MOVE_REPEAT_MS) {
                    send_command(CMD_LEFT);
                    last_repeat_time_ms = now;
                }
            } else {
                move_state = MOVE_IDLE;
            }
            break;

        case MOVE_RIGHT_REPEAT:
            if (pressed) {
                if ((now - last_repeat_time_ms) >= MOVE_REPEAT_MS) {
                    send_command(CMD_RIGHT);
                    last_repeat_time_ms = now;
                }
            } else {
                move_state = MOVE_IDLE;
            }
            break;
    }
}

static void handle_fire_button(void) {
    static uint32_t last_fire_ms = 0;

    bool pressed = !gpio_get(BTN_FIRE_PIN);
    bool rising = pressed && !fire_btn_prev;
    fire_btn_prev = pressed;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (rising && (now - last_fire_ms) >= DEBOUNCE_MS) {
        send_command(CMD_FIRE);
        last_fire_ms = now;
    }
}