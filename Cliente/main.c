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


DWORD WINAPI EscucharServidor(LPVOID Parametro) {
    SOCKET SocketServidor = *(SOCKET*)Parametro; //Socket desde el main
    char BufferRecepcion[1024];

    while (1) {
        memset(BufferRecepcion, 0, sizeof(BufferRecepcion));
        int BytesRecibidos = recv(SocketServidor, BufferRecepcion, sizeof(BufferRecepcion), 0);

        if (BytesRecibidos > 0) {
            printf("Mensaje de Java: %s\n", BufferRecepcion); //Mensaje de java
        } 
        else if (BytesRecibidos == 0 || BytesRecibidos == SOCKET_ERROR) {
            printf("Desconectado del servidor.\n");
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    printf("Iniciando Cliente de spaCEinvaders...\n");

    nave Canon;
    Canon.x = ANCHO_PANTALLA / 2;
    Canon.y = ALTO_PANTALLA - 50;
    Canon.vidas = VIDAS_INICIALES;
    Canon.puntuacion = 0;

    NodoAlien* CabezaListaAliens = NULL;

    WSADATA wsaData; //Socket
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET SocketCliente = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in DireccionServidor;
    DireccionServidor.sin_family = AF_INET;
    DireccionServidor.sin_port = htons(PUERTO_SERVIDOR); 
    DireccionServidor.sin_addr.s_addr = inet_addr(IP_SERVIDOR); 

    if (connect(SocketCliente, (struct sockaddr *)&DireccionServidor, sizeof(DireccionServidor)) == SOCKET_ERROR) {
        printf("Advertencia: No se pudo conectar a Java. Abriendo en modo offline.\n");
    } else {
        printf("Conectado exitosamente al Servidor!\n");
    }

    CreateThread(NULL, 0, EscucharServidor, &SocketCliente, 0, NULL);//Hilo para escuchar el servidor

    InitWindow(ANCHO_PANTALLA, ALTO_PANTALLA, "spaCEinvaders - Cliente Jugador");//Raylib
    SetTargetFPS(60); //Estos son los FPS constantes

    int JuegoActivo = 1;

    while (JuegoActivo && !WindowShouldClose()) { //Tocar la X o Esc
        
        //CONTROLES
        if (IsKeyDown(KEY_RIGHT)) Canon.x += 5;
        if (IsKeyDown(KEY_LEFT)) Canon.x -= 5;

        // B. DIBUJAR
        BeginDrawing();
        
        ClearBackground(BLACK);
        DrawRectangle(Canon.x, Canon.y, 40, 20, GREEN);//Cañón
        DrawText("spaCEinvaders", 10, 10, 20, LIGHTGRAY);// Texto en pantalla
        DrawText(TextFormat("Vidas: %d", Canon.vidas), 10, 40, 20, RED);

        EndDrawing();
    }

    printf("Cerrando el cliente...\n");
    closesocket(SocketCliente); 
    WSACleanup();  //Biblioteca en windows

    return 0;
}