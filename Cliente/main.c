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
#include "ListaAliens.h"

typedef struct {
    SOCKET socketServidor;
    nave* canon;
    NodoAlien** cabezaAliens;
    Proyectil* proyectil;
    CRITICAL_SECTION* bloqueoEstado;
    int* miIdJugador;
    char* textoBunkers;
    double* velocidadAliens;
    int* juegoActivo;
} ContextoServidor;

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
static char TextoBunkers[32] = "100%";
static double VelocidadAliens = 1.0;
static int JuegoActivo = 1;
static NodoAlien* cabezaListaAliens = NULL;
static BunkerLocal Bunkers[4];
static OvniLocal OvniActual = {0, 0, 0, 0, 0, 0};
static int DireccionAliens = 1;
static int DisparoAlienActivo = 0;
static int DisparoAlienX = 0;
static int DisparoAlienY = 0;
static int DisparoAlienVelocidad = 0;
static unsigned int UltimoDisparoAlienMs = 0;

static void InicializarBunkers(void) {
    int posicionesX[4] = {120, 275, 430, 585};
    for (int i = 0; i < 4; i++) {
        Bunkers[i].x = posicionesX[i];
        Bunkers[i].y = ALTO_PANTALLA - 150;
        Bunkers[i].vida = 100;
    }
}

static void ActualizarBunkersDesdeTexto(const char* texto) {
    int vida0 = 100, vida1 = 100, vida2 = 100, vida3 = 100;
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

static int HitBunker(int x, int y, int ancho, int alto) {
    for (int i = 0; i < 4; i++) {
        if (Bunkers[i].vida <= 0) {
            continue;
        }

        if (x < Bunkers[i].x + 60 && x + ancho > Bunkers[i].x &&
            y < Bunkers[i].y + 35 && y + alto > Bunkers[i].y) {
            return i;
        }
    }

    return -1;
}

static void DañarBunkerLocal(int indice, int dano) {
    if (indice < 0 || indice >= 4 || Bunkers[indice].vida <= 0) {
        return;
    }

    Bunkers[indice].vida -= dano;
    if (Bunkers[indice].vida < 0) {
        Bunkers[indice].vida = 0;
    }
}

static void DibujarBunkers(void) {
    for (int i = 0; i < 4; i++) {
        if (Bunkers[i].vida <= 0) {
            continue;
        }

        Color color = DARKGREEN;
        if (Bunkers[i].vida < 75) color = GREEN;
        if (Bunkers[i].vida < 50) color = LIME;
        if (Bunkers[i].vida < 25) color = YELLOW;

        DrawRectangle(Bunkers[i].x, Bunkers[i].y, 60, 35, color);
        DrawRectangleLines(Bunkers[i].x, Bunkers[i].y, 60, 35, DARKGRAY);
    }
}

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
            if (canon->x < 0) canon->x = 0;
            break;
        case CMD_RIGHT:
            canon->x += VELOCIDAD_CANON;
            if (canon->x > ANCHO_PANTALLA - ANCHO_CANON) canon->x = ANCHO_PANTALLA - ANCHO_CANON;
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

static void CrearOleadaAliens(void) {
    liberar_lista_aliens(cabezaListaAliens);
    cabezaListaAliens = NULL;
    DireccionAliens = 1;
    DisparoAlienActivo = 0;
    UltimoDisparoAlienMs = 0;

    int siguienteId = 1;
    for (int fila = 0; fila < 4; fila++) {
        int tipo = 1;
        int puntos = PTS_CALAMAR;

        if (fila == 0) {
            tipo = 3;
            puntos = PTS_PULPO;
        } else if (fila < 3) {
            tipo = 2;
            puntos = PTS_CANGREJO;
        }

        for (int columna = 0; columna < 10; columna++) {
            int x = 100 + (columna * 48);
            int y = 60 + (fila * 34);
            cabezaListaAliens = agregar_alien(cabezaListaAliens, siguienteId++, x, y, tipo, puntos);
        }
    }
}

static void ActualizarAliens(float deltaTime, nave* canon, SOCKET socketServidor) {
    if (ServidorConectado) {
        return;
    }

    if (cabezaListaAliens == NULL) {
        return;
    }

    int minX = ANCHO_PANTALLA;
    int maxX = 0;
    for (NodoAlien* nodo = cabezaListaAliens; nodo != NULL; nodo = nodo->siguiente) {
        if (nodo->dato.x < minX) minX = nodo->dato.x;
        if (nodo->dato.x > maxX) maxX = nodo->dato.x;
    }

    int paso = (int)(VelocidadAliens * deltaTime);
    if (paso < 1) {
        paso = 1;
    }

    int golpeaBorde = 0;
    if (DireccionAliens > 0 && maxX + 30 + paso >= ANCHO_PANTALLA - 20) {
        golpeaBorde = 1;
    }
    if (DireccionAliens < 0 && minX - paso <= 20) {
        golpeaBorde = 1;
    }

    if (golpeaBorde) {
        DireccionAliens *= -1;
        for (NodoAlien* nodo = cabezaListaAliens; nodo != NULL; nodo = nodo->siguiente) {
            nodo->dato.y += 16;
        }
    } else {
        for (NodoAlien* nodo = cabezaListaAliens; nodo != NULL; nodo = nodo->siguiente) {
            nodo->dato.x += (paso * DireccionAliens);
        }
    }

    NodoAlien** enlaceActual = &cabezaListaAliens;
    while (*enlaceActual != NULL) {
        NodoAlien* nodo = *enlaceActual;
        int alienX = nodo->dato.x;
        int alienY = nodo->dato.y;
        int alienAncho = 30;
        int alienAlto = 20;
        int alienCayoEnJugador = (alienY + alienAlto >= canon->y && alienX < canon->x + ANCHO_CANON && alienX + alienAncho > canon->x);

        int bunkerImpactado = HitBunker(alienX, alienY, alienAncho, alienAlto);
        if (bunkerImpactado >= 0) {
            DañarBunkerLocal(bunkerImpactado, 20);
            char mensajeBunker[64];
            snprintf(mensajeBunker, sizeof(mensajeBunker), "BUNKER_HIT,%d,%d", bunkerImpactado, 20);
            EnviarLineaAlServidor(socketServidor, mensajeBunker);
            *enlaceActual = nodo->siguiente;
            free(nodo);
            continue;
        }

        if (alienCayoEnJugador || alienY + alienAlto >= canon->y) {
            if (canon->vidas > 0) {
                canon->vidas -= 1;
            }
            EnviarLineaAlServidor(socketServidor, "PERDI_VIDA");
            *enlaceActual = nodo->siguiente;
            free(nodo);
            if (canon->vidas <= 0) {
                JuegoActivo = 0;
            }
            continue;
        }

        enlaceActual = &(*enlaceActual)->siguiente;
    }
}

static NodoAlien* AlienMasBajoPorColumna(int columnaObjetivo) {
    NodoAlien* mejor = NULL;

    for (NodoAlien* nodo = cabezaListaAliens; nodo != NULL; nodo = nodo->siguiente) {
        if ((nodo->dato.x / 48) == columnaObjetivo) {
            if (mejor == NULL || nodo->dato.y > mejor->dato.y) {
                mejor = nodo;
            }
        }
    }

    return mejor;
}

static void ActualizarDisparoAlien(float deltaTime, nave* canon, SOCKET socketServidor) {
    if (DisparoAlienActivo) {
        DisparoAlienY += DisparoAlienVelocidad;
        if (DisparoAlienY > ALTO_PANTALLA) {
            DisparoAlienActivo = 0;
        } else {
            int bunkerImpactado = HitBunker(DisparoAlienX, DisparoAlienY, 4, 10);
            if (bunkerImpactado >= 0) {
                DañarBunkerLocal(bunkerImpactado, 10);
                char mensaje[64];
                snprintf(mensaje, sizeof(mensaje), "BUNKER_HIT,%d,%d", bunkerImpactado, 10);
                EnviarLineaAlServidor(socketServidor, mensaje);
                DisparoAlienActivo = 0;
                return;
            }

            if (DisparoAlienX >= canon->x && DisparoAlienX <= canon->x + ANCHO_CANON &&
                DisparoAlienY >= canon->y && DisparoAlienY <= canon->y + ALTO_CANON) {
                if (canon->vidas > 0) {
                    canon->vidas -= 1;
                }
                EnviarLineaAlServidor(socketServidor, "PERDI_VIDA");
                DisparoAlienActivo = 0;
                if (canon->vidas <= 0) {
                    JuegoActivo = 0;
                }
            }
        }
    }

    if (DisparoAlienActivo || cabezaListaAliens == NULL) {
        return;
    }

    unsigned int ahoraMs = (unsigned int)(GetTime() * 1000.0);
    if (ahoraMs - UltimoDisparoAlienMs < 900) {
        return;
    }

    int columnaObjetivo = rand() % 10;
    NodoAlien* atacante = AlienMasBajoPorColumna(columnaObjetivo);
    if (atacante != NULL) {
        DisparoAlienActivo = 1;
        DisparoAlienX = atacante->dato.x + 12;
        DisparoAlienY = atacante->dato.y + 20;
        DisparoAlienVelocidad = 6;
        UltimoDisparoAlienMs = ahoraMs;
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
        *cabezaAliens = agregar_alien(*cabezaAliens, idAlien, x, y, TipoDesdePuntos(ptsAlien), ptsAlien);
        return;
    }

    if (sscanf(mensaje, "MOVER_ALIEN,%d,%d,%d", &idAlien, &x, &y) == 3) {
        for (NodoAlien* nodo = *cabezaAliens; nodo != NULL; nodo = nodo->siguiente) {
            if (nodo->dato.id == idAlien) {
                nodo->dato.x = x;
                nodo->dato.y = y;
                break;
            }
        }
        return;
    }

    if (sscanf(mensaje, "NUEVO_OVNI,%d,%d,%d,%d,%d", &idAlien, &x, &y, &vidas, &puntos) == 5) {
        OvniActual.activo = 1;
        OvniActual.id = idAlien;
        OvniActual.x = x;
        OvniActual.y = y;
        OvniActual.direccion = vidas;
        OvniActual.puntos = puntos;
        return;
    }

    if (sscanf(mensaje, "MOVER_OVNI,%d,%d,%d", &idAlien, &x, &y) == 3) {
        if (OvniActual.activo && OvniActual.id == idAlien) {
            OvniActual.x = x;
            OvniActual.y = y;
        }
        return;
    }

    if (sscanf(mensaje, "BORRAR_OVNI,%d", &idAlien) == 1) {
        if (OvniActual.activo && OvniActual.id == idAlien) {
            OvniActual.activo = 0;
        }
        return;
    }

    if (sscanf(mensaje, "BORRAR_ALIEN,%d", &idAlien) == 1) {
        *cabezaAliens = eliminar_alien(*cabezaAliens, idAlien);
        return;
    }

    if (strncmp(mensaje, "NUEVO_OVNI,", 11) == 0) {
        return;
    }

    if (sscanf(mensaje, "VELOCIDAD,%lf", velocidadAliens) == 1) {
        return;
    }

    if (sscanf(mensaje, "BUNKERS,%31s", textoBunkers) == 1) {
        ActualizarBunkersDesdeTexto(textoBunkers);
        return;
    }

    if (sscanf(mensaje, "BUNKER_HIT,%d,%d", &idAlien, &vidas) == 2) {
        DañarBunkerLocal(idAlien, vidas);
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

static void ActualizarProyectil(Proyectil* proyectil, NodoAlien** cabezaAliens, SOCKET socket, nave* canon) {
    if (!proyectil->activo) {
        return;
    }

    proyectil->y -= proyectil->velocidad;
    if (proyectil->y < 0) {
        proyectil->activo = 0;
        return;
    }

    int bunkerImpactado = HitBunker(proyectil->x, proyectil->y, 4, 10);
    if (bunkerImpactado >= 0) {
        DañarBunkerLocal(bunkerImpactado, 20);
        char mensajeBunker[64];
        snprintf(mensajeBunker, sizeof(mensajeBunker), "BUNKER_HIT,%d,%d", bunkerImpactado, 20);
        EnviarLineaAlServidor(socket, mensajeBunker);
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

            snprintf(mensaje, sizeof(mensaje), "MATE_ALIEN,%d,%s", alien->id, tipo);
            EnviarLineaAlServidor(socket, mensaje);

            canon->puntuacion += alien->puntos;

            *cabezaAliens = eliminar_alien(*cabezaAliens, alien->id);
            proyectil->activo = 0;
            return;
        }
        actual = actual->siguiente;
    }

    if (OvniActual.activo &&
        proyectil->x >= OvniActual.x && proyectil->x <= OvniActual.x + 48 &&
        proyectil->y >= OvniActual.y && proyectil->y <= OvniActual.y + 20) {
        char mensaje[64];
        snprintf(mensaje, sizeof(mensaje), "MATE_OVNI,%d", OvniActual.id);
        EnviarLineaAlServidor(socket, mensaje);
        canon->puntuacion += OvniActual.puntos;
        OvniActual.activo = 0;
        proyectil->activo = 0;
        return;
    }
}

DWORD WINAPI EscucharServidor(LPVOID parametro) {
    ContextoServidor* contexto = (ContextoServidor*)parametro;
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
                        indiceLinea = 0;
                    }
                } else if (indiceLinea < (int)sizeof(linea) - 1) {
                    linea[indiceLinea++] = c;
                }
            }
        } else if (bytesRecibidos == 0 || bytesRecibidos == SOCKET_ERROR) {
            ServidorConectado = 0;
            break;
        }
    }

    return 0;
}

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

    const char* puertoSerial = (argc > 1) ? argv[1] : PUERTO_SERIAL;

    nave canon;
    canon.x = ANCHO_PANTALLA / 2;
    canon.y = ALTO_PANTALLA - 50;
    canon.vidas = VIDAS_INICIALES;
    canon.puntuacion = 0;

    Proyectil proyectil = {0, 0, 8, 0};
    InicializarBunkers();

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

    if (!ServidorConectado) {
        CrearOleadaAliens();
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
    srand((unsigned int)GetTickCount());

    while (JuegoActivo && !WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (ComandoPendiente != 0) {
            char comando = ComandoPendiente;
            ComandoPendiente = 0;
            EnterCriticalSection(&BloqueoEstado);
            AplicarComandoLocal(&canon, &proyectil, comando);
            LeaveCriticalSection(&BloqueoEstado);
        }

        if (IsKeyDown(KEY_RIGHT)) {
            EnterCriticalSection(&BloqueoEstado);
            AplicarComandoLocal(&canon, &proyectil, CMD_RIGHT);
            LeaveCriticalSection(&BloqueoEstado);
            if (ServidorConectado) EnviarComandoControlador(socketCliente, CMD_RIGHT);
        }
        if (IsKeyDown(KEY_LEFT)) {
            EnterCriticalSection(&BloqueoEstado);
            AplicarComandoLocal(&canon, &proyectil, CMD_LEFT);
            LeaveCriticalSection(&BloqueoEstado);
            if (ServidorConectado) EnviarComandoControlador(socketCliente, CMD_LEFT);
        }
        if (IsKeyPressed(KEY_SPACE)) {
            EnterCriticalSection(&BloqueoEstado);
            AplicarComandoLocal(&canon, &proyectil, CMD_FIRE);
            LeaveCriticalSection(&BloqueoEstado);
            if (ServidorConectado) EnviarComandoControlador(socketCliente, CMD_FIRE);
        }

        EnterCriticalSection(&BloqueoEstado);
        ActualizarAliens(deltaTime, &canon, socketCliente);
        ActualizarProyectil(&proyectil, &cabezaListaAliens, socketCliente, &canon);
        ActualizarDisparoAlien(deltaTime, &canon, socketCliente);
        LeaveCriticalSection(&BloqueoEstado);

        BeginDrawing();
        ClearBackground(BLACK);

        EnterCriticalSection(&BloqueoEstado);
        for (NodoAlien* nodo = cabezaListaAliens; nodo != NULL; nodo = nodo->siguiente) {
            DrawRectangle(nodo->dato.x, nodo->dato.y, 30, 20, ColorPorTipoAlien(nodo->dato.tipo));
        }

        if (proyectil.activo) {
            DrawRectangle(proyectil.x, proyectil.y, 4, 10, YELLOW);
        }

        if (DisparoAlienActivo) {
            DrawRectangle(DisparoAlienX, DisparoAlienY, 4, 10, RED);
        }

        if (OvniActual.activo) {
            DrawRectangle(OvniActual.x, OvniActual.y, 48, 20, SKYBLUE);
            DrawText(TextFormat("%d", OvniActual.puntos), OvniActual.x + 8, OvniActual.y - 14, 12, SKYBLUE);
        }

        DibujarBunkers();

        DrawRectangle(canon.x, canon.y, ANCHO_CANON, ALTO_CANON, GREEN);
        DrawText("spaCEinvaders", 10, 10, 20, LIGHTGRAY);
        DrawText(TextFormat("J%d | Vidas: %d | Puntos: %d", MiIdJugador, canon.vidas, canon.puntuacion), 10, 40, 18, WHITE);
        DrawText(TextFormat("Velocidad aliens: %.1f | Bunkers: %s", VelocidadAliens, TextoBunkers), 10, 65, 18, ORANGE);
        LeaveCriticalSection(&BloqueoEstado);

        EndDrawing();
    }

    liberar_lista_aliens(cabezaListaAliens);
    serial_cerrar(&puertoControlador);
    if (hiloControlador != NULL) {
        CloseHandle(hiloControlador);
    }
    closesocket(socketCliente);
    WSACleanup();
    DeleteCriticalSection(&BloqueoEstado);
    return 0;
}