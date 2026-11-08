#ifndef JORNADA_H_INCLUDED
#define JORNADA_H_INCLUDED

#include "ContenedoresContraReloj.h"
#include "escenario.h"
#include "pila.h"
#include "cola.h"
#include "lista.h"

#define LARGO_CODIGO      3
#define MAX_COD_OPERACION (LARGO_CODIGO + 1)

typedef struct {
    int    id;                           /* 1 = M1 */
    int    ocupado;                      /* 0 = libre: buque no tiene datos validos */
    tBuque buque;
} tMuelle;

typedef struct {
    int   id;                            /* 1 = Z1 */
    int   cantidad;                      /* se compara con cfg.capacidadPila */
    tPila pila;                          /* elementos: tContenedor */
} tZona;

typedef struct {
    int         tiempoInicio;
    int         tiempoFin;
    char        codigo[MAX_COD_OPERACION];   /* "DES", "REU", "ENT", "ESP" */
    tContenedor contenedor;                  /* "." en ESP */
    char        origen[MAX_ID];              /* "M1", "Z2" o "." */
    char        destino[MAX_ID];             /* "Z3", "K004" o "." */
} tOperacion;

typedef enum {
    JORNADA_EN_CURSO,
    FIN_NORMAL,
    FIN_BLOQUEO,
    FIN_TIEMPO
} tMotivoFin;

typedef tLista tHistorial;               /* elementos: tOperacion */

typedef struct {
    tMuelle* muelles;                    /* vector fijo con malloc */
    int      cantMuelles;

    tCola    colaBuques;                 /* elementos: tBuque */
    int      buquesEnEspera;

    tCola    colaCamiones;               /* elementos: tCamion */
    int      camionesEnEspera;

    tLista   zonas;                      /* elementos: tZona */
} tPuerto;

typedef struct {
    int        contenedoresEntregados;
    int        buquesDescargados;
    int        camionesPendientes;       /* se completa al cerrar */
    int        reubicaciones;
    int        tiempoUtilizado;          /* se completa al cerrar */
    int        puntuacion;               /* se completa al cerrar */
    tMotivoFin motivoFin;
} tResumen;

typedef struct {
    tConfiguracion cfg;
    int            tiempoActual;
    tEscenario     escenario;
    tPuerto        puerto;
    tResumen       resumen;
    tHistorial     historial;
} tJornada;

int iniciar_jornada(tJornada* pJornada, const tConfiguracion* pConfiguracion);
int ejecutar_jornada(tJornada* pJornada);
void liberar_jornada(tJornada* pJornada);
int avanzar_reloj(tJornada* pJornada, int tiempoNuevo);
void evaluar_estado(tJornada* pJornada);
int procesar_arribos(tJornada* pJornada, int tiempoLimite);
int arribo_buque(tPuerto* pPuerto, const tBuque* pBuque);
void asignar_muelles(tPuerto* pPuerto);
int buscar_muelle_libre(const tPuerto* pPuerto);
int hay_buque_atracado(const tPuerto* pPuerto);
void mostrar_puerto(const tJornada* pJornada);
int crear_puerto(tPuerto* pPuerto, const tConfiguracion* pConfiguracion);
void liberar_puerto(tPuerto* pPuerto);
int proximo_tiempo_arribo(tJornada* pJornada);
int hay_operacion_posible(tJornada* pJornada);
int hay_ent_posible(tJornada* pJornada);
int hay_reu_posible(tJornada* pJornada);
int hay_des_posible(tJornada* pJornada);
int validar_orden(tJornada* pJornada, const tOperacion* pOperacion, int cantParametros);
int costo_operacion(const char* codigo, const tConfiguracion* cfg);

#endif /* JORNADA_H_INCLUDED */
