#include "jornada.h"

/*
Inicializa la jornada: carga el escenario desde puerto.txt, crea el puerto
y pone todos los contadores del resumen en cero.
Devuelve EXITO si todo salio bien, ERROR en caso contrario.
*/
int iniciar_jornada(tJornada* pJornada, const tConfiguracion* pConfiguracion) {

    pJornada->cfg = *pConfiguracion;

    if(cargar_escenario(PUERTO_ARCHIVO, &pJornada->cfg, &pJornada->escenario) == ERROR) {
        return ERROR;
    }

    if(crear_puerto(&pJornada->puerto, &pJornada->cfg) == ERROR) {
        liberar_escenario(&pJornada->escenario);
        return ERROR;
    }

    pJornada->tiempoActual = 0;

    pJornada->resumen.contenedoresEntregados = 0;
    pJornada->resumen.buquesDescargados      = 0;
    pJornada->resumen.camionesPendientes     = 0;
    pJornada->resumen.reubicaciones          = 0;
    pJornada->resumen.tiempoUtilizado        = 0;
    pJornada->resumen.puntuacion             = 0;
    pJornada->resumen.motivoFin              = JORNADA_EN_CURSO;
    pJornada->huboSaltoAutomatico            = 0;
    pJornada->tiempoAntesSalto               = 0;

    crear_lista(&pJornada->historial);

    return EXITO;
}

/*
Bucle principal de la jornada: procesa los eventos de T=0, limpia la pantalla
y muestra la tabla de operaciones. Si en T=0 hubo un salto automatico, lo
informa debajo de la tabla antes del primer prompt.
Luego pide ordenes al operador hasta que la jornada termine (FIN_NORMAL,
FIN_TIEMPO o FIN_BLOQUEO). Tras cada operacion exitosa limpia la pantalla,
vuelve a mostrar la tabla y, si hubo un salto automatico, lo informa.
Al finalizar muestra el historial y el resumen con la puntuacion definitiva.
Devuelve EXITO si termino normalmente, ERROR si hubo un fallo de memoria.
*/
int ejecutar_jornada(tJornada* pJornada) {

    char orden[MAX_BUFFER];
    int c;
    int resultado;

    if(avanzar_reloj(pJornada, 0) == ERROR) {
        return ERROR;
    }
    evaluar_estado(pJornada);

    if(pJornada->resumen.motivoFin == JORNADA_EN_CURSO) {
        system("cls");
        mostrar_ayuda(&pJornada->cfg);
        if(pJornada->huboSaltoAutomatico) {
            printf("\n[Salto automatico: T=%d sin operaciones posibles] Se avanzo a T=%d.\n",
                   pJornada->tiempoAntesSalto, pJornada->tiempoActual);
            pJornada->huboSaltoAutomatico = 0;
        }
    }

    while(pJornada->resumen.motivoFin == JORNADA_EN_CURSO) {

        printf("[T=%d] Ingrese operacion: ", pJornada->tiempoActual);

        if(fgets(orden, MAX_BUFFER, stdin) == NULL) {
            printf("Error al leer la orden\n");
            return ERROR;
        }

        if(strchr(orden, '\n') == NULL) {
            while((c = getchar()) != '\n' && c != EOF);
        }

        orden[strcspn(orden, "\r\n")] = '\0';

        resultado = procesar_orden(pJornada, orden);

        if(resultado == ERROR) {
            return ERROR;
        }

        if(resultado == ORDEN_EJECUTADA && pJornada->resumen.motivoFin == JORNADA_EN_CURSO) {
            printf("\nOperacion realizada. Presione Enter para continuar...");
            getchar();
            system("cls");
            mostrar_ayuda(&pJornada->cfg);
            if(pJornada->huboSaltoAutomatico) {
                printf("\n[Salto automatico: T=%d sin operaciones posibles] Se avanzo a T=%d.\n",
                       pJornada->tiempoAntesSalto, pJornada->tiempoActual);
                pJornada->huboSaltoAutomatico = 0;
            }
        }
    }

    system("cls");

    pJornada->resumen.tiempoUtilizado    = pJornada->tiempoActual;
    pJornada->resumen.camionesPendientes = pJornada->puerto.camionesEnEspera;
    pJornada->resumen.puntuacion         = calcular_puntuacion(pJornada);

    mostrar_historial(&pJornada->historial);
    mostrar_resumen(pJornada);

    printf("Presione Enter para volver al menu principal...");
    getchar();
    system("cls");

    return EXITO;
}

/*
Parsea y ejecuta una linea ingresada por el operador.
Una orden rechazada no es un error: se informa el motivo y devuelve EXITO.
Devuelve ORDEN_EJECUTADA si la operacion se completo, EXITO si fue rechazada,
ERROR si fallo algo grave como falta de memoria.
*/
int procesar_orden(tJornada* pJornada, char* linea) {

    tOperacion operacion;
    int cantParametros;
    int T_inicio;

    cantParametros = parsear_orden(linea, &operacion);

    if(cantParametros == NO_ENCONTRADO) {
        printf("Formato invalido. Use: DES M1 Z1 | REU Z1 Z2 | ENT | ESP | VER\n");
        return EXITO;
    }

    if(validar_orden(pJornada, &operacion, cantParametros) == ERROR) {
        return EXITO;
    }

    if(strcmp(operacion.codigo, "VER") == 0) {
        if(mostrar_puerto(pJornada) == ERROR) {
            return ERROR;
        }

    } else if(strcmp(operacion.codigo, "ESP") == 0) {
        T_inicio = pJornada->tiempoActual;
        operacion.tiempoInicio = T_inicio;
        operacion.tiempoFin    = T_inicio + COSTO_ESP;
        strcpy(operacion.contenedor.codigo, ".");

        if(avanzar_reloj(pJornada, T_inicio + COSTO_ESP) == ERROR) {
            return ERROR;
        }
        enlistar_final(&pJornada->historial, &operacion, sizeof(tOperacion));
        evaluar_estado(pJornada);

    } else if(strcmp(operacion.codigo, "DES") == 0) {
        if(ejecutar_des(pJornada, &operacion) == ERROR) {
            return ERROR;
        }

    } else if(strcmp(operacion.codigo, "REU") == 0) {
        if(ejecutar_reu(pJornada, &operacion) == ERROR) {
            return ERROR;
        }

    } else if(strcmp(operacion.codigo, "ENT") == 0) {
        if(ejecutar_ent(pJornada, &operacion) == ERROR) {
            return ERROR;
        }
    }

    return ORDEN_EJECUTADA;
}

/*
Extrae el codigo de operacion y hasta dos parametros de la linea ingresada.
Devuelve la cantidad de parametros reconocidos (0, 1 o 2), o NO_ENCONTRADO si
el formato es invalido.
Solo analiza la forma: la validez del codigo y de los recursos la determina validar_orden.
*/
int parsear_orden(char* linea, tOperacion* pOperacion) {

    char* pParam1;
    char* pParam2;

    if(strlen(linea) < LARGO_CODIGO || (linea[LARGO_CODIGO] != '\0' && linea[LARGO_CODIGO] != ' ')) {
        return NO_ENCONTRADO;
    }

    strncpy(pOperacion->codigo, linea, LARGO_CODIGO);
    pOperacion->codigo[LARGO_CODIGO] = '\0';
    strcpy(pOperacion->origen, ".");
    strcpy(pOperacion->destino, ".");

    if(linea[LARGO_CODIGO] == '\0') {
        return 0;
    }

    pParam1 = linea + LARGO_CODIGO + 1;
    pParam2 = strchr(pParam1, ' ');

    if(pParam2 != NULL) {
        *pParam2 = '\0';
        pParam2++;
    }

    if(*pParam1 == '\0' || strlen(pParam1) >= MAX_ID) {
        return NO_ENCONTRADO;
    }

    strcpy(pOperacion->origen, pParam1);

    if(pParam2 == NULL) {
        return 1;
    }

    if(*pParam2 == '\0' || strchr(pParam2, ' ') != NULL || strlen(pParam2) >= MAX_ID) {
        return NO_ENCONTRADO;
    }

    strcpy(pOperacion->destino, pParam2);

    return 2;
}

/*
Devuelve la cantidad de parametros que espera el codigo (0 o 2),
o NO_ENCONTRADO si el codigo no existe.
*/
int parametros_esperados(const char* codigo) {

    if(strcmp(codigo, "DES") == 0 || strcmp(codigo, "REU") == 0) {
        return 2;
    }

    if(strcmp(codigo, "ENT") == 0 || strcmp(codigo, "ESP") == 0 || strcmp(codigo, "VER") == 0) {
        return 0;
    }

    return NO_ENCONTRADO;
}

/*
Extrae el numero N de la cadena "MN".
Formato valido: 'M' seguido de uno o mas digitos, sin otros caracteres.
Devuelve el numero si es valido, NO_ENCONTRADO en caso contrario.
*/
int parsear_id_muelle(const char* param) {

    char* fin;
    int numero;

    if(param[0] != 'M') {
        return NO_ENCONTRADO;
    }

    if(!isdigit((unsigned char)param[1])) {
        return NO_ENCONTRADO;
    }

    numero = (int)strtol(param + 1, &fin, BASE_DECIMAL);

    if(*fin != '\0') {
        return NO_ENCONTRADO;
    }

    return numero;
}

/*
Extrae el numero N de la cadena "ZN".
Formato valido: 'Z' seguido de uno o mas digitos, sin otros caracteres.
Devuelve el numero si es valido, NO_ENCONTRADO en caso contrario.
*/
int parsear_id_zona(const char* param) {

    char* fin;
    int numero;

    if(param[0] != 'Z') {
        return NO_ENCONTRADO;
    }

    if(!isdigit((unsigned char)param[1])) {
        return NO_ENCONTRADO;
    }

    numero = (int)strtol(param + 1, &fin, BASE_DECIMAL);

    if(*fin != '\0') {
        return NO_ENCONTRADO;
    }

    return numero;
}

/*
Devuelve el costo en minutos de la operacion indicada por codigo.
VER tiene costo COSTO_VER (cero).
*/
int costo_operacion(const char* codigo, const tConfiguracion* cfg) {

    if(strcmp(codigo, "DES") == 0) return cfg->tiempoDescargaContenedor;
    if(strcmp(codigo, "REU") == 0) return cfg->tiempoReubicacionContenedor;
    if(strcmp(codigo, "ENT") == 0) return cfg->tiempoCargaCamion;
    if(strcmp(codigo, "ESP") == 0) return COSTO_ESP;
    return COSTO_VER;
}

/*
Valida la orden en seis capas: mayusculas, codigo valido, cantidad de parametros,
existencia de recursos, precondiciones del estado actual y tiempo disponible.
Devuelve EXITO si la orden es valida, ERROR si fue rechazada (ya informo el motivo).
*/
int validar_orden(tJornada* pJornada, const tOperacion* pOperacion, int cantParametros) {

    int i;
    int esperados;
    int numMuelle;
    int numZona;
    int numZonaOrigen;
    int numZonaDestino;
    int cantZona;
    int costo;
    tCamion camionFrente;

    for(i = 0; i < LARGO_CODIGO; i++) {
        if(pOperacion->codigo[i] < 'A' || pOperacion->codigo[i] > 'Z') {
            printf("Ingrese la orden en mayusculas\n");
            return ERROR;
        }
    }

    esperados = parametros_esperados(pOperacion->codigo);
    if(esperados == NO_ENCONTRADO) {
        printf("Operacion inexistente: %s\n", pOperacion->codigo);
        return ERROR;
    }

    if(cantParametros != esperados) {
        if(esperados == 0) {
            printf("%s no lleva parametros\n", pOperacion->codigo);
        } else if(strcmp(pOperacion->codigo, "DES") == 0) {
            printf("DES necesita 2 parametros. Ejemplo: DES M1 Z1\n");
        } else {
            printf("REU necesita 2 parametros. Ejemplo: REU Z1 Z2\n");
        }
        return ERROR;
    }

    if(strcmp(pOperacion->codigo, "DES") == 0) {

        numMuelle = parsear_id_muelle(pOperacion->origen);
        if(numMuelle == NO_ENCONTRADO) {
            printf("%s no tiene el formato correcto (use M1, M2, ...)\n", pOperacion->origen);
            return ERROR;
        }
        if(numMuelle < ID_MINIMO || numMuelle > pJornada->cfg.cantidadMuelles) {
            printf("%s no existe (hay M1 a M%d)\n", pOperacion->origen, pJornada->cfg.cantidadMuelles);
            return ERROR;
        }

        numZona = parsear_id_zona(pOperacion->destino);
        if(numZona == NO_ENCONTRADO) {
            printf("%s no tiene el formato correcto (use Z1, Z2, ...)\n", pOperacion->destino);
            return ERROR;
        }
        if(numZona < ID_MINIMO || numZona > pJornada->cfg.cantidadZonasAlmacenamiento) {
            printf("%s no existe (hay Z1 a Z%d)\n", pOperacion->destino, pJornada->cfg.cantidadZonasAlmacenamiento);
            return ERROR;
        }

    } else if(strcmp(pOperacion->codigo, "REU") == 0) {

        numZonaOrigen = parsear_id_zona(pOperacion->origen);
        if(numZonaOrigen == NO_ENCONTRADO) {
            printf("%s no tiene el formato correcto (use Z1, Z2, ...)\n", pOperacion->origen);
            return ERROR;
        }
        if(numZonaOrigen < ID_MINIMO || numZonaOrigen > pJornada->cfg.cantidadZonasAlmacenamiento) {
            printf("%s no existe (hay Z1 a Z%d)\n", pOperacion->origen, pJornada->cfg.cantidadZonasAlmacenamiento);
            return ERROR;
        }

        numZonaDestino = parsear_id_zona(pOperacion->destino);
        if(numZonaDestino == NO_ENCONTRADO) {
            printf("%s no tiene el formato correcto (use Z1, Z2, ...)\n", pOperacion->destino);
            return ERROR;
        }
        if(numZonaDestino < ID_MINIMO || numZonaDestino > pJornada->cfg.cantidadZonasAlmacenamiento) {
            printf("%s no existe (hay Z1 a Z%d)\n", pOperacion->destino, pJornada->cfg.cantidadZonasAlmacenamiento);
            return ERROR;
        }
    }

    if(strcmp(pOperacion->codigo, "DES") == 0) {

        if(!pJornada->puerto.muelles[numMuelle - ID_MINIMO].ocupado) {
            printf("M%d no tiene ningun buque atracado\n", numMuelle);
            return ERROR;
        }

        if(pJornada->puerto.muelles[numMuelle - ID_MINIMO].buque.proximoADescargar >=
           pJornada->puerto.muelles[numMuelle - ID_MINIMO].buque.cantContenedores) {
            printf("El buque en M%d ya descargo todos sus contenedores\n", numMuelle);
            return ERROR;
        }

        cantZona = ver_cantidad_zona(&pJornada->puerto.zonas, numZona);
        if(cantZona >= pJornada->cfg.capacidadPila) {
            printf("Z%d esta llena (%d/%d)\n", numZona, cantZona, pJornada->cfg.capacidadPila);
            return ERROR;
        }

    } else if(strcmp(pOperacion->codigo, "REU") == 0) {

        if(numZonaOrigen == numZonaDestino) {
            printf("Las zonas deben ser distintas\n");
            return ERROR;
        }

        if(ver_cantidad_zona(&pJornada->puerto.zonas, numZonaOrigen) == 0) {
            printf("Z%d esta vacia\n", numZonaOrigen);
            return ERROR;
        }

        cantZona = ver_cantidad_zona(&pJornada->puerto.zonas, numZonaDestino);
        if(cantZona >= pJornada->cfg.capacidadPila) {
            printf("Z%d esta llena (%d/%d)\n", numZonaDestino, cantZona, pJornada->cfg.capacidadPila);
            return ERROR;
        }

    } else if(strcmp(pOperacion->codigo, "ENT") == 0) {

        if(pJornada->puerto.camionesEnEspera == 0) {
            printf("No hay camiones esperando\n");
            return ERROR;
        }

        ver_primero_cola(&pJornada->puerto.colaCamiones, &camionFrente, sizeof(tCamion));
        if(!contenedor_en_algun_tope(&pJornada->puerto.zonas, camionFrente.pedido.codigo)) {
            printf("%s no esta en el tope de ninguna zona\n", camionFrente.pedido.codigo);
            return ERROR;
        }
    }

    costo = costo_operacion(pOperacion->codigo, &pJornada->cfg);
    if(costo > COSTO_VER && pJornada->tiempoActual + costo > pJornada->cfg.duracionJornadaMinutos) {
        printf("No hay tiempo: %s tarda %d min y solo quedan %d\n",
               pOperacion->codigo, costo,
               pJornada->cfg.duracionJornadaMinutos - pJornada->tiempoActual);
        return ERROR;
    }

    return EXITO;
}

/*
Lleva el reloj a tiempoNuevo, procesa todos los arribos de buques y camiones
con tiempoLlegada <= tiempoNuevo y asigna muelles libres.
Devuelve EXITO si todo salio bien, ERROR si hubo fallo de memoria en procesar_arribos.
*/
int avanzar_reloj(tJornada* pJornada, int tiempoNuevo) {

    pJornada->tiempoActual = tiempoNuevo;

    if(procesar_arribos(pJornada, tiempoNuevo) == ERROR) {
        return ERROR;
    }

    asignar_muelles(&pJornada->puerto);

    return EXITO;
}

/*
Devuelve EXITO si hay al menos una DES ejecutable en el estado actual:
algun muelle con buque con contenedores pendientes, alguna zona con espacio
y tiempo suficiente para completar la operacion.
*/
int hay_des_posible(tJornada* pJornada) {

    int i;
    int idZona;
    int hayMuelleConCarga;

    if(pJornada->tiempoActual + pJornada->cfg.tiempoDescargaContenedor >
       pJornada->cfg.duracionJornadaMinutos) {
        return ERROR;
    }

    hayMuelleConCarga = ERROR;
    for(i = 0; i < pJornada->puerto.cantMuelles; i++) {
        if(pJornada->puerto.muelles[i].ocupado &&
           pJornada->puerto.muelles[i].buque.proximoADescargar <
           pJornada->puerto.muelles[i].buque.cantContenedores) {
            hayMuelleConCarga = EXITO;
            break;
        }
    }
    if(!hayMuelleConCarga) return ERROR;

    for(idZona = ID_MINIMO; idZona <= pJornada->cfg.cantidadZonasAlmacenamiento; idZona++) {
        if(ver_cantidad_zona(&pJornada->puerto.zonas, idZona) < pJornada->cfg.capacidadPila) {
            return EXITO;
        }
    }

    return ERROR;
}

/*
Devuelve EXITO si hay al menos una REU ejecutable en el estado actual:
alguna zona origen no vacia y alguna zona destino diferente con espacio,
y tiempo suficiente para completar la operacion.
*/
int hay_reu_posible(tJornada* pJornada) {

    int origen;
    int destino;

    if(pJornada->tiempoActual + pJornada->cfg.tiempoReubicacionContenedor >
       pJornada->cfg.duracionJornadaMinutos) {
        return ERROR;
    }

    for(origen = ID_MINIMO; origen <= pJornada->cfg.cantidadZonasAlmacenamiento; origen++) {
        if(ver_cantidad_zona(&pJornada->puerto.zonas, origen) == 0) continue;
        for(destino = ID_MINIMO; destino <= pJornada->cfg.cantidadZonasAlmacenamiento; destino++) {
            if(destino == origen) continue;
            if(ver_cantidad_zona(&pJornada->puerto.zonas, destino) < pJornada->cfg.capacidadPila) {
                return EXITO;
            }
        }
    }

    return ERROR;
}

/*
Devuelve EXITO si hay al menos una ENT ejecutable en el estado actual:
hay camiones en la cola, el contenedor pedido por el primero esta en el tope
de alguna zona, y hay tiempo suficiente para completar la operacion.
*/
int hay_ent_posible(tJornada* pJornada) {

    tCamion camion;

    if(pJornada->tiempoActual + pJornada->cfg.tiempoCargaCamion >
       pJornada->cfg.duracionJornadaMinutos) {
        return ERROR;
    }

    if(pJornada->puerto.camionesEnEspera == 0) return ERROR;

    ver_primero_cola(&pJornada->puerto.colaCamiones, &camion, sizeof(tCamion));
    return contenedor_en_algun_tope(&pJornada->puerto.zonas, camion.pedido.codigo);
}

/*
Devuelve EXITO si al menos una de DES, REU o ENT es posible en el estado actual.
*/
int hay_operacion_posible(tJornada* pJornada) {

    return hay_des_posible(pJornada) ||
           hay_reu_posible(pJornada) ||
           hay_ent_posible(pJornada);
}

/*
Devuelve el tiempo del proximo arribo pendiente (buque o camion, el menor de los dos).
Solo debe llamarse cuando haya al menos un evento futuro en el escenario.
*/
int proximo_tiempo_arribo(tJornada* pJornada) {

    tBuque  buque;
    tCamion camion;
    int hayBuque;
    int hayCamion;
    int tiempoBuque;
    int tiempoCamion;

    hayBuque  = ver_primero_lista(&pJornada->escenario.buques,   &buque,  sizeof(tBuque))  == EXITO;
    hayCamion = ver_primero_lista(&pJornada->escenario.camiones, &camion, sizeof(tCamion)) == EXITO;

    tiempoBuque  = hayBuque  ? buque.tiempoLlegada  : pJornada->cfg.duracionJornadaMinutos + 1;
    tiempoCamion = hayCamion ? camion.tiempoLlegada : pJornada->cfg.duracionJornadaMinutos + 1;

    return tiempoBuque < tiempoCamion ? tiempoBuque : tiempoCamion;
}

/*
Determina si la jornada termino y por que, o si corresponde hacer un salto
automatico al proximo arribo cuando no hay operaciones posibles.
Orden de verificacion: FIN_NORMAL, FIN_TIEMPO, bloqueo con o sin eventos futuros.
Si hay eventos futuros avanza el reloj recursivamente hasta que haya operaciones
posibles o se alcance un fin definitivo.
Cuando hace un salto, activa huboSaltoAutomatico y guarda tiempoAntesSalto
para que ejecutar_jornada pueda informarlo al operador.
*/
void evaluar_estado(tJornada* pJornada) {

    tPuerto* pPuerto = &pJornada->puerto;
    int hayEventosFuturos;
    int hayBuquesConCarga;
    int proximoArribo;

    hayBuquesConCarga = hay_buque_atracado(pPuerto) || pPuerto->buquesEnEspera > 0;

    hayEventosFuturos = !lista_vacia(&pJornada->escenario.buques) ||
                        !lista_vacia(&pJornada->escenario.camiones);

    if(!hayEventosFuturos && !hayBuquesConCarga && pPuerto->camionesEnEspera == 0) {
        pJornada->resumen.motivoFin = FIN_NORMAL;
        return;
    }

    if(pJornada->tiempoActual >= pJornada->cfg.duracionJornadaMinutos) {
        pJornada->resumen.motivoFin = FIN_TIEMPO;
        return;
    }

    if(!hay_operacion_posible(pJornada)) {
        if(!hayEventosFuturos) {
            pJornada->resumen.motivoFin = FIN_BLOQUEO;
        } else {
            proximoArribo = proximo_tiempo_arribo(pJornada);

            pJornada->tiempoAntesSalto    = pJornada->tiempoActual;
            pJornada->huboSaltoAutomatico = 1;

            if(avanzar_reloj(pJornada, proximoArribo) == ERROR) {
                pJornada->resumen.motivoFin = FIN_BLOQUEO;
                return;
            }
            evaluar_estado(pJornada);
        }
    }
}

/*
Libera todos los recursos dinamicos de la jornada: escenario, puerto e historial.
*/
void liberar_jornada(tJornada* pJornada) {

    liberar_escenario(&pJornada->escenario);
    liberar_puerto(&pJornada->puerto);
    vaciar_lista(&pJornada->historial);
}

/*
Reserva memoria para los muelles y crea la cola de buques, la cola de camiones
y la lista de zonas con sus pilas.
Devuelve EXITO si todo salio bien, ERROR si fallo malloc.
*/
int crear_puerto(tPuerto* pPuerto, const tConfiguracion* pConfiguracion) {

    int i;
    tZona zona;

    pPuerto->cantMuelles = pConfiguracion->cantidadMuelles;
    pPuerto->muelles     = (tMuelle*)malloc(pPuerto->cantMuelles * sizeof(tMuelle));

    if(pPuerto->muelles == NULL) {
        printf("Error al reservar memoria para los muelles\n");
        pPuerto->cantMuelles = 0;
        return ERROR;
    }

    for(i = 0; i < pPuerto->cantMuelles; i++) {
        pPuerto->muelles[i].id      = i + 1;
        pPuerto->muelles[i].ocupado = LIBRE;
    }

    crear_cola(&pPuerto->colaBuques);
    crear_cola(&pPuerto->colaCamiones);
    crear_lista(&pPuerto->zonas);

    pPuerto->buquesEnEspera   = 0;
    pPuerto->camionesEnEspera = 0;

    for(i = 0; i < pConfiguracion->cantidadZonasAlmacenamiento; i++) {
        zona.id        = i + 1;
        zona.cantidad  = 0;
        zona.capacidad = pConfiguracion->capacidadPila;
        crear_pila(&zona.pila);
        enlistar_final(&pPuerto->zonas, &zona, sizeof(tZona));
    }

    return EXITO;
}

/*
Libera todos los recursos dinamicos del puerto: manifiestos de buques atracados,
buques en la cola de espera, camiones, zonas de almacenamiento y el array de muelles.
*/
void liberar_puerto(tPuerto* pPuerto) {

    int i;
    tBuque buque;
    tZona zona;

    for(i = 0; i < pPuerto->cantMuelles; i++) {
        if(pPuerto->muelles[i].ocupado == OCUPADO) {
            free(pPuerto->muelles[i].buque.manifiesto);
        }
    }

    free(pPuerto->muelles);
    pPuerto->muelles     = NULL;
    pPuerto->cantMuelles = 0;

    while(!cola_vacia(&pPuerto->colaBuques)) {
        desencolar(&pPuerto->colaBuques, &buque, sizeof(tBuque));
        free(buque.manifiesto);
    }

    vaciar_cola(&pPuerto->colaCamiones);

    pPuerto->buquesEnEspera   = 0;
    pPuerto->camionesEnEspera = 0;

    while(!lista_vacia(&pPuerto->zonas)) {
        desenlistar_inicio(&pPuerto->zonas, &zona, sizeof(tZona));
        vaciar_pila(&zona.pila);
    }
}

/*
Mueve del escenario al puerto todos los buques y camiones con tiempoLlegada <= tiempoLimite.
Los buques se encolan o atracan segun la disponibilidad de muelles.
Devuelve EXITO si todo salio bien, ERROR si fallo un malloc interno.
*/
int procesar_arribos(tJornada* pJornada, int tiempoLimite) {

    tBuque buque;
    tCamion camion;
    tEscenario* pEscenario = &pJornada->escenario;
    tPuerto* pPuerto       = &pJornada->puerto;

    while(ver_primero_lista(&pEscenario->buques, &buque, sizeof(tBuque)) == EXITO &&
          buque.tiempoLlegada <= tiempoLimite) {

        desenlistar_inicio(&pEscenario->buques, &buque, sizeof(tBuque));

        if(arribo_buque(pPuerto, &buque) == ERROR) {
            free(buque.manifiesto);
            printf("Error al reservar memoria para la cola de buques\n");
            return ERROR;
        }
    }

    while(ver_primero_lista(&pEscenario->camiones, &camion, sizeof(tCamion)) == EXITO &&
          camion.tiempoLlegada <= tiempoLimite) {

        desenlistar_inicio(&pEscenario->camiones, &camion, sizeof(tCamion));
        encolar(&pPuerto->colaCamiones, &camion, sizeof(tCamion));
        pPuerto->camionesEnEspera++;
        printf("Arribo %s (pide %s): espera en la cola de camiones\n", camion.id, camion.pedido.codigo);
    }

    return EXITO;
}

/*
Intenta atracar el buque en el muelle libre de menor id.
Si hay buques esperando en la cola o no hay muelle libre, encola el buque.
Devuelve EXITO si el buque fue atracado o encolado, ERROR si fallo encolar.
*/
int arribo_buque(tPuerto* pPuerto, const tBuque* pBuque) {

    int i = NO_ENCONTRADO;

    if(cola_vacia(&pPuerto->colaBuques)) {
        i = buscar_muelle_libre(pPuerto);
    }

    if(i != NO_ENCONTRADO) {
        pPuerto->muelles[i].buque   = *pBuque;
        pPuerto->muelles[i].ocupado = OCUPADO;
        printf("Arribo %s: atraca en M%d\n", pBuque->id, pPuerto->muelles[i].id);
        return EXITO;
    }

    encolar(&pPuerto->colaBuques, pBuque, sizeof(tBuque));
    pPuerto->buquesEnEspera++;
    printf("Arribo %s: no hay muelle libre, espera en la cola de buques\n", pBuque->id);

    return EXITO;
}

/*
Asigna muelles libres a los buques de la cola, en orden de llegada y
de menor id de muelle disponible.
*/
void asignar_muelles(tPuerto* pPuerto) {

    int i;

    i = buscar_muelle_libre(pPuerto);

    while(i != NO_ENCONTRADO && !cola_vacia(&pPuerto->colaBuques)) {
        desencolar(&pPuerto->colaBuques, &pPuerto->muelles[i].buque, sizeof(tBuque));
        pPuerto->muelles[i].ocupado = OCUPADO;
        pPuerto->buquesEnEspera--;
        printf("%s sale de la cola y atraca en M%d\n", pPuerto->muelles[i].buque.id, pPuerto->muelles[i].id);
        i = buscar_muelle_libre(pPuerto);
    }
}

/*
Devuelve el indice del primer muelle libre, o NO_ENCONTRADO si todos estan ocupados.
*/
int buscar_muelle_libre(const tPuerto* pPuerto) {

    int i;

    for(i = 0; i < pPuerto->cantMuelles; i++) {
        if(pPuerto->muelles[i].ocupado == LIBRE) {
            return i;
        }
    }

    return NO_ENCONTRADO;
}

/*
Devuelve EXITO si hay al menos un muelle ocupado, ERROR si todos estan libres.
*/
int hay_buque_atracado(const tPuerto* pPuerto) {

    int i;

    for(i = 0; i < pPuerto->cantMuelles; i++) {
        if(pPuerto->muelles[i].ocupado == OCUPADO) {
            return EXITO;
        }
    }

    return ERROR;
}

/*
Imprime una linea del recuadro con texto alineado a la izquierda hasta ANCHO_RECUADRO.
*/
void linea_recuadro(const char* texto) {

    printf("| %-*s |\n", ANCHO_RECUADRO, texto);
}

/*
Imprime una linea separadora del recuadro del tipo "#===...===#" o "|---...---|".
*/
void separador_recuadro(char extremo, char relleno) {

    int i;

    printf("%c", extremo);
    for(i = 0; i < ANCHO_RECUADRO + 2; i++) {
        printf("%c", relleno);
    }
    printf("%c\n", extremo);
}

/*
Muestra el recuadro de ayuda con todas las operaciones disponibles, sus costos
y los limites del puerto (muelles, zonas, capacidad, tiempo de fin).
*/
void mostrar_ayuda(const tConfiguracion* pConfiguracion) {

    char texto[MAX_BUFFER];

    printf("\n");
    separador_recuadro('#', '=');
    linea_recuadro("                   OPERACIONES DEL OPERADOR DEL PUERTO");
    separador_recuadro('#', '=');

    sprintf(texto, "%-58sCosto: %3d min", "DES Mx Zy  Descargar", pConfiguracion->tiempoDescargaContenedor);
    linea_recuadro(texto);
    linea_recuadro("           Toma el proximo contenedor del buque atracado en el muelle Mx");
    linea_recuadro("           y lo apila en la zona Zy. Ej: DES M1 Z2");
    separador_recuadro('|', '-');

    sprintf(texto, "%-58sCosto: %3d min", "REU Zx Zy  Reubicar", pConfiguracion->tiempoReubicacionContenedor);
    linea_recuadro(texto);
    linea_recuadro("           Mueve el contenedor del tope de la zona Zx al tope de la");
    linea_recuadro("           zona Zy. Las zonas deben ser distintas. Ej: REU Z1 Z3");
    separador_recuadro('|', '-');

    sprintf(texto, "%-58sCosto: %3d min", "ENT        Entregar", pConfiguracion->tiempoCargaCamion);
    linea_recuadro(texto);
    linea_recuadro("           Entrega al camion del frente de la cola su contenedor. Solo");
    linea_recuadro("           se puede si ese contenedor esta en el tope de alguna zona.");
    separador_recuadro('|', '-');

    sprintf(texto, "%-58sCosto: %3d min", "ESP        Esperar", COSTO_ESP);
    linea_recuadro(texto);
    linea_recuadro("           Avanza el reloj un minuto sin hacer otra operacion.");
    separador_recuadro('|', '-');

    sprintf(texto, "%-58sCosto: %3d min", "VER        Ver estado", COSTO_VER);
    linea_recuadro(texto);
    linea_recuadro("           Muestra muelles, zonas, camiones y puntuacion.");
    separador_recuadro('#', '=');

    sprintf(texto, "Muelles: M1 a M%d   Zonas: Z1 a Z%d   Capacidad por zona: %d   Fin: T=%d",
            pConfiguracion->cantidadMuelles, pConfiguracion->cantidadZonasAlmacenamiento,
            pConfiguracion->capacidadPila, pConfiguracion->duracionJornadaMinutos);
    linea_recuadro(texto);
    separador_recuadro('#', '=');
}

/*
Implementa la operacion VER: muestra el estado completo del puerto (muelles,
zonas, camiones en cola y puntuacion provisoria).
*/
int mostrar_puerto(tJornada* pJornada) {

    int i;

    printf("\n#========================= ESTADO DEL PUERTO =========================#\n");
    printf("  Tiempo: T=%d de %d                     (quedan %d minutos)\n",
           pJornada->tiempoActual, pJornada->cfg.duracionJornadaMinutos,
           pJornada->cfg.duracionJornadaMinutos - pJornada->tiempoActual);
    printf("#---------------------------------------------------------------------#\n");

    printf("  MUELLES\n");
    for(i = 0; i < pJornada->puerto.cantMuelles; i++) {
        mostrar_muelle(&pJornada->puerto.muelles[i]);
    }
    printf("\n  Buques esperando: %d\n", pJornada->puerto.buquesEnEspera);
    printf("#---------------------------------------------------------------------#\n");

    printf("  ZONAS (capacidad %d)                       base ............ tope\n", pJornada->cfg.capacidadPila);
    mostrar_zonas(&pJornada->puerto.zonas);
    printf("#---------------------------------------------------------------------#\n");

    printf("  CAMIONES (ventana de planificacion)\n");
    mostrar_ventana_camiones(&pJornada->puerto.colaCamiones, pJornada->puerto.camionesEnEspera);
    printf("#---------------------------------------------------------------------#\n");

    printf("  Entregados: %d   Buques descargados: %d   Camiones en cola: %d\n",
           pJornada->resumen.contenedoresEntregados, pJornada->resumen.buquesDescargados,
           pJornada->puerto.camionesEnEspera);
    printf("  Puntuacion provisoria: %dx%d + %dx%d - %dx%d = %d\n",
           PUNTOS_ENTREGA,   pJornada->resumen.contenedoresEntregados,
           PUNTOS_BUQUE,     pJornada->resumen.buquesDescargados,
           PENALIDAD_CAMION, pJornada->puerto.camionesEnEspera,
           calcular_puntuacion(pJornada));
    printf("#=====================================================================#\n");

    return EXITO;
}

/*
Muestra el estado de un muelle: libre o el buque atracado con su manifiesto
y la cantidad de contenedores pendientes de descargar.
*/
void mostrar_muelle(const tMuelle* pMuelle) {

    int i;
    int faltan;
    const tBuque* pBuque = &pMuelle->buque;

    if(pMuelle->ocupado == LIBRE) {
        printf("    M%d: libre\n", pMuelle->id);
        return;
    }

    printf("    M%d: %-6s proximo -> [%s]", pMuelle->id, pBuque->id,
           pBuque->manifiesto[pBuque->proximoADescargar].codigo);

    for(i = pBuque->proximoADescargar + 1; i < pBuque->cantContenedores; i++) {
        printf(" %s", pBuque->manifiesto[i].codigo);
    }

    faltan = pBuque->cantContenedores - pBuque->proximoADescargar;

    if(faltan == UN_CONTENEDOR) {
        printf("   (falta 1 de %d)\n", pBuque->cantContenedores);
    } else {
        printf("   (faltan %d de %d)\n", faltan, pBuque->cantContenedores);
    }
}

/*
Accion para recorrer_lista: muestra el contenido de una zona usando una pila auxiliar
para invertir el orden sin modificar la zona original.
*/
void accion_mostrar_zona(void* dato) {

    tZona* pZona = (tZona*)dato;
    tContenedor contenedor;
    tPila aux;

    crear_pila(&aux);

    printf("    Z%d (%d/%d): ", pZona->id, pZona->cantidad, pZona->capacidad);

    if(pila_vacia(&pZona->pila)) {
        printf(" vacia\n");
        vaciar_pila(&aux);
        return;
    }

    while(!pila_vacia(&pZona->pila)) {
        desapilar(&pZona->pila, &contenedor, sizeof(tContenedor));
        apilar(&aux, &contenedor, sizeof(tContenedor));
    }

    while(!pila_vacia(&aux)) {
        desapilar(&aux, &contenedor, sizeof(tContenedor));
        printf(" %s", contenedor.codigo);
        apilar(&pZona->pila, &contenedor, sizeof(tContenedor));
    }

    printf("   <- tope");
    if(pZona->cantidad == pZona->capacidad) {
        printf("   LLENA");
    }
    printf("\n");

    vaciar_pila(&aux);
}

/*
Muestra todas las zonas del puerto recorriendolas con accion_mostrar_zona.
*/
void mostrar_zonas(tLista* pZonas) {

    recorrer_lista(pZonas, accion_mostrar_zona);
}

/*
Muestra los primeros VENTANA_CAMIONES camiones de la cola rotando temporalmente.
La cola queda en el mismo orden al terminar.
*/
int mostrar_ventana_camiones(tCola* pCola, int cantCamiones) {

    int i;
    tCamion camion;

    if(cantCamiones == 0) {
        printf("    (no hay camiones esperando)\n");
        return EXITO;
    }

    for(i = 0; i < cantCamiones; i++) {
        desencolar(pCola, &camion, sizeof(tCamion));

        if(i < VENTANA_CAMIONES) {
            printf("    %d) %s --> Solicita %s", i + 1, camion.id, camion.pedido.codigo);
            if(i == FRENTE_COLA) {
                printf("   <-- Frente");
            }
            printf("\n");
        }

        encolar(pCola, &camion, sizeof(tCamion));
    }

    if(cantCamiones > VENTANA_CAMIONES) {
        printf("    (+%d camion(es) mas esperando)\n", cantCamiones - VENTANA_CAMIONES);
    }

    return EXITO;
}

/*
Comparador para buscar_en_lista: compara una tZona por su id.
La clave es un puntero a int.
*/
int comparar_zona_por_id(const void* dato, const void* clave) {

    return ((const tZona*)dato)->id - *(const int*)clave;
}

/*
Comparador para buscar_en_lista: compara el codigo del contenedor en el tope
de la zona con la clave (puntero a char).
Devuelve 0 si el tope coincide, distinto de 0 si no (o si la zona esta vacia).
*/
int comparar_zona_por_tope(const void* dato, const void* clave) {

    tZona* pZona = (tZona*)dato;
    tContenedor tope;

    if(pila_vacia(&pZona->pila)) {
        return 1;
    }

    ver_tope(&pZona->pila, &tope, sizeof(tContenedor));
    return strcmp(tope.codigo, (const char*)clave);
}

/*
Desapila el contenedor del tope de la zona con el id indicado.
validar_orden ya verifico que la zona no esta vacia.
*/
int desapilar_de_zona(tLista* pZonas, int idZona, tContenedor* pContenedor) {

    tZona* pZona = (tZona*)buscar_en_lista(pZonas, &idZona, comparar_zona_por_id);
    desapilar(&pZona->pila, pContenedor, sizeof(tContenedor));
    pZona->cantidad--;

    return EXITO;
}

/*
Apila un contenedor en la zona con el id indicado y actualiza zona.cantidad.
validar_orden ya verifico que la zona tiene espacio libre.
*/
int apilar_en_zona(tLista* pZonas, int idZona, const tContenedor* pContenedor) {

    tZona* pZona = (tZona*)buscar_en_lista(pZonas, &idZona, comparar_zona_por_id);
    apilar(&pZona->pila, pContenedor, sizeof(tContenedor));
    pZona->cantidad++;

    return EXITO;
}

/*
Devuelve la cantidad de contenedores de la zona con el id indicado.
*/
int ver_cantidad_zona(tLista* pZonas, int idZona) {

    tZona* pZona = (tZona*)buscar_en_lista(pZonas, &idZona, comparar_zona_por_id);
    return pZona != NULL ? pZona->cantidad : 0;
}

/*
Devuelve el id de la zona que tenga el contenedor indicado en su tope,
o NO_ENCONTRADO si ninguna zona lo tiene en el tope.
*/
int buscar_zona_con_tope(tLista* pZonas, const char* codigoCont) {

    tZona* pZona = (tZona*)buscar_en_lista(pZonas, codigoCont, comparar_zona_por_tope);
    return pZona != NULL ? pZona->id : NO_ENCONTRADO;
}

/*
Devuelve EXITO si el contenedor con el codigo dado esta en el tope de alguna zona,
ERROR si no esta accesible en ninguna.
*/
int contenedor_en_algun_tope(tLista* pZonas, const char* codigoCont) {

    return buscar_en_lista(pZonas, codigoCont, comparar_zona_por_tope) != NULL;
}

/*
DES Mx Zy: informa el proximo contenedor del buque, procesa arribos durante
la operacion, descarga el contenedor al tope de la zona Zy, libera el muelle
si el buque termino, y registra la operacion en el historial.
Devuelve EXITO si todo salio bien, ERROR si fallo un malloc interno.
*/
int ejecutar_des(tJornada* pJornada, const tOperacion* pOperacion) {

    int T_inicio;
    int T_fin;
    int numMuelle;
    int numZona;
    tMuelle* pMuelle;
    tContenedor contenedor;
    tOperacion registro;
    char idBuque[MAX_ID];

    T_inicio  = pJornada->tiempoActual;
    T_fin     = T_inicio + pJornada->cfg.tiempoDescargaContenedor;
    numMuelle = parsear_id_muelle(pOperacion->origen);
    numZona   = parsear_id_zona(pOperacion->destino);
    pMuelle   = &pJornada->puerto.muelles[numMuelle - ID_MINIMO];

    printf("Proximo contenedor de %s en M%d: %s\n",
           pMuelle->buque.id, numMuelle,
           pMuelle->buque.manifiesto[pMuelle->buque.proximoADescargar].codigo);

    if(T_fin - T_inicio > DURACION_MINIMA_OP) {
        if(procesar_arribos(pJornada, T_fin - 1) == ERROR) return ERROR;
        asignar_muelles(&pJornada->puerto);
    }

    contenedor = pMuelle->buque.manifiesto[pMuelle->buque.proximoADescargar];
    pMuelle->buque.proximoADescargar++;

    apilar_en_zona(&pJornada->puerto.zonas, numZona, &contenedor);

    strcpy(idBuque, pMuelle->buque.id);
    if(pMuelle->buque.proximoADescargar >= pMuelle->buque.cantContenedores) {
        free(pMuelle->buque.manifiesto);
        pMuelle->buque.manifiesto = NULL;
        pMuelle->ocupado = LIBRE;
        pJornada->resumen.buquesDescargados++;
        printf("%s termino de descargarse: M%d queda libre\n", idBuque, numMuelle);
    }

    printf("DES: %s de %s (M%d) -> Z%d\n", contenedor.codigo, idBuque, numMuelle, numZona);

    if(procesar_arribos(pJornada, T_fin) == ERROR) return ERROR;
    asignar_muelles(&pJornada->puerto);

    registro.tiempoInicio = T_inicio;
    registro.tiempoFin    = T_fin;
    strcpy(registro.codigo, "DES");
    registro.contenedor   = contenedor;
    sprintf(registro.origen,  "M%d", numMuelle);
    sprintf(registro.destino, "Z%d", numZona);

    pJornada->tiempoActual = T_fin;

    enlistar_final(&pJornada->historial, &registro, sizeof(tOperacion));

    evaluar_estado(pJornada);

    return EXITO;
}

/*
REU Zx Zy: mueve el contenedor del tope de la zona Zx al tope de la zona Zy,
procesando los arribos que ocurran durante la operacion.
Devuelve EXITO si todo salio bien, ERROR si fallo un malloc interno.
*/
int ejecutar_reu(tJornada* pJornada, const tOperacion* pOperacion) {

    int T_inicio;
    int T_fin;
    int idOrigen;
    int idDestino;
    tContenedor contenedor;
    tOperacion registro;

    T_inicio  = pJornada->tiempoActual;
    T_fin     = T_inicio + pJornada->cfg.tiempoReubicacionContenedor;
    idOrigen  = parsear_id_zona(pOperacion->origen);
    idDestino = parsear_id_zona(pOperacion->destino);

    if(T_fin - T_inicio > DURACION_MINIMA_OP) {
        if(procesar_arribos(pJornada, T_fin - 1) == ERROR) return ERROR;
        asignar_muelles(&pJornada->puerto);
    }

    desapilar_de_zona(&pJornada->puerto.zonas, idOrigen, &contenedor);
    apilar_en_zona(&pJornada->puerto.zonas, idDestino, &contenedor);

    pJornada->resumen.reubicaciones++;
    printf("REU: %s de Z%d -> Z%d\n", contenedor.codigo, idOrigen, idDestino);

    if(procesar_arribos(pJornada, T_fin) == ERROR) return ERROR;
    asignar_muelles(&pJornada->puerto);

    registro.tiempoInicio = T_inicio;
    registro.tiempoFin    = T_fin;
    strcpy(registro.codigo, "REU");
    registro.contenedor   = contenedor;
    sprintf(registro.origen,  "Z%d", idOrigen);
    sprintf(registro.destino, "Z%d", idDestino);

    pJornada->tiempoActual = T_fin;

    enlistar_final(&pJornada->historial, &registro, sizeof(tOperacion));

    evaluar_estado(pJornada);

    return EXITO;
}

/*
ENT: busca el contenedor pedido por el primer camion de la cola en el tope de
alguna zona, lo desapila, despacha el camion y registra la operacion en el historial.
Devuelve EXITO si todo salio bien, ERROR si fallo un malloc interno.
*/
int ejecutar_ent(tJornada* pJornada, const tOperacion* pOperacion) {

    int T_inicio;
    int T_fin;
    int idZona;
    tCamion camion;
    tContenedor contenedor;
    tOperacion registro;

    T_inicio = pJornada->tiempoActual;
    T_fin    = T_inicio + pJornada->cfg.tiempoCargaCamion;

    if(T_fin - T_inicio > DURACION_MINIMA_OP) {
        if(procesar_arribos(pJornada, T_fin - 1) == ERROR) return ERROR;
        asignar_muelles(&pJornada->puerto);
    }

    ver_primero_cola(&pJornada->puerto.colaCamiones, &camion, sizeof(tCamion));
    idZona = buscar_zona_con_tope(&pJornada->puerto.zonas, camion.pedido.codigo);

    desapilar_de_zona(&pJornada->puerto.zonas, idZona, &contenedor);
    desencolar(&pJornada->puerto.colaCamiones, &camion, sizeof(tCamion));
    pJornada->puerto.camionesEnEspera--;
    pJornada->resumen.contenedoresEntregados++;

    printf("ENT: %s entregado a %s desde Z%d\n", contenedor.codigo, camion.id, idZona);

    if(procesar_arribos(pJornada, T_fin) == ERROR) return ERROR;
    asignar_muelles(&pJornada->puerto);

    registro.tiempoInicio = T_inicio;
    registro.tiempoFin    = T_fin;
    strcpy(registro.codigo, "ENT");
    registro.contenedor   = contenedor;
    sprintf(registro.origen,  "Z%d", idZona);
    strcpy(registro.destino, camion.id);

    pJornada->tiempoActual = T_fin;

    enlistar_final(&pJornada->historial, &registro, sizeof(tOperacion));

    evaluar_estado(pJornada);

    return EXITO;
}

/*
Calcula la puntuacion de la jornada:
PUNTOS_ENTREGA por contenedor entregado + PUNTOS_BUQUE por buque descargado
- PENALIDAD_CAMION por camion que quedo en cola al cierre.
*/
int calcular_puntuacion(const tJornada* pJornada) {

    return PUNTOS_ENTREGA   * pJornada->resumen.contenedoresEntregados +
           PUNTOS_BUQUE     * pJornada->resumen.buquesDescargados -
           PENALIDAD_CAMION * pJornada->puerto.camionesEnEspera;
}

/*
Accion para recorrer_lista: imprime una fila del historial con el intervalo
de tiempo, el codigo, el contenedor y el origen/destino de la operacion.
*/
void accion_mostrar_operacion(void* dato) {

    tOperacion* pOp = (tOperacion*)dato;

    if(strcmp(pOp->codigo, "ESP") == 0) {
        printf("  T=%3d->T=%3d | %-3s | %-10s | %-6s -> %s\n",
               pOp->tiempoInicio, pOp->tiempoFin,
               pOp->codigo,
               "-",
               "-", "-");
    } else {
        printf("  T=%3d->T=%3d | %-3s | %-10s | %-6s -> %s\n",
               pOp->tiempoInicio, pOp->tiempoFin,
               pOp->codigo,
               pOp->contenedor.codigo,
               pOp->origen,
               pOp->destino);
    }
}

/*
Muestra el historial completo de operaciones en orden cronologico.
Se llama al cerrar la jornada, antes de mostrar_resumen.
*/
void mostrar_historial(tHistorial* pHistorial) {

    printf("\n");
    separador_recuadro('#', '=');
    linea_recuadro("  HISTORIAL DE OPERACIONES");
    separador_recuadro('#', '=');
    printf("  %-14s | %-3s | %-10s | %s\n", "Intervalo", "Op", "Contenedor", "Origen -> Destino");
    separador_recuadro('|', '-');

    if(lista_vacia(pHistorial)) {
        linea_recuadro("  (ninguna operacion ejecutada)");
    } else {
        recorrer_lista(pHistorial, accion_mostrar_operacion);
    }

    separador_recuadro('#', '=');
}

/*
Muestra el resumen final de la jornada: motivo de cierre, tiempo utilizado,
estadisticas de la partida y puntuacion definitiva con la formula completa.
*/
void mostrar_resumen(const tJornada* pJornada) {

    const tResumen* r = &pJornada->resumen;
    const char* motivo;
    char texto[MAX_BUFFER];

    switch(r->motivoFin) {
        case FIN_NORMAL:
            motivo = "Jornada completada sin pendientes";
            break;
        case FIN_TIEMPO:
            motivo = "Se agoto el tiempo disponible";
            break;
        case FIN_BLOQUEO:
            motivo = "Bloqueo: no habia operaciones posibles";
            break;
        default:
            motivo = "(motivo desconocido)";
            break;
    }

    printf("\n");
    switch(r->motivoFin) {
        case FIN_NORMAL:
            printf(">>> JORNADA FINALIZADA EXITOSAMENTE <<<\n");
            break;
        case FIN_TIEMPO:
            printf(">>> JORNADA FINALIZADA: TIEMPO AGOTADO <<<\n");
            break;
        case FIN_BLOQUEO:
            printf(">>> JORNADA FINALIZADA: SIN OPERACIONES POSIBLES <<<\n");
            break;
        default:
            break;
    }

    printf("\n");
    separador_recuadro('#', '=');
    linea_recuadro("  RESUMEN FINAL");
    separador_recuadro('#', '=');

    sprintf(texto, "Motivo de fin    : %s", motivo);
    linea_recuadro(texto);

    sprintf(texto, "Tiempo utilizado : T=%d de %d minutos",
            r->tiempoUtilizado, pJornada->cfg.duracionJornadaMinutos);
    linea_recuadro(texto);

    separador_recuadro('|', '-');

    sprintf(texto, "Contenedores entregados  : %d", r->contenedoresEntregados);
    linea_recuadro(texto);

    sprintf(texto, "Buques descargados       : %d", r->buquesDescargados);
    linea_recuadro(texto);

    sprintf(texto, "Camiones pendientes      : %d", r->camionesPendientes);
    linea_recuadro(texto);

    sprintf(texto, "Reubicaciones realizadas : %d", r->reubicaciones);
    linea_recuadro(texto);

    separador_recuadro('|', '-');

    sprintf(texto, "Puntuacion: %dx%d + %dx%d - %dx%d = %d",
            PUNTOS_ENTREGA,   r->contenedoresEntregados,
            PUNTOS_BUQUE,     r->buquesDescargados,
            PENALIDAD_CAMION, r->camionesPendientes,
            r->puntuacion);
    linea_recuadro(texto);

    separador_recuadro('#', '=');
    printf("\n");
}
