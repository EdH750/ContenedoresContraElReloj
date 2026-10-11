#ifndef OPEREACIONESYCOMANDOS_H_INCLUDED
#define OPEREACIONESYCOMANDOS_H_INCLUDED

#include <string.h>
#include <stdio.h>
#include "cola.h"
#include "pila.h"
#include "Estructuras.h"
#include "lista.h"

#define ZONAS_IGUALES 2
#define ZONA_INEXISTENTE 3
#define ZONA_VACIA 4
#define ZONA_LLENA 5
#define SIN_TIEMPO 6

void OperacionVER(tJornada *Jornada);
int OperacionREU(tJornada *jornada, int idOrigen, int idDestino);
void ArmarCodigo(char *destino, char *letra, int id);
int ContarBuquesEnEspera(tCola *cola);
void ImprimirPilaContenedores(tPila *pila);
void ImprimirVentanaDePlanificacion(tCola *cola);

#endif // OPEREACIONESYCOMANDOS_H_INCLUDED
