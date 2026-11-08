#ifndef PILA_H_INCLUDED
#define PILA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Estructuras.h"

#define SIN_MEM 1
#define TODO_OK 2
#define PILA_VACIA 3

#define MINIMO(X,Y) ( (X)<= (Y) ? (X) : (Y) )

void CrearPila(tPila* p);
int PilaLlena(const tPila* p, unsigned tam);
int PonerEnPila(tPila* p, unsigned tam, void* dato);
int VerTope(const tPila* p, unsigned tam, void* dato);
int PilaVacia(const tPila* p);
int SacarDePila(tPila* p, unsigned tam, void* dato);
void VaciarPila(tPila* p);

#endif // PILA_H_INCLUDED
