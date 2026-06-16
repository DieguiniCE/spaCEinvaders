#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
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
<<<<<<< Updated upstream
=======
#include "ListaAliens.h"
>>>>>>> Stashed changes

typedef struct {
    SOCKET socketServidor;
    nave* canon;
<<<<<<< Updated upstream
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
=======
    NodoAlien** cabezaAliens;
    Proyectil* proyectil;
    CRITICAL_SECTION* bloqueoEstado;
    int* miIdJugador;
    char* textoBunkers;
    double* velocidadAliens;
    int* juegoActivo;
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
static int MiIdJugador = 1;
static char TextoBunkers[32] = "Bunkers 100%";
static double VelocidadAliens = 1.0;
static int JuegoActivo = 1;
static int SiguienteIdAlien = 1;

static void EnviarLineaAlServidor(SOCKET socketServidor, const char* mensaje) {
    if (socketServidor == INVALID_SOCKET || !ServidorConectado) {
        return;
    }

    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "%s\n", mensaje);
    send(socketServidor, buffer, len, 0);
}

static void EnviarComandoControlador(SOCKET socketServidor, char comando) {
    switch (comando) {
        case CMD_LEFT:
            EnviarLineaAlServidor(socketServidor, "MOVER,L");
            break;
        case CMD_RIGHT:
            EnviarLineaAlServidor(socketServidor, "MOVER,R");
            break;
        case CMD_FIRE:
            EnviarLineaAlServidor(socketServidor, "DISPARO");
            break;
        default:
            break;
    }
}

static void AplicarComandoLocal(nave* canon, Proyectil* proyectil, char comando) {
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
            if (!proyectil->activo) {
                proyectil->x = canon->x + ANCHO_CANON / 2 - 2;
                proyectil->y = canon->y - 10;
                proyectil->velocidad = 8;
                proyectil->activo = 1;
            }
            break;
        default:
            break;
    }
}

static Color ColorPorTipoAlien(int tipo) {
    switch (tipo) {
        case 2: return ORANGE;
        case 3: return PURPLE;
        default: return MAGENTA;
    }
}

static int TipoDesdePuntos(int puntos) {
    if (puntos >= 40) return 3;
    if (puntos >= 20) return 2;
    return 1;
}

static void ProcesarMensajeServidor(
    const char* mensaje,
    nave* canon,
    NodoAlien** cabezaAliens,
    Proyectil* proyectil,
    int* miIdJugador,
    char* textoBunkers,
    double* velocidadAliens,
    int* juegoActivo
) {
    int jugador = 0;
    int x = 0;
    int y = 0;
    int vidas = 0;
    int puntos = 0;
    int idAlien = 0;
    int ptsAlien = 0;
    char rol[16];

    if (sscanf(mensaje, "BIENVENIDA,%15s", rol) == 1) {
        if (strncmp(rol, "J", 1) == 0) {
            *miIdJugador = rol[1] - '0';
        }
        return;
    }

    if (sscanf(mensaje, "STATE_J%d,%d,%d,%d,%d", &jugador, &x, &y, &vidas, &puntos) == 5) {
        if (jugador == *miIdJugador) {
            canon->x = x;
            canon->y = y;
            canon->vidas = vidas;
            canon->puntuacion = puntos;
        }
        return;
    }

    if (sscanf(mensaje, "PUNTOS_J%d,%d", &jugador, &puntos) == 2 && jugador == *miIdJugador) {
        canon->puntuacion = puntos;
        return;
    }

    if (sscanf(mensaje, "VIDAS_J%d,%d", &jugador, &vidas) == 2 && jugador == *miIdJugador) {
        canon->vidas = vidas;
        return;
    }

    if (sscanf(mensaje, "GAME_OVER_J%d,", &jugador) == 1 && jugador == *miIdJugador) {
        *juegoActivo = 0;
        return;
    }

    if (sscanf(mensaje, "NUEVO_ALIEN,%d,%d,%d,%d", &idAlien, &x, &y, &ptsAlien) == 4) {
        *cabezaAliens = agregar_alien(*cabezaAliens, idAlien, x * 50, y * 40, TipoDesdePuntos(ptsAlien), ptsAlien);
        if (idAlien >= SiguienteIdAlien) {
            SiguienteIdAlien = idAlien + 1;
        }
        return;
    }

    if (strncmp(mensaje, "NUEVO_OVNI,", 11) == 0) {
        return;
    }

    if (sscanf(mensaje, "VELOCIDAD,%lf", velocidadAliens) == 1) {
        return;
    }

    if (sscanf(mensaje, "BUNKERS,%31s", textoBunkers) == 1) {
        return;
    }

    if (sscanf(mensaje, "DISPARO_J%d,%d", &jugador, &x) == 2 && jugador == *miIdJugador) {
        if (!proyectil->activo) {
            proyectil->x = x + ANCHO_CANON / 2 - 2;
            proyectil->y = canon->y - 10;
            proyectil->velocidad = 8;
            proyectil->activo = 1;
        }
    }
}

static void ActualizarProyectil(Proyectil* proyectil, NodoAlien** cabezaAliens, SOCKET socket) {
    if (!proyectil->activo) {
        return;
    }

    proyectil->y -= proyectil->velocidad;
    if (proyectil->y < 0) {
        proyectil->activo = 0;
        return;
    }

    NodoAlien* actual = *cabezaAliens;

    while (actual != NULL) {
        Alien* alien = &actual->dato;
        if (proyectil->x >= alien->x && proyectil->x <= alien->x + 30 &&
            proyectil->y >= alien->y && proyectil->y <= alien->y + 20) {
            char mensaje[64];
            const char* tipo = "calamar";
            if (alien->tipo == 2) tipo = "cangrejo";
            else if (alien->tipo == 3) tipo = "pulpo";

            snprintf(mensaje, sizeof(mensaje), "MATE_ALIEN,%s", tipo);
            EnviarLineaAlServidor(socket, mensaje);

            *cabezaAliens = eliminar_alien(*cabezaAliens, alien->id);
            proyectil->activo = 0;
            return;
        }
        actual = actual->siguiente;
    }
}

DWORD WINAPI EscucharServidor(LPVOID parametro) {
    ContextoServidor* contexto = (ContextoServidor*)parametro;
>>>>>>> Stashed changes
    char bufferRecepcion[1024];
    char linea[1024];
    int indiceLinea = 0;

    while (1) {
        memset(bufferRecepcion, 0, sizeof(bufferRecepcion));
<<<<<<< Updated upstream
        int bytesRecibidos = recv(contexto->socketServidor, bufferRecepcion, sizeof(bufferRecepcion) - 1, 0);
=======
        int bytesRecibidos = recv(contexto->socketServidor, bufferRecepcion,
                                  sizeof(bufferRecepcion) - 1, 0);
>>>>>>> Stashed changes

        if (bytesRecibidos > 0) {
            for (int i = 0; i < bytesRecibidos; i++) {
                char c = bufferRecepcion[i];
                if (c == '\n' || c == '\r') {
                    if (indiceLinea > 0) {
                        linea[indiceLinea] = '\0';
<<<<<<< Updated upstream
                        ProcesarEstadoServidor(linea, contexto->canon);
                        printf("Estado del servidor: %s\n", linea);
=======
                        EnterCriticalSection(contexto->bloqueoEstado);
                        ProcesarMensajeServidor(
                            linea,
                            contexto->canon,
                            contexto->cabezaAliens,
                            contexto->proyectil,
                            contexto->miIdJugador,
                            contexto->textoBunkers,
                            contexto->velocidadAliens,
                            contexto->juegoActivo
                        );
                        LeaveCriticalSection(contexto->bloqueoEstado);
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
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
=======
DWORD WINAPI EscucharControlador(LPVOID parametro) {
    ContextoControlador* contexto = (ContextoControlador*)parametro;
    char byteComando = 0;

    while (contexto->puerto->conectado) {
        if (serial_leer_byte(contexto->puerto, &byteComando, 100)) {
            if (byteComando == CMD_LEFT || byteComando == CMD_RIGHT || byteComando == CMD_FIRE) {
                *(contexto->comandoPendiente) = byteComando;
                if (*(contexto->servidorConectado)) {
                    EnviarComandoControlador(contexto->socketServidor, byteComando);
                }
            }
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    printf("Iniciando Cliente de spaCEinvaders...\n");
>>>>>>> Stashed changes

    const char* puertoSerial = (argc > 1) ? argv[1] : PUERTO_SERIAL;

<<<<<<< Updated upstream
    while (JuegoActivo && !WindowShouldClose()) {

=======
    nave canon;
    canon.x = ANCHO_PANTALLA / 2;
    canon.y = ALTO_PANTALLA - 50;
    canon.vidas = VIDAS_INICIALES;
    canon.puntuacion = 0;

    Proyectil proyectil = {0, 0, 8, 0};
    NodoAlien* cabezaListaAliens = NULL;

    InitializeCriticalSection(&BloqueoEstado);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_port = htons(PUERTO_SERVIDOR);
    direccionServidor.sin_addr.s_addr = inet_addr(IP_SERVIDOR);

    if (connect(socketCliente, (struct sockaddr*)&direccionServidor, sizeof(direccionServidor)) == SOCKET_ERROR) {
        printf("Advertencia: No se pudo conectar al servidor Java. Modo offline.\n");
        ServidorConectado = 0;
    } else {
        printf("Conectado al servidor en %s:%d\n", IP_SERVIDOR, PUERTO_SERVIDOR);
        ServidorConectado = 1;
    }

    ContextoServidor contextoServidor = {
        socketCliente, &canon, &cabezaListaAliens, &proyectil,
        &BloqueoEstado, &MiIdJugador, TextoBunkers, &VelocidadAliens, &JuegoActivo
    };
    CreateThread(NULL, 0, EscucharServidor, &contextoServidor, 0, NULL);

    PuertoSerial puertoControlador;
    HANDLE hiloControlador = NULL;
    ContextoControlador contextoControlador;

    if (serial_abrir(&puertoControlador, puertoSerial, UART_BAUD_RATE)) {
        printf("Controlador Pico conectado en %s (%d baud)\n", puertoSerial, UART_BAUD_RATE);
        contextoControlador.puerto = &puertoControlador;
        contextoControlador.socketServidor = socketCliente;
        contextoControlador.servidorConectado = &ServidorConectado;
        contextoControlador.comandoPendiente = &ComandoPendiente;
        hiloControlador = CreateThread(NULL, 0, EscucharControlador, &contextoControlador, 0, NULL);
    } else {
        printf("Sin controlador en %s. Teclado: flechas + espacio.\n", puertoSerial);
    }

    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "spaCEinvaders - Cliente Jugador");
    SetTargetFPS(60);

    while (JuegoActivo && !WindowShouldClose()) {
>>>>>>> Stashed changes
        if (ComandoPendiente != 0) {
            char comando = ComandoPendiente;
            ComandoPendiente = 0;
            EnterCriticalSection(&BloqueoEstado);
<<<<<<< Updated upstream
            AplicarComandoLocal(&Canon, comando);
=======
            AplicarComandoLocal(&canon, &proyectil, comando);
>>>>>>> Stashed changes
            LeaveCriticalSection(&BloqueoEstado);
        }

        if (IsKeyDown(KEY_RIGHT)) {
<<<<<<< Updated upstream
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
=======
            AplicarComandoLocal(&canon, &proyectil, CMD_RIGHT);
            EnviarComandoControlador(socketCliente, CMD_RIGHT);
        }
        if (IsKeyDown(KEY_LEFT)) {
            AplicarComandoLocal(&canon, &proyectil, CMD_LEFT);
            EnviarComandoControlador(socketCliente, CMD_LEFT);
        }
        if (IsKeyPressed(KEY_SPACE)) {
            AplicarComandoLocal(&canon, &proyectil, CMD_FIRE);
            EnviarComandoControlador(socketCliente, CMD_FIRE);
        }

        EnterCriticalSection(&BloqueoEstado);
        ActualizarProyectil(&proyectil, &cabezaListaAliens, socketCliente);
        LeaveCriticalSection(&BloqueoEstado);

        BeginDrawing();
        ClearBackground(BLACK);

        EnterCriticalSection(&BloqueoEstado);
        for (NodoAlien* nodo = cabezaListaAliens; nodo != NULL; nodo = nodo->siguiente) {
            DrawRectangle(nodo->dato.x, nodo->dato.y, 30, 20, ColorPorTipoAlien(nodo->dato.tipo));
        }

        DrawRectangle(canon.x, canon.y, ANCHO_CANON, ALTO_CANON, GREEN);
        if (proyectil.activo) {
            DrawRectangle(proyectil.x, proyectil.y, 4, 10, YELLOW);
        }

        DrawText("spaCEinvaders", 10, 10, 20, LIGHTGRAY);
        DrawText(TextFormat("Jugador %d", MiIdJugador), 650, 10, 20, SKYBLUE);
        DrawText(TextFormat("Vidas: %d", canon.vidas), 10, 40, 20, RED);
        DrawText(TextFormat("Puntos: %d", canon.puntuacion), 10, 70, 20, YELLOW);
        DrawText(TextFormat("Velocidad aliens: %.1f", VelocidadAliens), 10, 100, 18, ORANGE);
        DrawText(TextFormat("Bunkers: %s", TextoBunkers), 10, 125, 18, LIME);

        if (puertoControlador.conectado) {
            DrawText(TextFormat("Controlador: %s", puertoSerial), 10, 155, 18, SKYBLUE);
        } else {
            DrawText("Controlador: no conectado (teclado activo)", 10, 155, 18, GRAY);
        }
        LeaveCriticalSection(&BloqueoEstado);
>>>>>>> Stashed changes

        EndDrawing();
    }

    printf("Cerrando el cliente...\n");

<<<<<<< Updated upstream
    if (PuertoControlador.conectado) {
        serial_cerrar(&PuertoControlador);
=======
    if (puertoControlador.conectado) {
        serial_cerrar(&puertoControlador);
>>>>>>> Stashed changes
    }
    if (hiloControlador != NULL) {
        WaitForSingleObject(hiloControlador, 1000);
        CloseHandle(hiloControlador);
    }

<<<<<<< Updated upstream
    closesocket(SocketCliente);
=======
    liberar_lista_aliens(cabezaListaAliens);
    closesocket(socketCliente);
>>>>>>> Stashed changes
    WSACleanup();
    DeleteCriticalSection(&BloqueoEstado);

    return 0;
}
