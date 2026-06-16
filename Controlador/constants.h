/**
 * constants.h
 * spaCEinvaders - Raspberry Pi Pico Controller
<<<<<<< Updated upstream
 * Constantes globales del control físico.
=======
 * Constantes globales del control fisico.
>>>>>>> Stashed changes
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

<<<<<<< Updated upstream
/* ── Pines GPIO ─────────────────────────────────────────────────── */
#define BTN_MOVE_PIN     16   /* GP16 – botón de movimiento (1 toque=izq, 2=der) */
#define BTN_FIRE_PIN     13   /* GP13 – botón de disparo                          */

/* ── UART ───────────────────────────────────────────────────────── */
#define UART_ID          uart0
#define UART_TX_PIN      0    /* GP0 – TX (conectar al RX del PC/cliente)         */
#define UART_RX_PIN      1    /* GP1 – RX (conectar al TX del PC/cliente)         */
#define UART_BAUD_RATE   115200

/* ── Detección de doble toque ───────────────────────────────────── */
/*  Ventana en ms para detectar un segundo toque en el botón de movimiento */
#define DOUBLE_TAP_WINDOW_MS   400

/* ── Debounce ───────────────────────────────────────────────────── */
#define DEBOUNCE_MS      50

/* ── Protocolo de mensajes (1 byte por comando) ─────────────────── */
/*
 * El cliente C en la PC recibe estos bytes por UART y los traduce
 * a mensajes de texto para el servidor Java por socket TCP.
 *
 *  Byte   Significado
 *  ----   -----------
 *  'L'    Mover cañón a la IZQUIERDA
 *  'R'    Mover cañón a la DERECHA
 *  'F'    DISPARO (Fire)
 *
 * Se eligió un protocolo de un solo byte ASCII para:
 *   - Máxima simplicidad de parsing en el cliente C.
 *   - Legibilidad durante depuración con cualquier monitor serie.
 *   - Mínima latencia (no hay cabecera ni checksum necesarios a
 *     esta velocidad de juego).
 */
=======
/* Pines GPIO */
#define BTN_MOVE_PIN     16   /* GP16 - boton movimiento (1 toque=izq, 2=der) */
#define BTN_FIRE_PIN     13   /* GP13 - boton disparo                          */

/* UART por USB (CDC) - 115200 8N1, un byte ASCII por comando */
#define UART_BAUD_RATE   115200

/* Movimiento continuo: retencion breve para distinguir doble toque y repeticion */
#define DOUBLE_TAP_WINDOW_MS   180
#define MOVE_HOLD_THRESHOLD_MS 180
#define MOVE_REPEAT_MS          90

/* Debounce (ms) */
#define DEBOUNCE_MS      50

/* Protocolo: 1 byte por comando */
>>>>>>> Stashed changes
#define CMD_LEFT         'L'
#define CMD_RIGHT        'R'
#define CMD_FIRE         'F'

<<<<<<< Updated upstream
/* ── Indicador LED onboard ──────────────────────────────────────── */
#define LED_PIN          25   /* LED interno del Pico (confirmación visual)       */
#define LED_BLINK_MS     80   /* Duración del parpadeo al enviar un comando       */
=======
/* LED onboard para confirmacion visual */
#define LED_PIN          25
#define LED_BLINK_MS     80
>>>>>>> Stashed changes

#endif /* CONSTANTS_H */
