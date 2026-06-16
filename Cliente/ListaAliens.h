#ifndef LISTA_ALIENS_H
#define LISTA_ALIENS_H

#include "estructuras.h"

NodoAlien* agregar_alien(NodoAlien* cabezaActual, int id, int x, int y, int tipo, int puntos);
NodoAlien* eliminar_alien(NodoAlien* cabezaActual, int idABorrar);
void liberar_lista_aliens(NodoAlien* cabeza);

#endif /* LISTA_ALIENS_H */
