/**
 * espectador.c
 * Cliente espectador: se conecta al servidor y observa la partida sin enviar controles.
 * Uso: espectador.exe
 */
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
#include "ListaAliens.h"

typedef struct {
    int activo;
    int x;
    int y;
    int vidas;
    int puntos;
} EstadoJugador;

typedef struct {
    int x;
    int y;
    int vida;
} BunkerLocal;

typedef struct {
    int activo;
    int id;
    int x;
    int y;
    int direccion;
    int puntos;
} OvniLocal;

static EstadoJugador Jugador1 = {0, 0, ALTO_PANTALLA - 50, 3, 0};
static EstadoJugador Jugador2 = {0, 0, ALTO_PANTALLA - 50, 3, 0};
static NodoAlien* CabezaAliens = NULL;
static char TextoBunkers[BUFFER_BUNKERS] = BUNKER_TEXTO_INICIAL;
static double VelocidadAliens = 1.0;
static BunkerLocal Bunkers[4];
static OvniLocal OvniActual = {0, 0, 0, 0, 0, 0};
static CRITICAL_SECTION BloqueoEstado;

static void InicializarBunkers(void) {
    int posicionesX[BUNKER_CANTIDAD] = {BUNKER_POS_0, BUNKER_POS_1, BUNKER_POS_2, BUNKER_POS_3};
    for (int i = 0; i < BUNKER_CANTIDAD; i++) {
        Bunkers[i].x = posicionesX[i];
        Bunkers[i].y = ALTO_PANTALLA - BUNKER_Y_OFFSET;
        Bunkers[i].vida = BUNKER_LIFE_INICIAL;
    }
}

static void ActualizarBunkersDesdeTexto(const char* texto) {
    int vida0 = BUNKER_LIFE_INICIAL, vida1 = BUNKER_LIFE_INICIAL, vida2 = BUNKER_LIFE_INICIAL, vida3 = BUNKER_LIFE_INICIAL;
    if (sscanf(texto, "%d,%d,%d,%d", &vida0, &vida1, &vida2, &vida3) == 4) {
        Bunkers[0].vida = vida0;
        Bunkers[1].vida = vida1;
        Bunkers[2].vida = vida2;
        Bunkers[3].vida = vida3;
    } else if (sscanf(texto, "%d%%", &vida0) == 1 || sscanf(texto, "%d", &vida0) == 1) {
        Bunkers[0].vida = vida0;
        Bunkers[1].vida = vida0;
        Bunkers[2].vida = vida0;
        Bunkers[3].vida = vida0;
    }
}

static void DibujarBunkers(void) {
    for (int i = 0; i < BUNKER_CANTIDAD; i++) {
        if (Bunkers[i].vida <= 0) {
            continue;
        }

        Color color = DARKGREEN;
        if (Bunkers[i].vida < BUNKER_LIFE_ALTA) color = GREEN;
        if (Bunkers[i].vida < BUNKER_LIFE_MEDIA) color = LIME;
        if (Bunkers[i].vida < BUNKER_LIFE_BAJA) color = YELLOW;

        DrawRectangle(Bunkers[i].x, Bunkers[i].y, BUNKER_ANCHO, BUNKER_ALTO, color);
        DrawRectangleLines(Bunkers[i].x, Bunkers[i].y, BUNKER_ANCHO, BUNKER_ALTO, DARKGRAY);
    }
}

static void ProcesarLineaEspectador(const char* linea) {
    int jugador = 0;
    int x = 0;
    int y = 0;
    int vidas = 0;
    int puntos = 0;
    int idAlien = 0;
    int ptsAlien = 0;

    if (sscanf(linea, "STATE_J%d,%d,%d,%d,%d", &jugador, &x, &y, &vidas, &puntos) == 5) {
        EstadoJugador* estado = (jugador == 1) ? &Jugador1 : &Jugador2;
        estado->activo = 1;
        estado->x = x;
        estado->y = y;
        estado->vidas = vidas;
        estado->puntos = puntos;
        return;
    }

    if (sscanf(linea, "NUEVO_ALIEN,%d,%d,%d,%d", &idAlien, &x, &y, &ptsAlien) == 4) {
        int tipo = (ptsAlien >= 40) ? 3 : ((ptsAlien >= 20) ? 2 : 1);
        CabezaAliens = agregar_alien(CabezaAliens, idAlien, x, y, tipo, ptsAlien);
        return;
    }

    if (sscanf(linea, "MOVER_ALIEN,%d,%d,%d", &idAlien, &x, &y) == 3) {
        for (NodoAlien* nodo = CabezaAliens; nodo != NULL; nodo = nodo->siguiente) {
            if (nodo->dato.id == idAlien) {
                nodo->dato.x = x;
                nodo->dato.y = y;
                break;
            }
        }
        return;
    }

    if (sscanf(linea, "NUEVO_OVNI,%d,%d,%d,%d,%d", &idAlien, &x, &y, &vidas, &puntos) == 5) {
        OvniActual.activo = 1;
        OvniActual.id = idAlien;
        OvniActual.x = x;
        OvniActual.y = y;
        OvniActual.direccion = vidas;
        OvniActual.puntos = puntos;
        return;
    }

    if (sscanf(linea, "MOVER_OVNI,%d,%d,%d", &idAlien, &x, &y) == 3) {
        if (OvniActual.activo && OvniActual.id == idAlien) {
            OvniActual.x = x;
            OvniActual.y = y;
        }
        return;
    }

    if (sscanf(linea, "BORRAR_OVNI,%d", &idAlien) == 1) {
        if (OvniActual.activo && OvniActual.id == idAlien) {
            OvniActual.activo = 0;
        }
        return;
    }

    if (sscanf(linea, "BORRAR_ALIEN,%d", &idAlien) == 1) {
        CabezaAliens = eliminar_alien(CabezaAliens, idAlien);
        return;
    }

    if (sscanf(linea, "BORRAR_JUGADOR,%d", &idAlien) == 1) {
        if (idAlien == 1) {
            Jugador1.activo = 0;
        } else if (idAlien == 2) {
            Jugador2.activo = 0;
        }
        return;
    }

    if (sscanf(linea, "VELOCIDAD,%lf", &VelocidadAliens) == 1) {
        return;
    }

    if (sscanf(linea, "BUNKERS,%31s", TextoBunkers) == 1) {
        ActualizarBunkersDesdeTexto(TextoBunkers);
        return;
    }
}

DWORD WINAPI EscucharServidorEspectador(LPVOID parametro) {
    SOCKET socketServidor = *(SOCKET*)parametro;
    char buffer[1024];
    char linea[1024];
    int indice = 0;

    while (1) {
        int bytes = recv(socketServidor, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            break;
        }

        for (int i = 0; i < bytes; i++) {
            char c = buffer[i];
            if (c == '\n' || c == '\r') {
                if (indice > 0) {
                    linea[indice] = '\0';
                    EnterCriticalSection(&BloqueoEstado);
                    ProcesarLineaEspectador(linea);
                    LeaveCriticalSection(&BloqueoEstado);
                    indice = 0;
                }
            } else if (indice < (int)sizeof(linea) - 1) {
                linea[indice++] = c;
            }
        }
    }
    return 0;
}

int main(void) {
    InitializeCriticalSection(&BloqueoEstado);
    InicializarBunkers();

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in direccion;
    direccion.sin_family = AF_INET;
    direccion.sin_port = htons(PUERTO_SERVIDOR);
    direccion.sin_addr.s_addr = inet_addr(IP_SERVIDOR);

    if (connect(socketCliente, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        printf("No se pudo conectar al servidor.\n");
        return 1;
    }

    send(socketCliente, "ROL,ESPECTADOR\n", (int)strlen("ROL,ESPECTADOR\n"), 0);
    printf("Espectador conectado. Observando partida...\n");
    CreateThread(NULL, 0, EscucharServidorEspectador, &socketCliente, 0, NULL);

    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "spaCEinvaders - Espectador");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        EnterCriticalSection(&BloqueoEstado);
        for (NodoAlien* nodo = CabezaAliens; nodo != NULL; nodo = nodo->siguiente) {
            DrawRectangle(nodo->dato.x, nodo->dato.y, 30, 20, MAGENTA);
        }

        if (OvniActual.activo) {
            DrawRectangle(OvniActual.x, OvniActual.y, 48, 20, SKYBLUE);
            DrawText(TextFormat("%d", OvniActual.puntos), OvniActual.x + 8, OvniActual.y - 14, 12, SKYBLUE);
        }

        DibujarBunkers();

        if (Jugador1.activo) {
            DrawRectangle(Jugador1.x, Jugador1.y, ANCHO_CANON, ALTO_CANON, GREEN);
        }
        if (Jugador2.activo) {
            DrawRectangle(Jugador2.x, Jugador2.y, ANCHO_CANON, ALTO_CANON, BLUE);
        }

        DrawText("spaCEinvaders - ESPECTADOR", 10, 10, 20, LIGHTGRAY);
        DrawText(TextFormat("J1: %d pts | %d vidas", Jugador1.puntos, Jugador1.vidas), 10, 40, 18, GREEN);
        DrawText(TextFormat("J2: %d pts | %d vidas", Jugador2.puntos, Jugador2.vidas), 10, 65, 18, BLUE);
        DrawText(TextFormat("Velocidad: %.1f | Bunkers: %s", VelocidadAliens, TextoBunkers), 10, 90, 18, ORANGE);
        LeaveCriticalSection(&BloqueoEstado);

        EndDrawing();
    }

    liberar_lista_aliens(CabezaAliens);
    closesocket(socketCliente);
    WSACleanup();
    DeleteCriticalSection(&BloqueoEstado);
    return 0;
}
