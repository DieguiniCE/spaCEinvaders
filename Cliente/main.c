#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>


#pragma comment(lib, "ws2_32.lib") //Enlaza la librería estática de sockets en Windows

#include "constantes.h"
#include "estructuras.h"

int main(int argc, char *argv[]) {
    printf("Iniciando Cliente de spaCEinvaders...\n");
    printf("Configurado para el puerto: %d\n", PUERTO_SERVIDOR);

    nave canon;
    canon.x = ANCHO_PANTALLA / 2;
    canon.y = ALTO_PANTALLA - 50;
    canon.vidas = VIDAS_INICIALES;
    canon.puntuacion = 0;

    printf("Jugador listo en posicion X: %d, Y: %d con %d vidas.\n", canon.x, canon.y, canon.vidas);

    // Puntero inicial para la lista de aliens
    NodoAlien* CabezaListaAliens = NULL;
    
    WSADATA wsaData; //WinSock
    int resultadoWSA = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (resultadoWSA != 0) {
        printf("Error: WSAStartup fallo con error: %d\n", resultadoWSA);
        return -1;
    }

    SOCKET SocketCliente;
    struct sockaddr_in DireccionServidor;
    char BufferRecepcion[1024] = {0};

    SocketCliente = socket(AF_INET, SOCK_STREAM, 0);//Socket
    if (SocketCliente == INVALID_SOCKET) {
        printf("Error: No se pudo crear el socket. Codigo de error: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    printf("Socket creado exitosamente.\n");

    DireccionServidor.sin_family = AF_INET;
    DireccionServidor.sin_port = htons(PUERTO_SERVIDOR); 
    DireccionServidor.sin_addr.s_addr = inet_addr(IP_SERVIDOR);

    printf("Intentando conectar al servidor Java en la IP %s...\n", IP_SERVIDOR);
    if (connect(SocketCliente, (struct sockaddr *)&DireccionServidor, sizeof(DireccionServidor)) == SOCKET_ERROR) {
        printf("Error: La conexion al servidor fallo. Asegurate de que Java este corriendo.\n");
        closesocket(SocketCliente);
        WSACleanup();
        return -1;
    }
    printf("Conectado exitosamente al Servidor!\n");


    int JuegoActivo = 1;

    while (JuegoActivo) {
        memset(BufferRecepcion, 0, sizeof(BufferRecepcion));

        int BytesRecibidos = recv(SocketCliente, BufferRecepcion, sizeof(BufferRecepcion), 0); //"read()"

        if (BytesRecibidos > 0) {
            printf("Mensaje del Servidor: %s\n", BufferRecepcion);
        } else if (BytesRecibidos == 0) {
            printf("El servidor ha cerrado la conexion.\n");
            JuegoActivo = 0; 
        } else {
            printf("Error leyendo el mensaje del servidor.\n");
            JuegoActivo = 0;
        }
    }

    printf("Cerrando el cliente...\n");
    closesocket(SocketCliente); 
    WSACleanup();  //Biblioteca en windows

    return 0;
}