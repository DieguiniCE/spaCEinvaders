#ifndef SERIAL_H
#define SERIAL_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct {
    HANDLE handle;
    int conectado;
} PuertoSerial;

int serial_abrir(PuertoSerial* puerto, const char* nombrePuerto, DWORD baudios);
void serial_cerrar(PuertoSerial* puerto);
int serial_leer_byte(PuertoSerial* puerto, char* byte, DWORD timeoutMs);

#endif /* SERIAL_H */
