#ifndef ESTRUCTURAS_H_INCLUDED
#define ESTRUCTURAS_H_INCLUDED

#define MAX_COD_CONTENEDOR 12

typedef struct sNodo
{
    void* info;
    unsigned tamInfo;
    struct sNodo *sig;
}tNodo;

typedef struct
{
    tNodo *primero, *ultimo;
}tCola;

typedef tNodo *tLista;

typedef tNodo *tPila;

typedef struct
{
    char codigo[MAX_COD_CONTENEDOR];
}tContenedor;

typedef struct
{
    char id[8];
    int tLlegada;
    tPila manifiesto;
    int cantContenedores;
    int proxADescargar;
}tBuque;

typedef struct
{
    char id[8];
    int tLlegada;
    tContenedor pedido;
}tCamion;

typedef struct
{
    int id;
    tBuque *buque;
}tMuelle;

typedef struct
{
    int id;
    int cantidad;
    tPila pila;
}tZona;

typedef struct
{
    int tInicio;
    int tFin;
    char codigo[4];
    tContenedor contenedor;
    char origen[8];
    char destino[8];
}tOperacion;

typedef enum
{
    JORNADA_EN_CURSO,
    FIN_NORMAL,

    FIN_BLOQUEO,
    FIN_TIEMPO,
}tMotivoFin;

typedef tLista tHistorial;

typedef struct
{
    tLista buques;
    int cantBuques;
    int proxBuque;
    tLista camiones;
    int cantCamiones;
    int proxCamion;
}tEscenario;

typedef struct
{
    tLista muelles;
    int cantMuelles;
    tCola colaBuques;
    tCola colaCamiones;
    tLista zonas;
}tPuerto;

typedef struct
{
    int contenedoresEntregados;
    int buquesDescargados;
    int camionesPendientes;
    int reubicaciones;
    int tiempoUtilizado;
    int puntuacion;
    tMotivoFin motivoFin;
}tResumen;

typedef struct
{
    int duracionJornadaMinutos;
    int cantidadMuelles;
    int cantidadZonasAlmacenamiento;
    int capacidadPila;
    int maximoBuques;
    int maximoContenedoresPorBuque;
    int maximoCamiones;
    int tiempoDescargaContenedor;
    int tiempoReubicacionContenedor;
    int tiempoCargaCamion;
}tConfiguracion;

typedef struct
{
    tConfiguracion cfg;
    int tiempoActual;
    tEscenario escenario;
    tPuerto puerto;
    tResumen resumen;
    tHistorial historial;
}tJornada;

#endif // ESTRUCTURAS_H_INCLUDED
