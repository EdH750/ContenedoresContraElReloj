#include "OpereacionesYComandos.h"

    /*--------------------------------------------------------------*/
    /* Compara el contenedor en el tope de la pila
        contenida en la la zona con codContenedor
    */
    /*--------------------------------------------------------------*/

int cmpContenedores(void* codContendor, void* zonaActual){
    tZona* zona= zonaActual;
    tPila* pilaCont;
    tContenedor contenedor;
    pilaCont=&(zona->pila);
    if(PilaVacia(pilaCont))
        return  CONTENEDOR_FALTANTE;
    VerTope(pilaCont,sizeof(tContenedor),&contenedor);

    return strcmp((char*)codContendor,contenedor.codigo);
}

    /*--------------------------------------------------------------*/
    /* Desapila el contenedor de la zona que llega como parametro
    */
    /*--------------------------------------------------------------*/
void liberarEspacioZona(void* zonaActual){
    tZona *zona= zonaActual;
    tPila* pilaCont;
    tContenedor contenedor;
    pilaCont=&(zona->pila);
    if(PilaVacia(pilaCont))
        return;
    SacarDePila(pilaCont,sizeof(tContenedor),&contenedor);
    zona->cantidad--;
}
int OperacionENT(tJornada* jornada){

    tCamion camionAtendido;
    tContenedor contBuscado;
    tCola* colaCamiones= &(jornada->puerto.colaCamiones);
    tLista* listaZonas=&(jornada->puerto.zonas);
    int resultadoBusqueda;
    int t_inicio = jornada->tiempoActual,
    t_fin = t_inicio + jornada->cfg.tiempoCargaCamion,
    t_finDeJornada= jornada->cfg.duracionJornadaMinutos;
    /*--------------------------------------------------------------*/
    /*1)Casos particulares de entrada
       1.1) Reviso que la cola de camiones no este vacia
       1.2)    "    "  la lista de zonas no este vacia
       1.3)    "    "  aun quede tiempo para realizar la operacion
    */
    /*--------------------------------------------------------------*/
    if(ColaVacia(colaCamiones))
        return COLA_CAMIONES_VACIA;
    if(ListaVacia(listaZonas))
        return LISTA_ZONAS_VACIA;
    if(t_fin>t_finDeJornada)
        return SIN_TIEMPO;
    /*--------------------------------------------------------------*/
    /* 2)Si ambas tienen al menos un elemento
        2.1)recupero de la cola de camiones el proximo camion
        2.2)guardo el contenedor del mismo para iniciar la busqueda
    */
    /*--------------------------------------------------------------*/
    VerPrimeroEnCola(colaCamiones,sizeof(tCamion),&camionAtendido);
    contBuscado=camionAtendido.pedido;
    /*--------------------------------------------------------------*/
    /* 3)Busco el contenedor dentro de la lista de zonas,
        modificando la cantidad de contenedores de la zona
        y desapilando si lo encuentra
        (Funcion usada en lista.h)
        (Comparacion y accion usada para la misma en esta misma hoja)
    */
    /*--------------------------------------------------------------*/
    resultadoBusqueda=buscarEnListaSinBorrar(listaZonas,
                                             contBuscado.codigo,
                                             MAX_COD_CONTENEDOR,
                                             cmpContenedores,
                                             liberarEspacioZona);
    /*--------------------------------------------------------------*/
    /*  4)Si se encontro el contBuscado
            4.1) Saco al camion actual de la cola de camiones
            4.2) Avanzo el tiempoActual segun lo que tarde la operacion
    */
    /*--------------------------------------------------------------*/
    if(resultadoBusqueda){
        SacarDeLaCola(colaCamiones,sizeof(tCamion),&camionAtendido);
        jornada->tiempoActual= t_fin;
        return CONTENEDOR_ENTREGADO;
    }

    return CONTENEDOR_FALTANTE;
}

