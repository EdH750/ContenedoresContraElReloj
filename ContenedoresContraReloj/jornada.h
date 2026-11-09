#ifndef JORNADA_H_INCLUDED
#define JORNADA_H_INCLUDED

#include "ContenedoresContraReloj.h"
#include "escenario.h"
#include "TDA/pila.h"
#include "TDA/cola.h"
#include "TDA/lista.h"

#define LARGO_CODIGO      3
#define MAX_COD_OPERACION (LARGO_CODIGO + 1)

#define ANCHO_RECUADRO    73             /* texto dentro de "| ... |" en la ayuda */
#define VENTANA_CAMIONES  3              /* camiones visibles en VER desde el frente */

typedef struct {
    int    id;                           /* 1 = M1 */
    int    ocupado;                      /* 0 = libre: buque no tiene datos validos */
    tBuque buque;
} tMuelle;

typedef struct {
    int   id;                            /* 1 = Z1 */
    int   cantidad;                      /* contenedores actuales */
    int   capacidad;                     /* maximo: copia de cfg.capacidadPila */
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
int procesar_orden(tJornada* pJornada, char* linea);
int parsear_orden(char* linea, tOperacion* pOperacion);
int parametros_esperados(const char* codigo);
int validar_orden(tJornada* pJornada, const tOperacion* pOperacion, int cantParametros);
int parsear_id_muelle(const char* param);
int parsear_id_zona(const char* param);
int costo_operacion(const char* codigo, const tConfiguracion* cfg);
/* Helpers de zona */
int ver_cantidad_zona(tLista* pZonas, int idZona);
int contenedor_en_algun_tope(tLista* pZonas, const char* codigoCont);
int comparar_zona_por_id(const void* dato, const void* clave);
int comparar_zona_por_tope(const void* dato, const void* clave);
void desapilar_de_zona(tLista* pZonas, int idZona, tContenedor* pContenedor);
int apilar_en_zona(tLista* pZonas, int idZona, const tContenedor* pContenedor);
int buscar_zona_con_tope(tLista* pZonas, const char* codigoCont);

/* Ejecucion de operaciones */
int ejecutar_des(tJornada* pJornada, const tOperacion* pOperacion);
int ejecutar_reu(tJornada* pJornada, const tOperacion* pOperacion);
int ejecutar_ent(tJornada* pJornada, const tOperacion* pOperacion);

/* Reloj y eventos */
int avanzar_reloj(tJornada* pJornada, int tiempoNuevo);
void evaluar_estado(tJornada* pJornada);
int procesar_arribos(tJornada* pJornada, int tiempoLimite);
int arribo_buque(tPuerto* pPuerto, const tBuque* pBuque);
void asignar_muelles(tPuerto* pPuerto);
int buscar_muelle_libre(const tPuerto* pPuerto);
int hay_buque_atracado(const tPuerto* pPuerto);

/* Paso 9: deteccion de bloqueo y salto automatico */
int hay_des_posible(tJornada* pJornada);
int hay_reu_posible(tJornada* pJornada);
int hay_ent_posible(tJornada* pJornada);
int hay_operacion_posible(tJornada* pJornada);
int proximo_tiempo_arribo(tJornada* pJornada);

/* Visualizacion */
void linea_recuadro(const char* texto);
void separador_recuadro(char extremo, char relleno);
void mostrar_ayuda(const tConfiguracion* pConfiguracion);
int mostrar_puerto(tJornada* pJornada);
void mostrar_muelle(const tMuelle* pMuelle);
void accion_mostrar_zona(void* dato);
int mostrar_zonas(tLista* pZonas, int cantZonas, int capacidad);
int mostrar_zona(tZona* pZona, int capacidad, tPila* pAux);
int mostrar_ventana_camiones(tCola* pCola, int cantCamiones);
int calcular_puntuacion(const tJornada* pJornada);

/* Paso 10: cierre de jornada */
void accion_mostrar_operacion(void* dato);
void mostrar_historial(tHistorial* pHistorial);
void mostrar_resumen(const tJornada* pJornada);

/* Puerto */
int crear_puerto(tPuerto* pPuerto, const tConfiguracion* pConfiguracion);
void liberar_puerto(tPuerto* pPuerto);

#endif /* JORNADA_H_INCLUDED */
