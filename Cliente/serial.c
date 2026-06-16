#include "serial.h"
#include <stdio.h>
#include <string.h>

int serial_abrir(PuertoSerial* puerto, const char* nombrePuerto, DWORD baudios) {
    char rutaPuerto[32];

    puerto->handle = INVALID_HANDLE_VALUE;
    puerto->conectado = 0;

    if (strncmp(nombrePuerto, "\\\\.\\", 4) == 0) {
        snprintf(rutaPuerto, sizeof(rutaPuerto), "%s", nombrePuerto);
    } else {
        snprintf(rutaPuerto, sizeof(rutaPuerto), "\\\\.\\%s", nombrePuerto);
    }

    puerto->handle = CreateFileA(
        rutaPuerto,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (puerto->handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    DCB config = {0};
    config.DCBlength = sizeof(DCB);
    if (!GetCommState(puerto->handle, &config)) {
        CloseHandle(puerto->handle);
        puerto->handle = INVALID_HANDLE_VALUE;
        return 0;
    }

    config.BaudRate = baudios;
    config.ByteSize = 8;
    config.Parity = NOPARITY;
    config.StopBits = ONESTOPBIT;
    config.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(puerto->handle, &config)) {
        CloseHandle(puerto->handle);
        puerto->handle = INVALID_HANDLE_VALUE;
        return 0;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    SetCommTimeouts(puerto->handle, &timeouts);

    puerto->conectado = 1;
    return 1;
}

void serial_cerrar(PuertoSerial* puerto) {
    if (puerto->handle != INVALID_HANDLE_VALUE) {
        CloseHandle(puerto->handle);
        puerto->handle = INVALID_HANDLE_VALUE;
    }
    puerto->conectado = 0;
}

int serial_leer_byte(PuertoSerial* puerto, char* byte, DWORD timeoutMs) {
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = timeoutMs;
    SetCommTimeouts(puerto->handle, &timeouts);

    DWORD bytesLeidos = 0;
    if (!ReadFile(puerto->handle, byte, 1, &bytesLeidos, NULL)) {
        return 0;
    }

    return bytesLeidos == 1;
}