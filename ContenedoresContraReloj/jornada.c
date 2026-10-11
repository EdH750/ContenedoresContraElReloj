#include "jornada.h"

int iniciar_jornada(tJornada* pJornada, const tConfiguracion* pConfiguracion) {

    pJornada->cfg = *pConfiguracion;

    if(cargar_escenario(PUERTO_ARCHIVO, &pJornada->cfg, &pJornada->escenario) == 0) {
        return 0;
    }

    if(crear_puerto(&pJornada->puerto, &pJornada->cfg) == 0) {
        liberar_escenario(&pJornada->escenario);
        return 0;
    }

    pJornada->tiempoActual = 0;

    pJornada->resumen.contenedoresEntregados = 0;
    pJornada->resumen.buquesDescargados      = 0;
    pJornada->resumen.camionesPendientes     = 0;
    pJornada->resumen.reubicaciones          = 0;
    pJornada->resumen.tiempoUtilizado        = 0;
    pJornada->resumen.puntuacion             = 0;
    pJornada->resumen.motivoFin              = JORNADA_EN_CURSO;

    crear_lista(&pJornada->historial);

    return 1;
}

int ejecutar_jornada(tJornada* pJornada) {

    char orden[MAX_BUFFER];

    printf("\nComienza la jornada (duracion: %d minutos)\n", pJornada->cfg.duracionJornadaMinutos);

    /* Antes de la primera orden se procesan los eventos de T=0 */
    if(avanzar_reloj(pJornada, 0) == 0) {
        return 0;
    }

    evaluar_estado(pJornada);

    while(pJornada->resumen.motivoFin == JORNADA_EN_CURSO) {

        printf("\n[T=%d] Ingrese orden: ", pJornada->tiempoActual);

        if(fgets(orden, MAX_BUFFER, stdin) == NULL) {
            printf("Error al leer la orden\n");
            return 0;
        }

        orden[strcspn(orden, "\r\n")] = '\0';

        /* PASO 7: validar la orden. PASO 8: DES, REU, ENT y ESP con su orden temporal */
        if(strcmp(orden, "VER") == 0) {
            mostrar_puerto(pJornada);
        } else if(strcmp(orden, "ESP") == 0) {
            if(avanzar_reloj(pJornada, pJornada->tiempoActual + 1) == 0) {
                return 0;
            }
            evaluar_estado(pJornada);
        } else {
            printf("Orden no reconocida\n");
        }
    }

    printf("\nFin de la jornada en T=%d\n", pJornada->tiempoActual);

    return 1;
}

/* Lleva el reloj a tiempoNuevo y aplica los eventos automaticos: todos los arribos
   con tiempoLlegada <= tiempoNuevo (aunque el salto sea de varios minutos) y la
   asignacion de muelles */
int avanzar_reloj(tJornada* pJornada, int tiempoNuevo) {

    pJornada->tiempoActual = tiempoNuevo;

    if(procesar_arribos(pJornada, tiempoNuevo) == 0) {
        return 0;
    }

    asignar_muelles(&pJornada->puerto);

    return 1;
}

/* Se llama despues de cada cambio de estado. Si la jornada termina, deja el motivo en el resumen */
void evaluar_estado(tJornada* pJornada) {

    tPuerto* pPuerto = &pJornada->puerto;
    int hayEventosFuturos;
    int hayBuquesConCarga;

    hayEventosFuturos = !lista_vacia(&pJornada->escenario.buques) ||
                        !lista_vacia(&pJornada->escenario.camiones);

    /* Un buque atracado siempre tiene carga: el muelle se libera cuando termina */
    hayBuquesConCarga = hay_buque_atracado(pPuerto) || pPuerto->buquesEnEspera > 0;

    if(!hayEventosFuturos && !hayBuquesConCarga && pPuerto->camionesEnEspera == 0) {
        pJornada->resumen.motivoFin = FIN_NORMAL;
    } else if(pJornada->tiempoActual >= pJornada->cfg.duracionJornadaMinutos) {
        pJornada->resumen.motivoFin = FIN_TIEMPO;
    }

    /* PASO 9: falta bloqueo y salto automatico; necesitan saber si hay un DES, REU o ENT valido */
}

void liberar_jornada(tJornada* pJornada) {

    liberar_escenario(&pJornada->escenario);
    liberar_puerto(&pJornada->puerto);

    /* tOperacion no tiene memoria dinamica adentro: alcanza con vaciar_lista */
    vaciar_lista(&pJornada->historial);
}

int crear_puerto(tPuerto* pPuerto, const tConfiguracion* pConfiguracion) {

    int i;
    tZona zona;

    pPuerto->cantMuelles = pConfiguracion->cantidadMuelles;
    pPuerto->muelles     = (tMuelle*)malloc(pPuerto->cantMuelles * sizeof(tMuelle));

    if(pPuerto->muelles == NULL) {
        printf("Error al reservar memoria para los muelles\n");
        pPuerto->cantMuelles = 0;
        return 0;
    }

    for(i = 0; i < pPuerto->cantMuelles; i++) {
        pPuerto->muelles[i].id      = i + 1;
        pPuerto->muelles[i].ocupado = 0;
    }

    crear_cola(&pPuerto->colaBuques);
    crear_cola(&pPuerto->colaCamiones);
    crear_lista(&pPuerto->zonas);

    pPuerto->buquesEnEspera   = 0;
    pPuerto->camionesEnEspera = 0;

    for(i = 0; i < pConfiguracion->cantidadZonasAlmacenamiento; i++) {

        zona.id       = i + 1;
        zona.cantidad = 0;
        crear_pila(&zona.pila);

        if(enlistar_final(&pPuerto->zonas, &zona, sizeof(tZona)) == 0) {
            printf("Error al reservar memoria para las zonas de almacenamiento\n");
            liberar_puerto(pPuerto);
            return 0;
        }
    }

    return 1;
}

void liberar_puerto(tPuerto* pPuerto) {

    int i;
    tBuque buque;
    tZona zona;

    for(i = 0; i < pPuerto->cantMuelles; i++) {
        if(pPuerto->muelles[i].ocupado == 1) {
            free(pPuerto->muelles[i].buque.manifiesto);
        }
    }

    free(pPuerto->muelles);

    pPuerto->muelles     = NULL;
    pPuerto->cantMuelles = 0;

    /* vaciar_cola no libera el manifiesto que esta dentro de cada tBuque */
    while(desencolar(&pPuerto->colaBuques, &buque, sizeof(tBuque)) == 1) {
        free(buque.manifiesto);
    }

    vaciar_cola(&pPuerto->colaCamiones);

    pPuerto->buquesEnEspera   = 0;
    pPuerto->camionesEnEspera = 0;

    /* vaciar_lista no libera los nodos de la pila que esta dentro de cada tZona */
    while(desenlistar_inicio(&pPuerto->zonas, &zona, sizeof(tZona)) == 1) {
        vaciar_pila(&zona.pila);
    }
}

/* Mueve del escenario al puerto todo lo que tenga tiempoLlegada <= tiempoLimite.
   Primero los buques y despues los camiones, cada uno en el orden de puerto.txt */
int procesar_arribos(tJornada* pJornada, int tiempoLimite) {

    tBuque buque;
    tCamion camion;
    tEscenario* pEscenario = &pJornada->escenario;
    tPuerto* pPuerto       = &pJornada->puerto;

    /* Las listas estan ordenadas por tiempoLlegada: alcanza con mirar el primero */
    while(ver_primero_lista(&pEscenario->buques, &buque, sizeof(tBuque)) == 1 &&
          buque.tiempoLlegada <= tiempoLimite) {

        desenlistar_inicio(&pEscenario->buques, &buque, sizeof(tBuque));

        if(arribo_buque(pPuerto, &buque) == 0) {
            /* El buque ya salio de la lista y no entro a la cola: su manifiesto es de esta funcion */
            free(buque.manifiesto);
            printf("Error al reservar memoria para la cola de buques\n");
            return 0;
        }
    }

    while(ver_primero_lista(&pEscenario->camiones, &camion, sizeof(tCamion)) == 1 &&
          camion.tiempoLlegada <= tiempoLimite) {

        desenlistar_inicio(&pEscenario->camiones, &camion, sizeof(tCamion));

        if(encolar(&pPuerto->colaCamiones, &camion, sizeof(tCamion)) == 0) {
            printf("Error al reservar memoria para la cola de camiones\n");
            return 0;
        }

        pPuerto->camionesEnEspera++;
        printf("Arribo %s (pide %s): espera en la cola de camiones\n", camion.id, camion.pedido.codigo);
    }

    return 1;
}

/* Atraca en el muelle libre de menor id; si no hay, va a la cola de espera */
int arribo_buque(tPuerto* pPuerto, const tBuque* pBuque) {

    int i = -1;

    /* Con buques esperando, el que llega va detras aunque haya un muelle libre */
    if(cola_vacia(&pPuerto->colaBuques)) {
        i = buscar_muelle_libre(pPuerto);
    }

    if(i != -1) {
        pPuerto->muelles[i].buque   = *pBuque;
        pPuerto->muelles[i].ocupado = 1;
        printf("Arribo %s: atraca en M%d\n", pBuque->id, pPuerto->muelles[i].id);
        return 1;
    }

    if(encolar(&pPuerto->colaBuques, pBuque, sizeof(tBuque)) == 0) {
        return 0;
    }

    pPuerto->buquesEnEspera++;
    printf("Arribo %s: no hay muelle libre, espera en la cola de buques\n", pBuque->id);

    return 1;
}

/* Pasa buques de la cola a los muelles libres, en orden de llegada y de menor id de muelle */
void asignar_muelles(tPuerto* pPuerto) {

    int i;

    i = buscar_muelle_libre(pPuerto);

    while(i != -1 && desencolar(&pPuerto->colaBuques, &pPuerto->muelles[i].buque, sizeof(tBuque)) == 1) {

        pPuerto->muelles[i].ocupado = 1;
        pPuerto->buquesEnEspera--;
        printf("%s sale de la cola y atraca en M%d\n", pPuerto->muelles[i].buque.id, pPuerto->muelles[i].id);

        i = buscar_muelle_libre(pPuerto);
    }
}

/* Devuelve la posicion del primer muelle libre, o -1 si estan todos ocupados */
int buscar_muelle_libre(const tPuerto* pPuerto) {

    int i;

    for(i = 0; i < pPuerto->cantMuelles; i++) {
        if(pPuerto->muelles[i].ocupado == 0) {
            return i;
        }
    }

    return -1;
}

int hay_buque_atracado(const tPuerto* pPuerto) {

    int i;

    for(i = 0; i < pPuerto->cantMuelles; i++) {
        if(pPuerto->muelles[i].ocupado == 1) {
            return 1;
        }
    }

    return 0;
}

/* Vista simple para probar los arribos; el VER completo es el PASO 6 */
void mostrar_puerto(const tJornada* pJornada) {

    int i;
    const tMuelle* pMuelle;

    printf("T=%d\n", pJornada->tiempoActual);

    for(i = 0; i < pJornada->puerto.cantMuelles; i++) {

        pMuelle = &pJornada->puerto.muelles[i];

        if(pMuelle->ocupado == 1) {
            printf("M%d: %s (proximo: %s, faltan %d)\n", pMuelle->id, pMuelle->buque.id,
                   pMuelle->buque.manifiesto[pMuelle->buque.proximoADescargar].codigo,
                   pMuelle->buque.cantContenedores - pMuelle->buque.proximoADescargar);
        } else {
            printf("M%d: libre\n", pMuelle->id);
        }
    }

    printf("Buques esperando: %d\n", pJornada->puerto.buquesEnEspera);
    printf("Camiones esperando: %d\n", pJornada->puerto.camionesEnEspera);
}
