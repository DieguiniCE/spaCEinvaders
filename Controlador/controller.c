/**
 * controller.c
 * spaCEinvaders – Control físico (Raspberry Pi Pico 2040)
 *
 * Comportamiento:
 *   Botón MOVE (GP16):
 *     · 1 toque  →  envía 'L' por UART  (mover izquierda)
 *     · 2 toques →  envía 'R' por UART  (mover derecha)
 *   Botón FIRE (GP13):
 *     · 1 toque  →  envía 'F' por UART  (disparar)
 *
 * Protocolo UART: 115200 8N1, un byte ASCII por comando.
 * El cliente C en la PC lee estos bytes y los reenvía al
 * servidor Java vía socket TCP.
 *
 * Compilar con el Pico SDK:
 *   mkdir build && cd build
 *   cmake .. -DPICO_SDK_PATH=/ruta/al/pico-sdk
 *   make
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "constants.h"

/* ── Prototipos ────────────────────────────────────────────────── */
static void uart_setup(void);
static void gpio_setup(void);
static void send_command(char cmd);
static void led_blink(void);
static void handle_move_button(void);
static void handle_fire_button(void);

/* ── Estado interno del botón de movimiento ─────────────────────
 * Usamos una máquina de estados sencilla para detectar doble toque
 * sin bloquear el loop principal.
 * ─────────────────────────────────────────────────────────────── */
typedef enum {
    MOVE_IDLE,          /* Esperando primer toque           */
    MOVE_FIRST_TAP,     /* Primer toque detectado, esperando */
    MOVE_WAIT_RELEASE   /* Esperando que suelte el botón     */
} MoveState;

static MoveState  move_state        = MOVE_IDLE;
static uint32_t   first_tap_time_ms = 0;   /* Timestamp del primer toque */
static bool       move_btn_prev     = false;
static bool       fire_btn_prev     = false;

/* ═══════════════════════════════════════════════════════════════ */
int main(void) {
    stdio_init_all();   /* Inicializa USB-serial (útil para debug) */
    uart_setup();
    gpio_setup();

    /* Pequeño delay para estabilizar los pines al arrancar */
    sleep_ms(200);

    while (true) {
        handle_move_button();
        handle_fire_button();
        sleep_ms(10);   /* Ciclo de 10 ms – más que suficiente para HID */
    }

    return 0;   /* Nunca llega aquí */
}

/* ── Inicialización UART ──────────────────────────────────────── */
static void uart_setup(void) {
    uart_init(UART_ID, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    /* Sin flow-control, sin paridad: 8N1 por defecto en el SDK */
}

/* ── Inicialización GPIO ─────────────────────────────────────── */
static void gpio_setup(void) {
    /* Botones con pull-up interno: reposo = HIGH, toque = LOW */
    gpio_init(BTN_MOVE_PIN);
    gpio_set_dir(BTN_MOVE_PIN, GPIO_IN);
    gpio_pull_up(BTN_MOVE_PIN);

    gpio_init(BTN_FIRE_PIN);
    gpio_set_dir(BTN_FIRE_PIN, GPIO_IN);
    gpio_pull_up(BTN_FIRE_PIN);

    /* LED onboard para feedback visual */
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

/* ── Enviar un byte de comando por UART (Virtual USB) ───────── */
static void send_command(char cmd) {
    putchar(cmd);                           /* Enviar por USB CDC (Virtual UART) */
    fflush(stdout);                         /* Asegurar que se envíe inmediatamente */
    led_blink();
                           /* Feedback visual inmediato */
}

/* ── Parpadeo breve del LED onboard ─────────────────────────── */
static void led_blink(void) {
    gpio_put(LED_PIN, 1);
    sleep_ms(LED_BLINK_MS);
    gpio_put(LED_PIN, 0);
}

/* ── Manejo del botón de movimiento (doble toque) ───────────── *
 *
 *  Máquina de estados:
 *
 *   IDLE ──[flanco bajada]──▶ FIRST_TAP  (guarda timestamp)
 *   FIRST_TAP ──[flanco bajada antes de DOUBLE_TAP_WINDOW]──▶ IDLE  (envía 'R')
 *   FIRST_TAP ──[timeout]──────────────────────────────────▶ IDLE  (envía 'L')
 *
 * ──────────────────────────────────────────────────────────── */
static void handle_move_button(void) {
    bool pressed = !gpio_get(BTN_MOVE_PIN);   /* LOW = presionado (pull-up) */
    bool rising  = pressed && !move_btn_prev; /* Flanco de bajada (botón pulsado) */
    move_btn_prev = pressed;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (move_state) {

        case MOVE_IDLE:
            if (rising) {
                move_state        = MOVE_FIRST_TAP;
                first_tap_time_ms = now;
            }
            break;

        case MOVE_FIRST_TAP:
            if (rising) {
                /* Segundo toque dentro de la ventana → derecha */
                send_command(CMD_RIGHT);
                move_state = MOVE_WAIT_RELEASE;
                printf("Segundo toque detectado dentro de la ventana, enviando 'R'\n");
            } else if ((now - first_tap_time_ms) >= DOUBLE_TAP_WINDOW_MS) {
                /* Timeout sin segundo toque → izquierda */
                send_command(CMD_LEFT);
                move_state = MOVE_IDLE;
                printf("Timeout: primer toque no seguido de segundo toque, enviando 'L'\n");
            }
            break;

        case MOVE_WAIT_RELEASE:
            /* Esperamos a que el usuario suelte el botón para no
             * disparar un primer toque fantasma al soltar */
            if (!pressed) {
                move_state = MOVE_IDLE;
            }
            break;
    }
}

/* ── Manejo del botón de disparo (con debounce simple) ──────── */
static void handle_fire_button(void) {
    static uint32_t last_fire_ms = 0;

    bool pressed = !gpio_get(BTN_FIRE_PIN);
    bool rising  = pressed && !fire_btn_prev;
    fire_btn_prev = pressed;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (rising && (now - last_fire_ms) >= DEBOUNCE_MS) {
        send_command(CMD_FIRE);
        last_fire_ms = now;
        printf("Botón de disparo presionado\n");
    }
}
