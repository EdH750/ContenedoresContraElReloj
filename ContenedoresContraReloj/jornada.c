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
    int c;
    int resultado;

    /* Antes del primer turno se procesan los eventos de T=0 (puede haber
       arribes o saltos automaticos). Los mensajes que genere se veran
       brevemente antes de que se limpie la pantalla */
    if(avanzar_reloj(pJornada, 0) == 0) {
        return 0;
    }
    evaluar_estado(pJornada);

    /* Mostrar la ayuda una sola vez al inicio (pantalla limpia).
       Si la jornada ya termino en T=0 (caso extremo) se saltea el bucle */
    if(pJornada->resumen.motivoFin == JORNADA_EN_CURSO) {
        system("cls");
        mostrar_ayuda(&pJornada->cfg);
    }

    while(pJornada->resumen.motivoFin == JORNADA_EN_CURSO) {

        printf("[T=%d] Ingrese operacion: ", pJornada->tiempoActual);

        if(fgets(orden, MAX_BUFFER, stdin) == NULL) {
            printf("Error al leer la orden\n");
            return 0;
        }

        /* Si la linea no entro en el buffer falta el '\n': descartar el resto */
        if(strchr(orden, '\n') == NULL) {
            while((c = getchar()) != '\n' && c != EOF);
        }

        orden[strcspn(orden, "\r\n")] = '\0';

        resultado = procesar_orden(pJornada, orden);

        if(resultado == 0) {
            /* Error grave de memoria: salir del bucle */
            return 0;
        }

        if(resultado == 2 && pJornada->resumen.motivoFin == JORNADA_EN_CURSO) {
            /* Operacion valida y la jornada sigue: pausar para que el operario
               lea el resultado, luego limpiar y volver a mostrar la ayuda */
            printf("\nOperacion realizada. Presione Enter para continuar...");
            getchar();
            system("cls");
            mostrar_ayuda(&pJornada->cfg);
        }
        /* Si resultado == 1 (rechazada): el error ya se mostro en pantalla,
           no se limpia para que el operario lo vea y corrija.
           Si resultado == 2 pero la jornada termino: se cae del bucle
           directamente al cierre (no hace falta otra pausa aqui). */
    }

    /* --- Paso 10: completar campos del resumen y mostrar cierre ---
       Limpiar la pantalla antes del historial para presentarlo ordenado */
    system("cls");

    /* tiempoUtilizado y camionesPendientes se conocen recien al terminar.
       puntuacion se calcula antes de liberar el puerto (que borraria
       camionesEnEspera). El orden importa: calcular primero, mostrar despues */
    pJornada->resumen.tiempoUtilizado    = pJornada->tiempoActual;
    pJornada->resumen.camionesPendientes = pJornada->puerto.camionesEnEspera;
    pJornada->resumen.puntuacion         = calcular_puntuacion(pJornada);

    mostrar_historial(&pJornada->historial);
    mostrar_resumen(pJornada);

    /* Pausa final: el operario lee el resumen y luego vuelve al menu */
    printf("Presione Enter para volver al menu principal...");
    getchar();
    system("cls");

    return 1;
}

/* Envoltorio de una orden del operario: parsear, validar y ejecutar.
   Una orden rechazada no es un error: se informa el motivo y devuelve 1.
   Devuelve 0 solo si falla algo grave (memoria) */
int procesar_orden(tJornada* pJornada, char* linea) {

    tOperacion operacion;
    int cantParametros;
    int T_inicio;

    cantParametros = parsear_orden(linea, &operacion);

    if(cantParametros == -1) {
        printf("Formato invalido. Use: DES M1 Z1 | REU Z1 Z2 | ENT | ESP | VER\n");
        return 1;
    }

    if(validar_orden(pJornada, &operacion, cantParametros) == 0) {
        return 1;
    }

    if(strcmp(operacion.codigo, "VER") == 0) {
        if(mostrar_puerto(pJornada) == 0) {
            return 0;
        }

    } else if(strcmp(operacion.codigo, "ESP") == 0) {
        T_inicio = pJornada->tiempoActual;
        operacion.tiempoInicio = T_inicio;
        operacion.tiempoFin    = T_inicio + 1;
        strcpy(operacion.contenedor.codigo, ".");
        /* origen y destino ya son "." desde parsear_orden */

        if(avanzar_reloj(pJornada, T_inicio + 1) == 0) {
            return 0;
        }
        if(enlistar_final(&pJornada->historial, &operacion, sizeof(tOperacion)) == 0) {
            return 0;
        }
        evaluar_estado(pJornada);

    } else if(strcmp(operacion.codigo, "DES") == 0) {
        if(ejecutar_des(pJornada, &operacion) == 0) {
            return 0;
        }

    } else if(strcmp(operacion.codigo, "REU") == 0) {
        if(ejecutar_reu(pJornada, &operacion) == 0) {
            return 0;
        }

    } else if(strcmp(operacion.codigo, "ENT") == 0) {
        if(ejecutar_ent(pJornada, &operacion) == 0) {
            return 0;
        }
    }

    /* Llegar aqui significa que la operacion fue ejecutada correctamente.
       Se devuelve 2 para que ejecutar_jornada sepa que debe pausar y limpiar.
       Los casos de rechazo ya retornaron 1 arriba (parseo invalido o validar_orden == 0).
       Los errores de memoria ya retornaron 0 dentro de cada ejecutar_X */
    return 2;
}

/* linea: "VER" o "DES M1 Z1". Devuelve la cantidad de parametros (0, 1 o 2), o -1 si el formato es invalido.
   Solo mira la forma: si el codigo existe y cuantos parametros lleva lo decide validar_orden */
int parsear_orden(char* linea, tOperacion* pOperacion) {

    char* pParam1;
    char* pParam2;

    /* El codigo son siempre 3 caracteres, seguidos del fin de linea o de un espacio */
    if(strlen(linea) < LARGO_CODIGO || (linea[LARGO_CODIGO] != '\0' && linea[LARGO_CODIGO] != ' ')) {
        return -1;
    }

    strncpy(pOperacion->codigo, linea, LARGO_CODIGO);
    pOperacion->codigo[LARGO_CODIGO] = '\0';
    strcpy(pOperacion->origen, ".");
    strcpy(pOperacion->destino, ".");

    if(linea[LARGO_CODIGO] == '\0') {
        return 0;                                   /* "VER" */
    }

    pParam1 = linea + LARGO_CODIGO + 1;             /* "M1 Z1" */
    pParam2 = strchr(pParam1, ' ');

    if(pParam2 != NULL) {
        *pParam2 = '\0';                            /* pParam1 = "M1" */
        pParam2++;                                  /* pParam2 = "Z1" */
    }

    /* Vacio = dos espacios seguidos o un espacio al final */
    if(*pParam1 == '\0' || strlen(pParam1) >= MAX_ID) {
        return -1;
    }

    strcpy(pOperacion->origen, pParam1);

    if(pParam2 == NULL) {
        return 1;                                   /* "DES M1" */
    }

    /* Otro espacio en pParam2 = un tercer parametro */
    if(*pParam2 == '\0' || strchr(pParam2, ' ') != NULL || strlen(pParam2) >= MAX_ID) {
        return -1;
    }

    strcpy(pOperacion->destino, pParam2);

    return 2;                                       /* "DES M1 Z1" */
}

/* Devuelve los parametros que lleva el codigo (0 o 2), o -1 si el codigo no existe */
int parametros_esperados(const char* codigo) {

    if(strcmp(codigo, "DES") == 0 || strcmp(codigo, "REU") == 0) {
        return 2;
    }

    if(strcmp(codigo, "ENT") == 0 || strcmp(codigo, "ESP") == 0 || strcmp(codigo, "VER") == 0) {
        return 0;
    }

    return -1;
}

/* --- Auxiliares de validar_orden --- */

/* Devuelve el numero N de "MN" o -1 si el formato es invalido.
   Formato valido: 'M' seguido de uno o mas digitos, sin otros caracteres.
   Ejemplos: "M1" -> 1, "M12" -> 12, "m1" -> -1, "MX" -> -1, "M" -> -1 */
int parsear_id_muelle(const char* param) {

    char* fin;
    int numero;

    /* param[0] es el primer caracter de la cadena.
       Verificamos que sea exactamente la letra 'M' mayuscula.
       Si es 'm', ' ', un numero u otra cosa, el formato es invalido */
    if(param[0] != 'M') {
        return -1;
    }

    /* param[1] es el segundo caracter. Con isdigit comprobamos que sea un digito
       del '0' al '9'. El cast a unsigned char evita comportamiento indefinido si
       el char tiene signo negativo en algunos compiladores.
       Si param[1] es '\0' (cadena "M" sola) o una letra, el formato es invalido */
    if(!isdigit((unsigned char)param[1])) {
        return -1;
    }

    /* strtol convierte la parte numerica de la cadena a entero.
       param + 1 es un puntero al segundo caracter, saltando la 'M'.
       &fin recibe la direccion del primer caracter que NO pudo convertir.
       El tercer argumento (10) indica base decimal */
    numero = (int)strtol(param + 1, &fin, 10);

    /* Si *fin no es '\0', habia caracteres no numericos despues del numero
       (por ejemplo "M1X" -> fin apunta a 'X'). Formato invalido */
    if(*fin != '\0') {
        return -1;
    }

    return numero;
}

/* Devuelve el numero N de "ZN" o -1 si el formato es invalido.
   Formato valido: 'Z' seguido de uno o mas digitos, sin otros caracteres.
   Misma logica que parsear_id_muelle pero para zonas */
int parsear_id_zona(const char* param) {

    char* fin;
    int numero;

    /* Igual que en parsear_id_muelle: primer caracter debe ser 'Z' exacto */
    if(param[0] != 'Z') {
        return -1;
    }

    /* El segundo caracter debe ser un digito: "Z1" es valido, "Z" o "ZA" no */
    if(!isdigit((unsigned char)param[1])) {
        return -1;
    }

    /* Convertimos todo lo que viene despues de la 'Z' a entero */
    numero = (int)strtol(param + 1, &fin, 10);

    /* Si quedo algo sin convertir (ej: "Z1A"), el formato es invalido */
    if(*fin != '\0') {
        return -1;
    }

    return numero;
}

/* Devuelve el costo en minutos de la operacion (0 para VER) */
int costo_operacion(const char* codigo, const tConfiguracion* cfg) {
    if(strcmp(codigo, "DES") == 0) return cfg->tiempoDescargaContenedor;
    if(strcmp(codigo, "REU") == 0) return cfg->tiempoReubicacionContenedor;
    if(strcmp(codigo, "ENT") == 0) return cfg->tiempoCargaCamion;
    if(strcmp(codigo, "ESP") == 0) return 1;
    return 0;   /* VER */
}

/* Valida la orden en capas, cortando en la primera que falla.
   Las rotaciones de la lista de zonas (capas 4 y 5) son temporales: la lista
   queda igual al terminar. No modifica ningun otro estado de la jornada.
   Devuelve 1 si la orden es valida, 0 si fue rechazada (ya informo el motivo) */
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

    /* === CAPA 1 - MAYUSCULAS ===
       Los tres caracteres del codigo deben ser letras mayusculas (A-Z).
       No se normaliza: si viene en minusculas o mixto se rechaza con mensaje claro.

       Los caracteres en C se comparan por su valor numerico en la tabla ASCII.
       'A' vale 65 y 'Z' vale 90, y estan en orden consecutivo.
       Comparar con < 'A' o > 'Z' detecta todo lo que no sea letra mayuscula:
       letras minusculas ('a'-'z'), digitos ('0'-'9'), simbolos, etc.
       Ejemplo: 'd' vale 100, que es mayor que 'Z' (90) -> falla la validacion */
    for(i = 0; i < LARGO_CODIGO; i++) {
        if(pOperacion->codigo[i] < 'A' || pOperacion->codigo[i] > 'Z') {
            printf("Ingrese la orden en mayusculas\n");
            return 0;
        }
    }

    /* === CAPA 2 - CODIGO VALIDO ===
       El codigo debe ser exactamente uno de los cinco conocidos: DES, REU, ENT, ESP o VER */
    esperados = parametros_esperados(pOperacion->codigo);
    if(esperados == -1) {
        printf("Operacion inexistente: %s\n", pOperacion->codigo);
        return 0;
    }

    /* === CAPA 3 - CANTIDAD DE PARAMETROS ===
       La cantidad recibida debe coincidir con la que exige el codigo */
    if(cantParametros != esperados) {
        if(esperados == 0) {
            printf("%s no lleva parametros\n", pOperacion->codigo);
        } else if(strcmp(pOperacion->codigo, "DES") == 0) {
            printf("DES necesita 2 parametros. Ejemplo: DES M1 Z1\n");
        } else {
            printf("REU necesita 2 parametros. Ejemplo: REU Z1 Z2\n");
        }
        return 0;
    }

    /* === CAPA 4 - EXISTENCIA DE RECURSOS ===
       Los parametros deben tener el formato correcto y referenciar recursos que
       existan en este puerto (segun la configuracion: cantidadMuelles y cantidadZonas) */
    if(strcmp(pOperacion->codigo, "DES") == 0) {

        numMuelle = parsear_id_muelle(pOperacion->origen);
        if(numMuelle == -1) {
            printf("%s no tiene el formato correcto (use M1, M2, ...)\n", pOperacion->origen);
            return 0;
        }
        if(numMuelle < 1 || numMuelle > pJornada->cfg.cantidadMuelles) {
            printf("%s no existe (hay M1 a M%d)\n", pOperacion->origen, pJornada->cfg.cantidadMuelles);
            return 0;
        }

        numZona = parsear_id_zona(pOperacion->destino);
        if(numZona == -1) {
            printf("%s no tiene el formato correcto (use Z1, Z2, ...)\n", pOperacion->destino);
            return 0;
        }
        if(numZona < 1 || numZona > pJornada->cfg.cantidadZonasAlmacenamiento) {
            printf("%s no existe (hay Z1 a Z%d)\n", pOperacion->destino, pJornada->cfg.cantidadZonasAlmacenamiento);
            return 0;
        }

    } else if(strcmp(pOperacion->codigo, "REU") == 0) {

        numZonaOrigen = parsear_id_zona(pOperacion->origen);
        if(numZonaOrigen == -1) {
            printf("%s no tiene el formato correcto (use Z1, Z2, ...)\n", pOperacion->origen);
            return 0;
        }
        if(numZonaOrigen < 1 || numZonaOrigen > pJornada->cfg.cantidadZonasAlmacenamiento) {
            printf("%s no existe (hay Z1 a Z%d)\n", pOperacion->origen, pJornada->cfg.cantidadZonasAlmacenamiento);
            return 0;
        }

        numZonaDestino = parsear_id_zona(pOperacion->destino);
        if(numZonaDestino == -1) {
            printf("%s no tiene el formato correcto (use Z1, Z2, ...)\n", pOperacion->destino);
            return 0;
        }
        if(numZonaDestino < 1 || numZonaDestino > pJornada->cfg.cantidadZonasAlmacenamiento) {
            printf("%s no existe (hay Z1 a Z%d)\n", pOperacion->destino, pJornada->cfg.cantidadZonasAlmacenamiento);
            return 0;
        }
    }

    /* === CAPA 5 - PRECONDICIONES ===
       El estado actual del puerto debe permitir la operacion.
       ESP y VER no tienen precondicion: siempre estan permitidos si pasan tiempo */
    if(strcmp(pOperacion->codigo, "DES") == 0) {

        /* El muelle debe tener un buque atracado */
        if(!pJornada->puerto.muelles[numMuelle - 1].ocupado) {
            printf("M%d no tiene ningun buque atracado\n", numMuelle);
            return 0;
        }
        /* El buque debe tener carga pendiente */
        if(pJornada->puerto.muelles[numMuelle - 1].buque.proximoADescargar >=
           pJornada->puerto.muelles[numMuelle - 1].buque.cantContenedores) {
            printf("El buque en M%d ya descargo todos sus contenedores\n", numMuelle);
            return 0;
        }
        /* La zona de destino no debe estar llena */
        cantZona = ver_cantidad_zona(&pJornada->puerto.zonas, numZona);
        if(cantZona >= pJornada->cfg.capacidadPila) {
            printf("Z%d esta llena (%d/%d)\n", numZona, cantZona, pJornada->cfg.capacidadPila);
            return 0;
        }

    } else if(strcmp(pOperacion->codigo, "REU") == 0) {

        /* Las zonas deben ser distintas */
        if(numZonaOrigen == numZonaDestino) {
            printf("Las zonas deben ser distintas\n");
            return 0;
        }
        /* La zona origen no debe estar vacia */
        if(ver_cantidad_zona(&pJornada->puerto.zonas, numZonaOrigen) == 0) {
            printf("Z%d esta vacia\n", numZonaOrigen);
            return 0;
        }
        /* La zona destino no debe estar llena */
        cantZona = ver_cantidad_zona(&pJornada->puerto.zonas, numZonaDestino);
        if(cantZona >= pJornada->cfg.capacidadPila) {
            printf("Z%d esta llena (%d/%d)\n", numZonaDestino, cantZona, pJornada->cfg.capacidadPila);
            return 0;
        }

    } else if(strcmp(pOperacion->codigo, "ENT") == 0) {

        /* Debe haber al menos un camion en la cola */
        if(pJornada->puerto.camionesEnEspera == 0) {
            printf("No hay camiones esperando\n");
            return 0;
        }
        /* El contenedor pedido por el camion del frente debe estar en el tope de alguna zona */
        ver_primero_cola(&pJornada->puerto.colaCamiones, &camionFrente, sizeof(tCamion));
        if(!contenedor_en_algun_tope(&pJornada->puerto.zonas, camionFrente.pedido.codigo)) {
            printf("%s no esta en el tope de ninguna zona\n", camionFrente.pedido.codigo);
            return 0;
        }
    }

    /* === CAPA 6 - TIEMPO ===
       La operacion debe poder completarse antes del fin de la jornada: T + costo <= duracion.
       VER tiene costo 0 y nunca llega a fallar aqui */
    costo = costo_operacion(pOperacion->codigo, &pJornada->cfg);
    if(costo > 0 && pJornada->tiempoActual + costo > pJornada->cfg.duracionJornadaMinutos) {
        printf("No hay tiempo: %s tarda %d min y solo quedan %d\n",
               pOperacion->codigo, costo,
               pJornada->cfg.duracionJornadaMinutos - pJornada->tiempoActual);
        return 0;
    }

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

/* --- Paso 9: Auxiliares para detectar si hay operaciones posibles --- */

/* Devuelve 1 si hay al menos una DES ejecutable ahora:
   - Hay un muelle con buque con contenedores pendientes.
   - Hay al menos una zona que no esta llena.
   - La operacion cabe en el tiempo restante de jornada. */
int hay_des_posible(tJornada* pJornada) {

    int i;
    int idZona;
    int hayMuelleConCarga;

    /* Capa de tiempo: tiempoActual + costo <= duracion */
    if(pJornada->tiempoActual + pJornada->cfg.tiempoDescargaContenedor >
       pJornada->cfg.duracionJornadaMinutos) {
        return 0;
    }

    /* Buscar un muelle con buque que tenga contenedores sin descargar */
    hayMuelleConCarga = 0;
    for(i = 0; i < pJornada->puerto.cantMuelles; i++) {
        if(pJornada->puerto.muelles[i].ocupado &&
           pJornada->puerto.muelles[i].buque.proximoADescargar <
           pJornada->puerto.muelles[i].buque.cantContenedores) {
            hayMuelleConCarga = 1;
            break;
        }
    }
    if(!hayMuelleConCarga) return 0;

    /* Buscar una zona con espacio libre para recibir el contenedor */
    for(idZona = 1; idZona <= pJornada->cfg.cantidadZonasAlmacenamiento; idZona++) {
        if(ver_cantidad_zona(&pJornada->puerto.zonas, idZona) < pJornada->cfg.capacidadPila) {
            return 1;
        }
    }
    return 0;
}

/* Devuelve 1 si hay al menos una REU ejecutable ahora:
   - Existe una zona origen no vacia y una zona destino diferente no llena.
   - La operacion cabe en el tiempo restante de jornada.
   El doble for recorre todas las combinaciones posibles de (origen, destino). */
int hay_reu_posible(tJornada* pJornada) {

    int origen;
    int destino;

    /* Capa de tiempo */
    if(pJornada->tiempoActual + pJornada->cfg.tiempoReubicacionContenedor >
       pJornada->cfg.duracionJornadaMinutos) {
        return 0;
    }

    /* Buscar un par (origen != destino) donde origen no este vacio
       y destino no este lleno. n es el numero de zonas (tipicamente <= 6),
       por lo que O(n^2) es completamente aceptable aqui */
    for(origen = 1; origen <= pJornada->cfg.cantidadZonasAlmacenamiento; origen++) {
        if(ver_cantidad_zona(&pJornada->puerto.zonas, origen) == 0) continue;
        for(destino = 1; destino <= pJornada->cfg.cantidadZonasAlmacenamiento; destino++) {
            if(destino == origen) continue;
            if(ver_cantidad_zona(&pJornada->puerto.zonas, destino) <
               pJornada->cfg.capacidadPila) {
                return 1;
            }
        }
    }
    return 0;
}

/* Devuelve 1 si hay al menos una ENT ejecutable ahora:
   - Hay al menos un camion en la cola.
   - El contenedor pedido por el camion del frente esta en el tope de alguna zona.
   - La operacion cabe en el tiempo restante de jornada. */
int hay_ent_posible(tJornada* pJornada) {

    tCamion camion;

    /* Capa de tiempo */
    if(pJornada->tiempoActual + pJornada->cfg.tiempoCargaCamion >
       pJornada->cfg.duracionJornadaMinutos) {
        return 0;
    }

    if(pJornada->puerto.camionesEnEspera == 0) return 0;

    /* El contenedor que pide el primer camion de la cola debe estar
       accesible: en el tope de alguna zona (no enterrado debajo de otro) */
    ver_primero_cola(&pJornada->puerto.colaCamiones, &camion, sizeof(tCamion));
    return contenedor_en_algun_tope(&pJornada->puerto.zonas, camion.pedido.codigo);
}

/* Devuelve 1 si al menos una de DES, REU o ENT es posible en el estado actual */
int hay_operacion_posible(tJornada* pJornada) {
    return hay_des_posible(pJornada) ||
           hay_reu_posible(pJornada) ||
           hay_ent_posible(pJornada);
}

/* Devuelve el tiempo del proximo arribo en el escenario (buque o camion,
   el menor de los dos). Solo se debe llamar cuando hayEventosFuturos es 1 */
int proximo_tiempo_arribo(tJornada* pJornada) {

    tBuque  buque;
    tCamion camion;
    int hayBuque;
    int hayCamion;
    int tiempoBuque;
    int tiempoCamion;

    hayBuque  = ver_primero_lista(&pJornada->escenario.buques,   &buque,  sizeof(tBuque))  == 1;
    hayCamion = ver_primero_lista(&pJornada->escenario.camiones, &camion, sizeof(tCamion)) == 1;

    tiempoBuque  = hayBuque  ? buque.tiempoLlegada  : pJornada->cfg.duracionJornadaMinutos + 1;
    tiempoCamion = hayCamion ? camion.tiempoLlegada : pJornada->cfg.duracionJornadaMinutos + 1;

    return tiempoBuque < tiempoCamion ? tiempoBuque : tiempoCamion;
}

/* Se llama despues de cada cambio de estado.
   Determina si la jornada termino y por que motivo, o si corresponde
   hacer un salto automatico al proximo arribo.

   Orden de verificacion:
   1. FIN_NORMAL  : no hay nada pendiente (sin eventos, sin buques, sin camiones).
   2. FIN_TIEMPO  : el reloj llego al fin de la jornada.
   3. BLOQUEO     : no hay DES/REU/ENT posible.
      3a. Sin eventos futuros -> FIN_BLOQUEO.
      3b. Con eventos futuros -> salto automatico al proximo arribo y re-evaluar. */
void evaluar_estado(tJornada* pJornada) {

    tPuerto* pPuerto = &pJornada->puerto;
    int hayEventosFuturos;
    int hayBuquesConCarga;
    int proximoArribo;

    /* Un buque siempre tiene carga mientras este atracado: el muelle se libera
       solo cuando el buque descarga todo (en ejecutar_des) */
    hayBuquesConCarga = hay_buque_atracado(pPuerto) || pPuerto->buquesEnEspera > 0;

    /* Lista de buques/camiones pendientes de llegar al puerto */
    hayEventosFuturos = !lista_vacia(&pJornada->escenario.buques) ||
                        !lista_vacia(&pJornada->escenario.camiones);

    /* --- FIN_NORMAL: no queda nada pendiente en todo el juego --- */
    if(!hayEventosFuturos && !hayBuquesConCarga && pPuerto->camionesEnEspera == 0) {
        pJornada->resumen.motivoFin = FIN_NORMAL;
        return;
    }

    /* --- FIN_TIEMPO: el reloj alcanzo la duracion configurada --- */
    if(pJornada->tiempoActual >= pJornada->cfg.duracionJornadaMinutos) {
        pJornada->resumen.motivoFin = FIN_TIEMPO;
        return;
    }

    /* --- Bloqueo / Salto automatico ---
       Si el operario no puede ejecutar ninguna DES, REU ni ENT, hay dos casos:
       a) Sin eventos futuros: nada va a cambiar -> FIN_BLOQUEO.
       b) Con eventos futuros: saltamos al proximo arribo sin requerir ESPs
          manuales, procesamos los nuevos buques/camiones y re-evaluamos.
       Nota: la recursion termina porque cada salto consume al menos un evento
       del escenario (hayEventosFuturos se acorta en cada llamada recursiva). */
    if(!hay_operacion_posible(pJornada)) {
        if(!hayEventosFuturos) {
            /* No hay arribes por venir ni operaciones posibles: bloqueo definitivo */
            pJornada->resumen.motivoFin = FIN_BLOQUEO;
        } else {
            /* Hay arribes pendientes: avanzar el reloj al proximo sin pedir ESP */
            proximoArribo = proximo_tiempo_arribo(pJornada);
            printf("\n[Salto automatico] Sin operaciones posibles. Avanzando a T=%d...\n",
                   proximoArribo);
            if(avanzar_reloj(pJornada, proximoArribo) == 0) {
                /* Error de memoria: terminar de forma controlada */
                pJornada->resumen.motivoFin = FIN_BLOQUEO;
                return;
            }
            /* Re-evaluar: los nuevos arribes pueden desbloquear operaciones */
            evaluar_estado(pJornada);
        }
    }
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
        zona.capacidad = pConfiguracion->capacidadPila;
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
/* REU Zx Zy: mueve el contenedor del tope de la zona Zx al tope de la zona Zy */
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

    /* Paso 1 */
    if(T_fin - T_inicio > 1) {
        if(procesar_arribos(pJornada, T_fin - 1) == 0) return 0;
        asignar_muelles(&pJornada->puerto);
    }

    /* Paso 2: sacar el contenedor de la zona origen y ponerlo en la zona destino */
    desapilar_de_zona(&pJornada->puerto.zonas, idOrigen, &contenedor);

    if(apilar_en_zona(&pJornada->puerto.zonas, idDestino, &contenedor) == 0) {
        /* Recovery: si falla el malloc al apilar en destino, devolver a origen */
        apilar_en_zona(&pJornada->puerto.zonas, idOrigen, &contenedor);
        return 0;
    }

    pJornada->resumen.reubicaciones++;
    printf("REU: %s de Z%d -> Z%d\n", contenedor.codigo, idOrigen, idDestino);

    /* Pasos 3 y 4 */
    if(procesar_arribos(pJornada, T_fin) == 0) return 0;
    asignar_muelles(&pJornada->puerto);

    /* Paso 5 */
    registro.tiempoInicio = T_inicio;
    registro.tiempoFin    = T_fin;
    strcpy(registro.codigo, "REU");
    registro.contenedor   = contenedor;
    sprintf(registro.origen,  "Z%d", idOrigen);
    sprintf(registro.destino, "Z%d", idDestino);

    pJornada->tiempoActual = T_fin;

    if(enlistar_final(&pJornada->historial, &registro, sizeof(tOperacion)) == 0) return 0;

    evaluar_estado(pJornada);
    return 1;
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

/* Imprime "| texto |" con el texto completado con espacios hasta el ancho del recuadro */
void linea_recuadro(const char* texto) {

    printf("| %-*s |\n", ANCHO_RECUADRO, texto);
}

/* "#=====...=====#" o "|-----...-----|" del mismo ancho que linea_recuadro */
void separador_recuadro(char extremo, char relleno) {

    int i;

    printf("%c", extremo);
    for(i = 0; i < ANCHO_RECUADRO + 2; i++) {
        printf("%c", relleno);
    }
    printf("%c\n", extremo);
}

/* Ayuda de operaciones: se muestra siempre antes de pedir una orden */
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

    sprintf(texto, "%-58sCosto: %3d min", "ESP        Esperar", 1);
    linea_recuadro(texto);
    linea_recuadro("           Avanza el reloj un minuto sin hacer otra operacion.");
    separador_recuadro('|', '-');

    sprintf(texto, "%-58sCosto: %3d min", "VER        Ver estado", 0);
    linea_recuadro(texto);
    linea_recuadro("           Muestra muelles, zonas, camiones y puntuacion.");
    separador_recuadro('#', '=');

    sprintf(texto, "Muelles: M1 a M%d   Zonas: Z1 a Z%d   Capacidad por zona: %d   Fin: T=%d",
            pConfiguracion->cantidadMuelles, pConfiguracion->cantidadZonasAlmacenamiento,
            pConfiguracion->capacidadPila, pConfiguracion->duracionJornadaMinutos);
    linea_recuadro(texto);
    separador_recuadro('#', '=');
}

/* VER. Devuelve 0 solo si falla la memoria al volver a poner un contenedor o un camion */
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
    if(mostrar_zonas(&pJornada->puerto.zonas, pJornada->cfg.cantidadZonasAlmacenamiento, pJornada->cfg.capacidadPila) == 0) {
        printf("Error al reservar memoria al mostrar las zonas\n");
        return 0;
    }
    printf("#---------------------------------------------------------------------#\n");

    printf("  CAMIONES (ventana de planificacion)\n");
    if(mostrar_ventana_camiones(&pJornada->puerto.colaCamiones, pJornada->puerto.camionesEnEspera) == 0) {
        printf("Error al reservar memoria al mostrar los camiones\n");
        return 0;
    }
    printf("#---------------------------------------------------------------------#\n");

    printf("  Entregados: %d   Buques descargados: %d   Camiones en cola: %d\n",
           pJornada->resumen.contenedoresEntregados, pJornada->resumen.buquesDescargados,
           pJornada->puerto.camionesEnEspera);
    printf("  Puntuacion provisoria: 10x%d + 5x%d - 2x%d = %d\n",
           pJornada->resumen.contenedoresEntregados, pJornada->resumen.buquesDescargados,
           pJornada->puerto.camionesEnEspera, calcular_puntuacion(pJornada));
    printf("#=====================================================================#\n");

    return 1;
}

/* "M1: B001   proximo -> [C102] C103 C104   (faltan 3 de 4)" */
void mostrar_muelle(const tMuelle* pMuelle) {

    int i;
    int faltan;
    const tBuque* pBuque = &pMuelle->buque;

    if(pMuelle->ocupado == 0) {
        printf("    M%d: libre\n", pMuelle->id);
        return;
    }

    printf("    M%d: %-6s proximo -> [%s]", pMuelle->id, pBuque->id,
           pBuque->manifiesto[pBuque->proximoADescargar].codigo);

    for(i = pBuque->proximoADescargar + 1; i < pBuque->cantContenedores; i++) {
        printf(" %s", pBuque->manifiesto[i].codigo);
    }

    faltan = pBuque->cantContenedores - pBuque->proximoADescargar;

    if(faltan == 1) {
        printf("   (falta 1 de %d)\n", pBuque->cantContenedores);
    } else {
        printf("   (faltan %d de %d)\n", faltan, pBuque->cantContenedores);
    }
}

/* Accion que se le pasa a recorrer_lista: muestra una zona usando una pila auxiliar
   local. La zona queda identica al terminar (doble inversion) */
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

    /* Paso 1: desapilar zona -> apilar aux (invierte el orden) */
    while(desapilar(&pZona->pila, &contenedor, sizeof(tContenedor)) == 1) {
        apilar(&aux, &contenedor, sizeof(tContenedor));
    }

    /* Paso 2: desapilar aux -> mostrar -> apilar zona (invierte de nuevo: queda igual) */
    while(desapilar(&aux, &contenedor, sizeof(tContenedor)) == 1) {
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

/* Provisorio hasta tener recorrer_lista: se rota la lista (cada zona sale del
   principio y vuelve al final), asi despues de cantZonas vueltas queda igual */
int mostrar_zonas(tLista* pZonas, int cantZonas, int capacidad) {

    recorrer_lista(pZonas, accion_mostrar_zona);
    return 1;
}

/* Queda para compilacion: se usa en pruebas directas de una zona con pila externa */
int mostrar_zona(tZona* pZona, int capacidad, tPila* pAux) {

    tContenedor contenedor;

    printf("    Z%d (%d/%d): ", pZona->id, pZona->cantidad, capacidad);

    if(pila_vacia(&pZona->pila)) {
        printf(" vacia\n");
        return 1;
    }

    while(desapilar(&pZona->pila, &contenedor, sizeof(tContenedor)) == 1) {
        if(apilar(pAux, &contenedor, sizeof(tContenedor)) == 0) {
            return 0;
        }
    }

    while(desapilar(pAux, &contenedor, sizeof(tContenedor)) == 1) {
        printf(" %s", contenedor.codigo);
        if(apilar(&pZona->pila, &contenedor, sizeof(tContenedor)) == 0) {
            return 0;
        }
    }

    printf("   <- tope");

    if(pZona->cantidad == capacidad) {
        printf("   LLENA");
    }

    printf("\n");

    return 1;
}

/* Rota la cola completa (sale del frente y vuelve al final) mostrando los
   primeros VENTANA_CAMIONES: despues de cantCamiones vueltas queda igual */
int mostrar_ventana_camiones(tCola* pCola, int cantCamiones) {

    int i;
    tCamion camion;

    if(cantCamiones == 0) {
        printf("    (no hay camiones esperando)\n");
        return 1;
    }

    for(i = 0; i < cantCamiones; i++) {

        desencolar(pCola, &camion, sizeof(tCamion));

        if(i < VENTANA_CAMIONES) {
            printf("    %d) %s --> Solicita %s", i + 1, camion.id, camion.pedido.codigo);
            if(i == 0) {
                printf("   <-- Frente");
            }
            printf("\n");
        }

        if(encolar(pCola, &camion, sizeof(tCamion)) == 0) {
            return 0;
        }
    }

    if(cantCamiones > VENTANA_CAMIONES) {
        printf("    (+%d camion(es) mas esperando)\n", cantCamiones - VENTANA_CAMIONES);
    }

    return 1;
}

/* --- Comparadores para buscar_en_lista con tZona --- */

/* Compara una zona por su id. clave es un puntero a int */
int comparar_zona_por_id(const void* dato, const void* clave) {
    return ((const tZona*)dato)->id - *(const int*)clave;
}

/* Compara una zona por el codigo del contenedor en su tope.
   clave es un puntero a char (el codigo a buscar).
   Devuelve 0 si el tope coincide, distinto de 0 si no */
int comparar_zona_por_tope(const void* dato, const void* clave) {

    tZona* pZona = (tZona*)dato;
    tContenedor tope;

    if(pila_vacia(&pZona->pila)) {
        return 1;
    }
    ver_tope(&pZona->pila, &tope, sizeof(tContenedor));
    return strcmp(tope.codigo, (const char*)clave);
}

/* --- Helpers para operar sobre zonas (usan buscar_en_lista) --- */

/* Desapila el contenedor del tope de la zona con ese id y lo copia en pContenedor.
   No puede fallar: validar_orden ya verifico que la zona no esta vacia */
void desapilar_de_zona(tLista* pZonas, int idZona, tContenedor* pContenedor) {

    tZona* pZona = (tZona*)buscar_en_lista(pZonas, &idZona, comparar_zona_por_id);
    desapilar(&pZona->pila, pContenedor, sizeof(tContenedor));
    pZona->cantidad--;
}

/* Apila un contenedor en la zona con ese id y actualiza zona.cantidad.
   Devuelve 0 si apilar falla (malloc): la zona queda sin cambios en ese caso */
int apilar_en_zona(tLista* pZonas, int idZona, const tContenedor* pContenedor) {

    tZona* pZona = (tZona*)buscar_en_lista(pZonas, &idZona, comparar_zona_por_id);
    if(apilar(&pZona->pila, pContenedor, sizeof(tContenedor)) == 0) {
        return 0;
    }
    pZona->cantidad++;
    return 1;
}

/* Devuelve la cantidad de contenedores de la zona con ese id */
int ver_cantidad_zona(tLista* pZonas, int idZona) {

    tZona* pZona = (tZona*)buscar_en_lista(pZonas, &idZona, comparar_zona_por_id);
    return pZona != NULL ? pZona->cantidad : 0;
}

/* Devuelve el id de la zona que tenga ese contenedor en su tope, o -1 si ninguna */
int buscar_zona_con_tope(tLista* pZonas, const char* codigoCont) {

    tZona* pZona = (tZona*)buscar_en_lista(pZonas, codigoCont, comparar_zona_por_tope);
    return pZona != NULL ? pZona->id : -1;
}

/* Devuelve 1 si el contenedor con ese codigo esta en el tope de alguna zona */
int contenedor_en_algun_tope(tLista* pZonas, const char* codigoCont) {

    return buscar_en_lista(pZonas, codigoCont, comparar_zona_por_tope) != NULL;
}

/* --- Operaciones con orden temporal (ADR-10) ---
   Cada funcion sigue los mismos 5 pasos:
     1. Arribar entre T_inicio y T_fin (exclusivo): solo si el costo es > 1 minuto.
     2. Aplicar las consecuencias en T_fin.
     3. Arribar en exactamente T_fin.
     4. Asignar buques en cola a muelles libres.
     5. Avanzar el reloj, guardar en historial y evaluar estado.
   El reloj (tiempoActual) se actualiza al final para que los prints de arribo
   muestren los tiempos correctos. */

/* DES Mx Zy: toma el proximo contenedor del buque atracado en el muelle Mx
   y lo apila en la zona Zy. Si el buque termina de descargarse, libera el muelle */
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
    pMuelle   = &pJornada->puerto.muelles[numMuelle - 1];

    /* Paso 1: arribar durante la operacion (T_inicio < T < T_fin) */
    if(T_fin - T_inicio > 1) {
        if(procesar_arribos(pJornada, T_fin - 1) == 0) return 0;
        asignar_muelles(&pJornada->puerto);
    }

    /* Paso 2: consecuencias en T_fin */
    /* Tomar el proximo contenedor del manifiesto del buque */
    contenedor = pMuelle->buque.manifiesto[pMuelle->buque.proximoADescargar];
    pMuelle->buque.proximoADescargar++;

    /* Apilarlo en la zona de destino */
    if(apilar_en_zona(&pJornada->puerto.zonas, numZona, &contenedor) == 0) {
        pMuelle->buque.proximoADescargar--;     /* revertir si falla el malloc */
        return 0;
    }

    /* Si el buque termino de descargarse, liberar el muelle */
    strcpy(idBuque, pMuelle->buque.id);         /* guardar id antes de limpiar */
    if(pMuelle->buque.proximoADescargar >= pMuelle->buque.cantContenedores) {
        free(pMuelle->buque.manifiesto);
        pMuelle->buque.manifiesto = NULL;
        pMuelle->ocupado = 0;
        pJornada->resumen.buquesDescargados++;
        printf("%s termino de descargarse: M%d queda libre\n", idBuque, numMuelle);
    }

    printf("DES: %s de %s (M%d) -> Z%d\n", contenedor.codigo, idBuque, numMuelle, numZona);

    /* Pasos 3 y 4: arribar en T_fin y asignar muelles */
    if(procesar_arribos(pJornada, T_fin) == 0) return 0;
    asignar_muelles(&pJornada->puerto);

    /* Paso 5: avanzar reloj, guardar en historial y evaluar */
    registro.tiempoInicio = T_inicio;
    registro.tiempoFin    = T_fin;
    strcpy(registro.codigo, "DES");
    registro.contenedor   = contenedor;
    sprintf(registro.origen,  "M%d", numMuelle);
    sprintf(registro.destino, "Z%d", numZona);

    pJornada->tiempoActual = T_fin;

    if(enlistar_final(&pJornada->historial, &registro, sizeof(tOperacion)) == 0) return 0;

    evaluar_estado(pJornada);
    return 1;
}

/* ENT: entrega al camion del frente de la cola el contenedor que pide,
   sacandolo del tope de la zona donde este */
int ejecutar_ent(tJornada* pJornada, const tOperacion* pOperacion) {

    int T_inicio;
    int T_fin;
    int idZona;
    tCamion camion;
    tContenedor contenedor;
    tOperacion registro;

    T_inicio = pJornada->tiempoActual;
    T_fin    = T_inicio + pJornada->cfg.tiempoCargaCamion;

    /* Paso 1 */
    if(T_fin - T_inicio > 1) {
        if(procesar_arribos(pJornada, T_fin - 1) == 0) return 0;
        asignar_muelles(&pJornada->puerto);
    }

    /* Paso 2: buscar la zona con el contenedor pedido, desapilarlo y despachar el camion */
    ver_primero_cola(&pJornada->puerto.colaCamiones, &camion, sizeof(tCamion));
    idZona = buscar_zona_con_tope(&pJornada->puerto.zonas, camion.pedido.codigo);

    desapilar_de_zona(&pJornada->puerto.zonas, idZona, &contenedor);
    desencolar(&pJornada->puerto.colaCamiones, &camion, sizeof(tCamion));
    pJornada->puerto.camionesEnEspera--;
    pJornada->resumen.contenedoresEntregados++;

    printf("ENT: %s entregado a %s desde Z%d\n", contenedor.codigo, camion.id, idZona);

    /* Pasos 3 y 4 */
    if(procesar_arribos(pJornada, T_fin) == 0) return 0;
    asignar_muelles(&pJornada->puerto);

    /* Paso 5 */
    registro.tiempoInicio = T_inicio;
    registro.tiempoFin    = T_fin;
    strcpy(registro.codigo, "ENT");
    registro.contenedor   = contenedor;
    sprintf(registro.origen,  "Z%d", idZona);
    strcpy(registro.destino, camion.id);

    pJornada->tiempoActual = T_fin;

    if(enlistar_final(&pJornada->historial, &registro, sizeof(tOperacion)) == 0) return 0;

    evaluar_estado(pJornada);
    return 1;
}

/* 10 por contenedor entregado + 5 por buque descargado - 2 por camion en cola */
int calcular_puntuacion(const tJornada* pJornada) {

    return 10 * pJornada->resumen.contenedoresEntregados +
            5 * pJornada->resumen.buquesDescargados -
            2 * pJornada->puerto.camionesEnEspera;
}

/* --- Paso 10: Cierre de jornada --- */

/* Accion que se le pasa a recorrer_lista para imprimir una fila del historial.
   dato apunta al info del nodo (tOperacion). Se muestra cada operacion con
   su intervalo de tiempo, codigo, contenedor involucrado, origen y destino.
   ESP no tiene contenedor real: se muestra "-" en esos campos */
void accion_mostrar_operacion(void* dato) {

    tOperacion* pOp = (tOperacion*)dato;

    if(strcmp(pOp->codigo, "ESP") == 0) {
        /* ESP: no hay contenedor ni origen/destino con sentido */
        printf("  T=%3d->T=%3d | %-3s | %-10s | %-6s -> %s\n",
               pOp->tiempoInicio, pOp->tiempoFin,
               pOp->codigo,
               "-",
               "-", "-");
    } else {
        /* DES: origen=Mx, destino=Zy
           REU: origen=Zx, destino=Zy
           ENT: origen=Zy, destino=Kxxx (id del camion) */
        printf("  T=%3d->T=%3d | %-3s | %-10s | %-6s -> %s\n",
               pOp->tiempoInicio, pOp->tiempoFin,
               pOp->codigo,
               pOp->contenedor.codigo,
               pOp->origen,
               pOp->destino);
    }
}

/* Imprime el historial completo de operaciones usando recorrer_lista.
   Se llama al cerrar la jornada, antes de mostrar_resumen */
void mostrar_historial(tHistorial* pHistorial) {

    printf("\n");
    separador_recuadro('#', '=');
    linea_recuadro("  HISTORIAL DE OPERACIONES");
    separador_recuadro('#', '=');
    printf("  %-14s | %-3s | %-10s | %s\n", "Intervalo", "Op", "Contenedor", "Origen -> Destino");
    separador_recuadro('|', '-');

    if(lista_vacia(pHistorial)) {
        /* No se ejecuto ninguna operacion en toda la jornada */
        linea_recuadro("  (ninguna operacion ejecutada)");
    } else {
        /* Cada nodo de la lista es un tOperacion; accion_mostrar_operacion
           imprime una fila por operacion en el orden cronologico en que
           se realizaron (enlistar_final mantiene ese orden) */
        recorrer_lista(pHistorial, accion_mostrar_operacion);
    }

    separador_recuadro('#', '=');
}

/* Imprime el resumen final de la jornada: motivo de cierre, tiempo, estadisticas
   y puntuacion definitiva. Se llama despues de mostrar_historial */
void mostrar_resumen(const tJornada* pJornada) {

    const tResumen* r = &pJornada->resumen;
    const char* motivo;
    char texto[MAX_BUFFER];

    /* Convertir el enum motivoFin a un texto legible */
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

    /* Mostrar la formula para que quede claro como se calculo la puntuacion */
    sprintf(texto, "Puntuacion: 10x%d + 5x%d - 2x%d = %d",
            r->contenedoresEntregados,
            r->buquesDescargados,
            r->camionesPendientes,
            r->puntuacion);
    linea_recuadro(texto);

    separador_recuadro('#', '=');
    printf("\n");
}
