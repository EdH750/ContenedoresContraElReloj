#ifndef OPERADOR_H_INCLUDED
#define OPERADOR_H_INCLUDED

#include "jornada.h"

#define MAX_NOMBRE_OPERADOR 30
#define OPERADORES_ARCHIVO  "operadores.bin"
#define JORNADAS_ARCHIVO    "jornadas.bin"
#define PRIMERA_JORNADA     1
#define PRIMERA_POSICION    1

typedef struct {
    int  id;
    char nombre[MAX_NOMBRE_OPERADOR];
} tOperador;

typedef struct {
    int  idJornada;
    int  idOperador;
    char nombreOperador[MAX_NOMBRE_OPERADOR];
    int  puntuacion;
    int  contenedoresEntregados;
    int  buquesDescargados;
    int  camionesPendientes;
    int  reubicaciones;
    int  tiempoUtilizado;
    int  motivoFin;
} tRegistroJornada;

typedef struct {
    int  idOperador;
    char nombre[MAX_NOMBRE_OPERADOR];
    int  totalPuntuacion;
    int  jornadasJugadas;
    int  totalEntregados;
    int  totalBuquesDescargados;
} tEntradaRanking;

int iniciar_sesion_operador(tOperador* pOperador);
int guardar_jornada(const tJornada* pJornada, const tOperador* pOperador);
void mostrar_ranking(void);
int cmp_operador_por_nombre(const void* a, const void* b);
int cmp_entrada_ranking_por_id(const void* dato, const void* clave);
int cmp_ranking(const void* a, const void* b);

#endif /* OPERADOR_H_INCLUDED */
