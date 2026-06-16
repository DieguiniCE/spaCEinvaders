#include <stdio.h>
#include <stdlib.h>
#include "ListaAliens.h"

NodoAlien* agregar_alien(NodoAlien* cabezaActual, int id, int x, int y, int tipo, int puntos) {
    NodoAlien* nuevoNodo = (NodoAlien*)malloc(sizeof(NodoAlien));
    if (nuevoNodo == NULL) {
        return cabezaActual;
    }

    nuevoNodo->dato.id = id;
    nuevoNodo->dato.x = x;
    nuevoNodo->dato.y = y;
    nuevoNodo->dato.tipo = tipo;
    nuevoNodo->dato.puntos = puntos;
    nuevoNodo->siguiente = cabezaActual;

    return nuevoNodo;
}

NodoAlien* eliminar_alien(NodoAlien* cabezaActual, int idABorrar) {
    if (cabezaActual == NULL) {
        return NULL;
    }

    if (cabezaActual->dato.id == idABorrar) {
        NodoAlien* nuevaCabeza = cabezaActual->siguiente;
        free(cabezaActual);
        return nuevaCabeza;
    }

    NodoAlien* actual = cabezaActual->siguiente;
    NodoAlien* anterior = cabezaActual;

    while (actual != NULL) {
        if (actual->dato.id == idABorrar) {
            anterior->siguiente = actual->siguiente;
            free(actual);
            return cabezaActual;
        }
        anterior = actual;
        actual = actual->siguiente;
    }

    return cabezaActual;
}

void liberar_lista_aliens(NodoAlien* cabeza) {
    while (cabeza != NULL) {
        NodoAlien* siguiente = cabeza->siguiente;
        free(cabeza);
        cabeza = siguiente;
    }
}
