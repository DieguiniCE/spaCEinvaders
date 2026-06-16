#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN  //Carga solo un pedazo de windows
#define NOGDI
#define NOUSER
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "include/raylib.h"
#pragma comment(lib, "ws2_32.lib")
#include "constantes.h"
#include "estructuras.h"
#include "serial.h"

typedef struct {
    SOCKET socketServidor;
    nave* canon;
    CRITICAL_SECTION* bloqueoEstado;
} ContextoServidor;

typedef struct {
    PuertoSerial* puerto;
    SOCKET socketServidor;
    int* servidorConectado;
    volatile char* comandoPendiente;
} ContextoControlador;

static int ServidorConectado = 0;
static volatile char ComandoPendiente = 0;
static CRITICAL_SECTION BloqueoEstado;

static void EnviarComandoAlServidor(SOCKET socketServidor, char comando) {
    if (socketServidor == INVALID_SOCKET) {
        return;
    }

    send(socketServidor, &comando, 1, 0);
}

static void AplicarComandoLocal(nave* canon, char comando) {
    switch (comando) {
        case CMD_LEFT:
            canon->x -= VELOCIDAD_CANON;
            if (canon->x < 0) {
                canon->x = 0;
            }
            break;
        case CMD_RIGHT:
            canon->x += VELOCIDAD_CANON;
            if (canon->x > ANCHO_PANTALLA - ANCHO_CANON) {
                canon->x = ANCHO_PANTALLA - ANCHO_CANON;
            }
            break;
        case CMD_FIRE:
            printf("Disparo!\n");
            break;
        default:
            break;
    }
}

static void ProcesarEstadoServidor(const char* mensaje, nave* canon) {
    int x = 0;
    int y = 0;
    int vidas = 0;
    int puntos = 0;

    if (sscanf(mensaje, "STATE:%d,%d,%d,%d", &x, &y, &vidas, &puntos) == 4) {
        EnterCriticalSection(&BloqueoEstado);
        canon->x = x;
        canon->y = y;
        canon->vidas = vidas;
        canon->puntuacion = puntos;
        LeaveCriticalSection(&BloqueoEstado);
    }
}

DWORD WINAPI EscucharServidor(LPVOID Parametro) {
    ContextoServidor* contexto = (ContextoServidor*)Parametro;
    char bufferRecepcion[1024];
    char linea[1024];
    int indiceLinea = 0;

    while (1) {
        memset(bufferRecepcion, 0, sizeof(bufferRecepcion));
        int bytesRecibidos = recv(contexto->socketServidor, bufferRecepcion, sizeof(bufferRecepcion) - 1, 0);

        if (bytesRecibidos > 0) {
            for (int i = 0; i < bytesRecibidos; i++) {
                char c = bufferRecepcion[i];
                if (c == '\n' || c == '\r') {
                    if (indiceLinea > 0) {
                        linea[indiceLinea] = '\0';
                        ProcesarEstadoServidor(linea, contexto->canon);
                        printf("Estado del servidor: %s\n", linea);
                        indiceLinea = 0;
                    }
                } else if (indiceLinea < (int)sizeof(linea) - 1) {
                    linea[indiceLinea++] = c;
                }
            }
        } else if (bytesRecibidos == 0 || bytesRecibidos == SOCKET_ERROR) {
            printf("Desconectado del servidor.\n");
            ServidorConectado = 0;
            break;
        }
    }
    return 0;
}

DWORD WINAPI EscucharControlador(LPVOID Parametro) {
    ContextoControlador* contexto = (ContextoControlador*)Parametro;
    char byteComando = 0;

    while (contexto->puerto->conectado) {
        if (serial_leer_byte(contexto->puerto, &byteComando, 100)) {
            if (byteComando == CMD_LEFT || byteComando == CMD_RIGHT || byteComando == CMD_FIRE) {
                printf("Comando del controlador: %c\n", byteComando);
                *(contexto->comandoPendiente) = byteComando;

                if (*(contexto->servidorConectado)) {
                    EnviarComandoAlServidor(contexto->socketServidor, byteComando);
                }
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    printf("Iniciando Cliente de spaCEinvaders...\n");

    const char* puertoSerial = (argc > 1) ? argv[1] : PUERTO_SERIAL;

    nave Canon;
    Canon.x = ANCHO_PANTALLA / 2;
    Canon.y = ALTO_PANTALLA - 50;
    Canon.vidas = VIDAS_INICIALES;
    Canon.puntuacion = 0;

    NodoAlien* CabezaListaAliens = NULL;
    (void)CabezaListaAliens;

    InitializeCriticalSection(&BloqueoEstado);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET SocketCliente = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in DireccionServidor;
    DireccionServidor.sin_family = AF_INET;
    DireccionServidor.sin_port = htons(PUERTO_SERVIDOR);
    DireccionServidor.sin_addr.s_addr = inet_addr(IP_SERVIDOR);

    if (connect(SocketCliente, (struct sockaddr *)&DireccionServidor, sizeof(DireccionServidor)) == SOCKET_ERROR) {
        printf("Advertencia: No se pudo conectar a Java. Abriendo en modo offline.\n");
        ServidorConectado = 0;
    } else {
        printf("Conectado exitosamente al Servidor!\n");
        ServidorConectado = 1;
    }

    ContextoServidor contextoServidor = { SocketCliente, &Canon, &BloqueoEstado };
    CreateThread(NULL, 0, EscucharServidor, &contextoServidor, 0, NULL);

    PuertoSerial PuertoControlador;
    HANDLE hiloControlador = NULL;
    ContextoControlador contextoControlador;

    if (serial_abrir(&PuertoControlador, puertoSerial, UART_BAUD_RATE)) {
        printf("Controlador conectado en %s (%d baud)\n", puertoSerial, UART_BAUD_RATE);
        contextoControlador.puerto = &PuertoControlador;
        contextoControlador.socketServidor = SocketCliente;
        contextoControlador.servidorConectado = &ServidorConectado;
        contextoControlador.comandoPendiente = &ComandoPendiente;
        hiloControlador = CreateThread(NULL, 0, EscucharControlador, &contextoControlador, 0, NULL);
    } else {
        printf("Advertencia: No se pudo abrir %s. Solo teclado disponible.\n", puertoSerial);
    }

    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "spaCEinvaders - Cliente Jugador");
    SetTargetFPS(60);

    int JuegoActivo = 1;

    while (JuegoActivo && !WindowShouldClose()) {

        if (ComandoPendiente != 0) {
            char comando = ComandoPendiente;
            ComandoPendiente = 0;
            EnterCriticalSection(&BloqueoEstado);
            AplicarComandoLocal(&Canon, comando);
            LeaveCriticalSection(&BloqueoEstado);
        }

        if (IsKeyDown(KEY_RIGHT)) {
            AplicarComandoLocal(&Canon, CMD_RIGHT);
            if (ServidorConectado) {
                EnviarComandoAlServidor(SocketCliente, CMD_RIGHT);
            }
        }
        if (IsKeyDown(KEY_LEFT)) {
            AplicarComandoLocal(&Canon, CMD_LEFT);
            if (ServidorConectado) {
                EnviarComandoAlServidor(SocketCliente, CMD_LEFT);
            }
        }
        if (IsKeyPressed(KEY_SPACE)) {
            AplicarComandoLocal(&Canon, CMD_FIRE);
            if (ServidorConectado) {
                EnviarComandoAlServidor(SocketCliente, CMD_FIRE);
            }
        }

        BeginDrawing();

        ClearBackground(BLACK);
        EnterCriticalSection(&BloqueoEstado);
        DrawRectangle(Canon.x, Canon.y, ANCHO_CANON, 20, GREEN);
        DrawText("spaCEinvaders", 10, 10, 20, LIGHTGRAY);
        DrawText(TextFormat("Vidas: %d", Canon.vidas), 10, 40, 20, RED);
        DrawText(TextFormat("Puntos: %d", Canon.puntuacion), 10, 70, 20, YELLOW);
        LeaveCriticalSection(&BloqueoEstado);

        if (PuertoControlador.conectado) {
            DrawText(TextFormat("Controlador: %s", puertoSerial), 10, 100, 18, SKYBLUE);
        }

        EndDrawing();
    }

    printf("Cerrando el cliente...\n");

    if (PuertoControlador.conectado) {
        serial_cerrar(&PuertoControlador);
    }
    if (hiloControlador != NULL) {
        WaitForSingleObject(hiloControlador, 1000);
        CloseHandle(hiloControlador);
    }

    closesocket(SocketCliente);
    WSACleanup();
    DeleteCriticalSection(&BloqueoEstado);

    return 0;
}
