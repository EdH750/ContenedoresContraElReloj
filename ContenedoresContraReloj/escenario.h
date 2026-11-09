#ifndef ESCENARIO_H_INCLUDED
#define ESCENARIO_H_INCLUDED

#include "ContenedoresContraReloj.h"
#include "lista.h"

#define MAX_ID 8

/* Lo que no depende del manifiesto en una linea de buque: id, ";T=", minuto, ";C=" y fin de linea */
#define LARGO_FIJO_LINEA 32

typedef struct {
    char codigo[MAX_COD_CONTENEDOR];     /* "C101" */
} tContenedor;

typedef struct {
    char         id[MAX_ID];             /* "B001" */
    int          tiempoLlegada;
    tContenedor* manifiesto;             /* vector con malloc, en orden de descarga */
    int          cantContenedores;
    int          proximoADescargar;      /* si llega a cantContenedores, el buque termino */
} tBuque;

typedef struct {
    char        id[MAX_ID];              /* "K001" */
    int         tiempoLlegada;
    tContenedor pedido;
} tCamion;

/* Cada buque y cada camion existe en un solo lugar a la vez: al llegar se
   desenlista de aca y pasa a la cola de espera o al muelle */
typedef struct {
    tLista buques;                       /* elementos: tBuque, ordenados por tiempoLlegada */

    tLista camiones;                     /* elementos: tCamion, ordenados por tiempoLlegada */
} tEscenario;

/* Parte de puerto.txt que se esta leyendo */
typedef enum {
    SECCION_CABECERA,
    SECCION_BUQUES,
    SECCION_CAMIONES
} tSeccion;

int cargar_escenario(const char* nombreArchivo, const tConfiguracion* pConfiguracion, tEscenario* pEscenario);
void liberar_escenario(tEscenario* pEscenario);
int cargar_buque(char* linea, tBuque* pBuque);
void cargar_camion(char* linea, tCamion* pCamion);
int contar_codigos(const char* codigos);

#endif /* ESCENARIO_H_INCLUDED */
