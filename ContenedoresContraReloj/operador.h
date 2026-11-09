#ifndef OPERADOR_H_INCLUDED
#define OPERADOR_H_INCLUDED

/* operador.h incluye jornada.h para poder usar tJornada en guardar_jornada.
   jornada.h no incluye operador.h, por lo que no hay dependencia circular */
#include "jornada.h"

/* ─── Constantes ─────────────────────────────────────────────────────── */

#define MAX_NOMBRE_OPERADOR 30       /* largo maximo del nombre (con '\0') */
#define OPERADORES_ARCHIVO  "operadores.bin"  /* registro maestro de operadores */
#define JORNADAS_ARCHIVO    "jornadas.bin"    /* historico de jornadas jugadas */

/* ─── Tipos de datos ─────────────────────────────────────────────────── */

/* Registro de un operador (en sesion y en operadores.bin) */
typedef struct {
    int  id;                              /* 1, 2, 3... asignado al registrarse */
    char nombre[MAX_NOMBRE_OPERADOR];
} tOperador;

/* Un registro en jornadas.bin (uno por jornada completada).
   Se escribe en binario con fwrite: el tamano es fijo para poder calcular
   la posicion de cualquier registro con ftell / sizeof */
typedef struct {
    int  idJornada;                       /* numero secuencial (1, 2, 3...) */
    int  idOperador;
    char nombreOperador[MAX_NOMBRE_OPERADOR];
    int  puntuacion;
    int  contenedoresEntregados;
    int  buquesDescargados;
    int  camionesPendientes;
    int  reubicaciones;
    int  tiempoUtilizado;
    int  motivoFin;                       /* valor del enum tMotivoFin */
} tRegistroJornada;

/* Una entrada del ranking (acumula varias jornadas del mismo operador) */
typedef struct {
    int  idOperador;
    char nombre[MAX_NOMBRE_OPERADOR];
    int  totalPuntuacion;
    int  jornadasJugadas;
    int  totalEntregados;
    int  totalBuquesDescargados;
} tEntradaRanking;

/* ─── Prototipos ─────────────────────────────────────────────────────── */

/* Pide el nombre al operario, lo busca en operadores.bin via ABB.
   Si no existe lo registra como nuevo. Llena *pOperador.
   Devuelve 1 si tuvo exito, 0 si hubo un error (ej: nombre vacio o malloc) */
int iniciar_sesion_operador(tOperador* pOperador);

/* Escribe un registro en jornadas.bin con el resultado de la jornada.
   Debe llamarse antes de liberar_jornada (usa pJornada->resumen).
   Devuelve 1 si tuvo exito, 0 si fallo al abrir el archivo */
int guardar_jornada(const tJornada* pJornada, const tOperador* pOperador);

/* Lee jornadas.bin, acumula resultados por operador, ordena y muestra
   el ranking. Si el archivo no existe avisa y retorna */
void mostrar_ranking(void);

/* Comparadores (tambien usados en operador.c, expuestos para tests) */
int cmp_operador_por_nombre(const void* a, const void* b);
int cmp_entrada_ranking_por_id(const void* dato, const void* clave);
int cmp_ranking_qsort(const void* a, const void* b);

#endif /* OPERADOR_H_INCLUDED */
