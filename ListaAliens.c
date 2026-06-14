#include <stdio.h>
#include <stdlib.h>
#include "estructuras.h"

NodoAlien* agregar_alien(NodoAlien* CabezaActual, int id, int x, int y, int tipo, int puntos) { //Crea alien (cabeza de lista)
    NodoAlien* NuevoNodo=(NodoAlien*)malloc(sizeof(NodoAlien));
    /*
    // Verificación de seguridad: si la RAM está llena, malloc devuelve NULL.
    if (NuevoNodo == NULL) {
        printf("Error: No hay memoria suficiente para crear el alien.\n");
        return CabezaActual; 
    }
    */
    NuevoNodo->dato.id = id; //Info del alien creado
    NuevoNodo->dato.x = x;
    NuevoNodo->dato.y = y;
    NuevoNodo->dato.tipo = tipo;
    NuevoNodo->dato.puntos = puntos;
    NuevoNodo->siguiente = CabezaActual; //Apuntar a siguiente en la lista

    return NuevoNodo;
}

NodoAlien* EliminarAlien(NodoAlien* CabezaActual, int IdABorrar) { //Elimina alien y libera memoria
    
    if (CabezaActual == NULL) return NULL;

    if (CabezaActual->dato.id == IdABorrar) { //Cabeza
        NodoAlien* NuevaCabeza = CabezaActual->siguiente; 
        free(CabezaActual);                               
        return NuevaCabeza;                         
    }

    NodoAlien* Actual = CabezaActual->siguiente; //En el cuerpo
    NodoAlien* Anterior = CabezaActual;

    while (Actual != NULL) {
        if (Actual->dato.id == IdABorrar) {
            Anterior->siguiente = Actual->siguiente;
            free(Actual); 
            return CabezaActual; 
        }
        Anterior = Actual;
        Actual = Actual->siguiente;
    }

    return CabezaActual;
}