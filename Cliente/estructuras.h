#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

//Nave
typedef struct {
    int x;
    int y;
    int vidas;
    int puntuacion;
} nave;

//Alien
typedef struct {
    int id;       // Para identificarlo cuando el servidor mande a borrarlo
    int x;
    int y;
    int tipo;     // 1: Calamar, 2: Cangrejo, 3: Pulpo
    int puntos;
} Alien;

//Lista enlazada de Aliens
typedef struct NodoAlien {
    Alien dato;         // La información
    struct NodoAlien* siguiente; // Puntero al siguiente en la lista
} NodoAlien;

//Proyectil
typedef struct {
    int x;
    int y;
    int velocidad;
    int activo;   // 1 si está en pantalla, 0 si ya impactó
} Proyectil;

#endif // ESTRUCTURAS_H