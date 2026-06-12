#include <stdio.h>
#include <stdlib.h>

#include "constantes.h"
#include "estructuras.h"

int main(int argc, char *argv[]) {
    printf("Iniciando Cliente de spaCEinvaders...\n");
    printf("Configurado para el puerto: %d\n", PUERTO_SERVIDOR);

    //Jugador :7
    nave canon;
    canon.x = ANCHO_PANTALLA / 2;
    canon.y = ALTO_PANTALLA - 50;
    canon.vidas = VIDAS_INICIALES;
    canon.puntuacion = 0;

    printf("Jugador listo en posicion X: %d, Y: %d con %d vidas.\n", canon.x, canon.y, canon.vidas);

    //Puntero inicial para la lista de aliens
    NodoAlien* CabezaListaAliens = NULL;

    // --- Aqui ira la configuracion del Socket Cliente ---
    
    // --- Aqui ira el Game Loop (ciclo infinito del juego) ---

    return 0;
}