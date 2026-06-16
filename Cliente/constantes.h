#ifndef CONSTANTES_H
#define CONSTANTES_H

/* Red */
#define PUERTO_SERVIDOR 5000
#define IP_SERVIDOR "127.0.0.1"

/* Pantalla */
#define ANCHO_PANTALLA 800
#define ALTO_PANTALLA 600

/* Logica del juego */
#define VIDAS_INICIALES 3
#define PTS_CALAMAR 10
#define PTS_CANGREJO 20
#define PTS_PULPO 40
#define VELOCIDAD_CANON 5
#define ANCHO_CANON 40
#define ALTO_CANON 20

/* Controlador fisico (Pico via UART USB / CDC serial) */
#define UART_BAUD_RATE 115200
#define PUERTO_SERIAL "COM4"

/* Protocolo de comandos (1 byte ASCII, igual que Controlador/constants.h) */
#define CMD_LEFT 'L'
#define CMD_RIGHT 'R'
#define CMD_FIRE 'F'

#endif /* CONSTANTES_H */