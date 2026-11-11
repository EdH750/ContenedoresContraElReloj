#include "aplicacion.h"

const char* NOMBRES_CAMPOS[CANTIDAD_CAMPOS_CONFIG] = {
    "duracion_jornada_minutos",
    "cantidad_muelles",
    "cantidad_zonas_almacenamiento",
    "capacidad_pila",
    "maximo_buques",
    "maximo_contenedores_por_buque",
    "maximo_camiones",
    "tiempo_descarga_contenedor",
    "tiempo_reubicacion_contenedor",
    "tiempo_carga_camion"
};

/*
Lee config.txt y llena pConfiguracion.
Lee linea por linea en orden de posicion de campo (campo 1, campo 2, ...).
Devuelve EXITO si todo es correcto, ERROR si hay algun error de lectura o validacion.
*/
int procesar_configuracion(const char* file_name, tConfiguracion* pConfiguracion) {

    FILE* pArchivo;
    int campo = PRIMER_CAMPO;
    int ok = EXITO;
    int i;
    int leidos[CANTIDAD_CAMPOS_CONFIG] = {0};
    char buffer[MAX_BUFFER];

    pArchivo = fopen(file_name, "rt");

    if(pArchivo == NULL) {
        printf("Error al abrir el archivo configuracion\n");
        return ERROR;
    }

    while(ok && fgets(buffer, sizeof(buffer), pArchivo) != NULL) {

        buffer[strcspn(buffer, "\r\n")] = '\0';

        if(linea_vacia(buffer)) {
            continue;
        }

        if(campo > CANTIDAD_CAMPOS_CONFIG) {
            printf("Error en configuracion: hay mas de %d campos\n", CANTIDAD_CAMPOS_CONFIG);
            ok = ERROR;
        } else if(parsear_campos(buffer, pConfiguracion, campo) == EXITO) {
            leidos[campo - 1] = EXITO;
            campo++;
        } else {
            ok = ERROR;
        }
    }

    fclose(pArchivo);

    if(ok == ERROR) {
        return ERROR;
    }

    for(i = 0; i < CANTIDAD_CAMPOS_CONFIG; i++) {
        if(leidos[i] == ERROR) {
            printf("Error en configuracion: falta el campo %s\n", NOMBRES_CAMPOS[i]);
            ok = ERROR;
        }
    }

    if(ok == ERROR || validar_campos(pConfiguracion) == ERROR) {
        printf("Los campos no cumplen los requisitos\n");
        return ERROR;
    }

    return EXITO;
}

/*
Devuelve EXITO si la linea contiene solo espacios, tabs o saltos de linea; ERROR en caso contrario.
Se usa para saltear lineas en blanco en config.txt sin contarlas como campo.
*/
int linea_vacia(const char* linea) {

    while(*linea != '\0') {
        if(!isspace((unsigned char)*linea)) {
            return ERROR;
        }
        linea++;
    }

    return EXITO;
}

/*
Extrae el numero de una linea con formato "nombre_campo: valor".
Busca el ultimo ':' con strrchr, convierte lo que sigue a entero y lo guarda
en el campo indicado de pConfiguracion (campo 1 al 10).
Devuelve EXITO si pudo leer el numero, ERROR si falta ':' o el valor no es entero.
*/
int parsear_campos(char* linea, tConfiguracion* pConfiguracion, int campo) {

    char* pLim;
    int valor;

    pLim = strrchr(linea, ':');

    if(pLim == NULL) {
        printf("Error en configuracion: falta ':' en el campo %s (\"%s\")\n", NOMBRES_CAMPOS[campo - 1], linea);
        return ERROR;
    }

    pLim++;

    if(leer_entero(pLim, &valor) == ERROR) {
        printf("Error en configuracion: %s no es un numero entero valido (\"%s\")\n", NOMBRES_CAMPOS[campo - 1], pLim);
        return ERROR;
    }

    switch(campo) {
        case 1:  pConfiguracion->duracionJornadaMinutos      = valor; break;
        case 2:  pConfiguracion->cantidadMuelles             = valor; break;
        case 3:  pConfiguracion->cantidadZonasAlmacenamiento = valor; break;
        case 4:  pConfiguracion->capacidadPila               = valor; break;
        case 5:  pConfiguracion->maximoBuques                = valor; break;
        case 6:  pConfiguracion->maximoContenedoresPorBuque  = valor; break;
        case 7:  pConfiguracion->maximoCamiones              = valor; break;
        case 8:  pConfiguracion->tiempoDescargaContenedor    = valor; break;
        case 9:  pConfiguracion->tiempoReubicacionContenedor = valor; break;
        case 10: pConfiguracion->tiempoCargaCamion           = valor; break;
    }

    return EXITO;
}

/*
Verifica que 'valor' este entre minimo y maximo (inclusive).
Devuelve EXITO si esta en rango, ERROR con mensaje descriptivo en caso contrario.
*/
int validar_rango(int campo, int valor, int minimo, int maximo) {

    if(valor >= minimo && valor <= maximo) {
        return EXITO;
    }

    if(maximo == INT_MAX) {
        printf("Error en configuracion: %s debe ser mayor o igual a %d (se leyo %d)\n", NOMBRES_CAMPOS[campo - 1], minimo, valor);
    } else {
        printf("Error en configuracion: %s debe estar entre %d y %d (se leyo %d)\n", NOMBRES_CAMPOS[campo - 1], minimo, maximo, valor);
    }

    return ERROR;
}

/*
Evalua todos los rangos de los campos de configuracion.
Acumula resultados con &= para informar todos los errores en un solo llamado.
Devuelve EXITO si todos los campos son validos, ERROR si al menos uno fallo.
*/
int validar_campos(tConfiguracion* pConfiguracion) {

    int ok = EXITO;
    int duracion = pConfiguracion->duracionJornadaMinutos;

    ok &= validar_rango(1,  pConfiguracion->duracionJornadaMinutos,      MINIMO_TIEMPO_JORNADA, MAXIMO_TIEMPO_JORNADA);
    ok &= validar_rango(2,  pConfiguracion->cantidadMuelles,             MINIMO_CANTIDAD, MAXIMO_MUELLES);
    ok &= validar_rango(3,  pConfiguracion->cantidadZonasAlmacenamiento, MINIMO_CANTIDAD, MAXIMO_ZONAS);
    ok &= validar_rango(4,  pConfiguracion->capacidadPila,               MINIMO_CANTIDAD, MAXIMO_CAPACIDAD_PILA);
    ok &= validar_rango(5,  pConfiguracion->maximoBuques,                MINIMO_CANTIDAD, MAXIMO_BUQUES);
    ok &= validar_rango(6,  pConfiguracion->maximoContenedoresPorBuque,  MINIMO_CANTIDAD, MAXIMO_CONTENEDORES_POR_BUQUE);
    ok &= validar_rango(7,  pConfiguracion->maximoCamiones,              MINIMO_CANTIDAD, MAXIMO_CAMIONES);
    ok &= validar_rango(8,  pConfiguracion->tiempoDescargaContenedor,    MINIMO_CANTIDAD, duracion);
    ok &= validar_rango(9,  pConfiguracion->tiempoReubicacionContenedor, MINIMO_CANTIDAD, duracion);
    ok &= validar_rango(10, pConfiguracion->tiempoCargaCamion,           MINIMO_CANTIDAD, duracion);

    return ok;
}

/*
Genera puerto.txt con un escenario aleatorio consistente con la configuracion.
Sortea buques con sus contenedores, distribuye los contenedores a camiones mediante
un shuffle Fisher-Yates, y escribe el archivo.
Devuelve EXITO si todo salio bien, ERROR si hubo un fallo de malloc o fopen.
*/
int generar_archivo_puerto(tConfiguracion* pConfiguracion) {

    FILE* pArchivo;
    int i, j, k;
    int maxContenedoresTotal;
    int numBuques;
    int totalContenedores;
    int maxCamionesValido;
    int numCamiones;
    char tmp[MAX_COD_CONTENEDOR];

    int*                      buqueT;
    int*                      buqueNumCont;
    int*                      camionT;
    char (*pool)[MAX_COD_CONTENEDOR];

    maxContenedoresTotal = pConfiguracion->maximoBuques * pConfiguracion->maximoContenedoresPorBuque;

    buqueT       = (int*)malloc(pConfiguracion->maximoBuques * sizeof(int));
    buqueNumCont = (int*)malloc(pConfiguracion->maximoBuques * sizeof(int));
    camionT      = (int*)malloc(pConfiguracion->maximoCamiones * sizeof(int));
    pool         = (char(*)[MAX_COD_CONTENEDOR])malloc(maxContenedoresTotal * sizeof(*pool));

    if(buqueT == NULL || buqueNumCont == NULL || camionT == NULL || pool == NULL) {
        printf("Error al reservar memoria\n");
        free(buqueT);
        free(buqueNumCont);
        free(camionT);
        free(pool);
        return ERROR;
    }

    numBuques         = (rand() % pConfiguracion->maximoBuques) + 1;
    totalContenedores = 0;

    for(i = 0; i < numBuques; i++) {
        buqueT[i] = rand() % pConfiguracion->duracionJornadaMinutos;
    }
    ordenar_tiempos(buqueT, numBuques);

    for(i = 0; i < numBuques; i++) {
        buqueNumCont[i] = (rand() % pConfiguracion->maximoContenedoresPorBuque) + 1;
        for(j = 0; j < buqueNumCont[i]; j++) {
            sprintf(pool[totalContenedores], "C%d%02d", i + 1, j + 1);
            totalContenedores++;
        }
    }

    for(i = totalContenedores - 1; i > 0; i--) {
        j = rand() % (i + 1);
        strcpy(tmp, pool[i]);
        strcpy(pool[i], pool[j]);
        strcpy(pool[j], tmp);
    }

    maxCamionesValido = pConfiguracion->maximoCamiones;
    if(totalContenedores < maxCamionesValido) {
        maxCamionesValido = totalContenedores;
    }
    numCamiones = (rand() % maxCamionesValido) + 1;

    for(k = 0; k < numCamiones; k++) {
        camionT[k] = rand() % pConfiguracion->duracionJornadaMinutos;
    }
    ordenar_tiempos(camionT, numCamiones);

    pArchivo = fopen(PUERTO_ARCHIVO, "wt");

    if(pArchivo == NULL) {
        printf("Error al crear el archivo %s\n", PUERTO_ARCHIVO);
        free(buqueT);
        free(buqueNumCont);
        free(camionT);
        free(pool);
        return ERROR;
    }

    fprintf(pArchivo, "JORNADA:%d\n",        pConfiguracion->duracionJornadaMinutos);
    fprintf(pArchivo, "MUELLES:%d\n",        pConfiguracion->cantidadMuelles);
    fprintf(pArchivo, "ZONAS:%d\n",          pConfiguracion->cantidadZonasAlmacenamiento);
    fprintf(pArchivo, "CAPACIDAD_PILA:%d\n", pConfiguracion->capacidadPila);
    fprintf(pArchivo, "\n");

    fprintf(pArchivo, "[BUQUES]\n");
    for(i = 0; i < numBuques; i++) {
        fprintf(pArchivo, "B%03d;T=%d;C=", i + 1, buqueT[i]);
        for(j = 0; j < buqueNumCont[i]; j++) {
            if(j > 0) fprintf(pArchivo, ",");
            fprintf(pArchivo, "C%d%02d", i + 1, j + 1);
        }
        fprintf(pArchivo, "\n");
    }

    fprintf(pArchivo, "\n");

    fprintf(pArchivo, "[CAMIONES]\n");
    for(k = 0; k < numCamiones; k++) {
        fprintf(pArchivo, "K%03d;T=%d;C=%s\n", k + 1, camionT[k], pool[k]);
    }

    fclose(pArchivo);

    free(buqueT);
    free(buqueNumCont);
    free(camionT);
    free(pool);

    return EXITO;
}

/*
Ordena el array 'tiempos' de menor a mayor usando insercion directa.
Garantiza que el primer elemento en llegar sea siempre el que tiene el menor T.
*/
void ordenar_tiempos(int* tiempos, int cantidad) {

    int i, j;
    int actual;

    for(i = 1; i < cantidad; i++) {

        actual = tiempos[i];
        j = i - 1;

        while(j >= 0 && tiempos[j] > actual) {
            tiempos[j + 1] = tiempos[j];
            j--;
        }

        tiempos[j + 1] = actual;
    }
}
