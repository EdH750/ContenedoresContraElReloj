#include "operador.h"
#include "abb.h"

/*
Compara dos tOperador por nombre para usarlos como clave del ABB.
*/
int cmp_operador_por_nombre(const void* a, const void* b) {
    const tOperador* oa = (const tOperador*)a;
    const tOperador* ob = (const tOperador*)b;
    return strcmp(oa->nombre, ob->nombre);
}

/*
Compara una tEntradaRanking con un int* (el id del operador).
Se usa con buscar_en_lista para localizar la entrada acumulada de un operador.
*/
int cmp_entrada_ranking_por_id(const void* dato, const void* clave) {
    return ((const tEntradaRanking*)dato)->idOperador - *(const int*)clave;
}

/*
Criterio de orden para el ranking:
1. Mayor puntuacion total (descendente).
2. Menor jornadas jugadas como desempate (ascendente).
3. Mayor entregas totales como segundo desempate (descendente).
*/
int cmp_ranking(const void* a, const void* b) {

    const tEntradaRanking* ra = (const tEntradaRanking*)a;
    const tEntradaRanking* rb = (const tEntradaRanking*)b;

    if(ra->totalPuntuacion != rb->totalPuntuacion) {
        return rb->totalPuntuacion - ra->totalPuntuacion;
    }

    if(ra->jornadasJugadas != rb->jornadasJugadas) {
        return ra->jornadasJugadas - rb->jornadasJugadas;
    }

    return rb->totalEntregados - ra->totalEntregados;
}

/*
Carga el archivo operadores.bin en el ABB indexado por nombre.
Si el archivo no existe, el ABB queda vacio.
*/
void cargar_abb_operadores(tABB* pa, const char* archivo) {

    FILE* pf;
    tOperador op;

    crear_abb(pa);

    pf = fopen(archivo, "rb");
    if(!pf) return;

    fread(&op, sizeof(tOperador), 1, pf);
    while(!feof(pf)) {
        insertar_abb(pa, &op, sizeof(tOperador), cmp_operador_por_nombre);
        fread(&op, sizeof(tOperador), 1, pf);
    }

    fclose(pf);
}

/*
Devuelve la cantidad de registros de operadores.bin (para asignar el proximo id).
Devuelve 0 si el archivo no existe.
*/
int contar_registros_operadores(const char* archivo) {

    FILE* pf;
    int cant;

    pf = fopen(archivo, "rb");
    if(!pf) return 0;

    fseek(pf, 0, SEEK_END);
    cant = (int)(ftell(pf) / sizeof(tOperador));
    fclose(pf);

    return cant;
}

/*
Guarda un nuevo operador al final de operadores.bin.
Crea el archivo si no existe.
Devuelve EXITO si se escribio correctamente, ERROR en caso contrario.
*/
int persistir_operador(const tOperador* pOp) {

    FILE* pf;

    pf = fopen(OPERADORES_ARCHIVO, "ab");
    if(!pf) return ERROR;

    fwrite(pOp, sizeof(tOperador), 1, pf);
    fclose(pf);

    return EXITO;
}

/*
Lee una linea del teclado y la deja en nombre (sin salto de linea, maximo MAX_NOMBRE_OPERADOR).
Si la linea no cabia en el buffer descarta el resto para no contaminar stdin.
Devuelve EXITO si leyo algo no vacio, ERROR si el nombre quedo vacio.
*/
int leer_nombre(char* nombre) {

    int c;

    printf("Nombre del operador: ");

    if(fgets(nombre, MAX_NOMBRE_OPERADOR, stdin) == NULL) {
        nombre[0] = '\0';
        return ERROR;
    }

    if(strchr(nombre, '\n') == NULL) {
        while((c = getchar()) != '\n' && c != EOF);
    }

    nombre[strcspn(nombre, "\r\n")] = '\0';

    return nombre[0] != '\0';
}

/*
Pide el nombre al operador, lo busca en operadores.bin via ABB y llena *pOperador.
Si no existe lo registra como nuevo asignandole el proximo id disponible.
Devuelve EXITO si todo salio bien, ERROR si el nombre quedo vacio o no se pudo persistir.
*/
int iniciar_sesion_operador(tOperador* pOperador) {

    tABB indice;
    tOperador busqueda;
    tOperador* encontrado;
    char nombre[MAX_NOMBRE_OPERADOR];

    cargar_abb_operadores(&indice, OPERADORES_ARCHIVO);

    do {
        if(!leer_nombre(nombre)) {
            printf("El nombre no puede estar vacio. Ingrese nuevamente.\n");
        }
    } while(nombre[0] == '\0');

    strncpy(busqueda.nombre, nombre, MAX_NOMBRE_OPERADOR - 1);
    busqueda.nombre[MAX_NOMBRE_OPERADOR - 1] = '\0';
    busqueda.id = 0;

    encontrado = (tOperador*)buscar_abb(&indice, &busqueda, cmp_operador_por_nombre);

    if(encontrado) {
        *pOperador = *encontrado;
    } else {
        pOperador->id = contar_registros_operadores(OPERADORES_ARCHIVO) + 1;
        strncpy(pOperador->nombre, nombre, MAX_NOMBRE_OPERADOR - 1);
        pOperador->nombre[MAX_NOMBRE_OPERADOR - 1] = '\0';

        if(!persistir_operador(pOperador)) {
            printf("Error: no se pudo registrar el operador en el archivo.\n");
            vaciar_abb(&indice);
            return ERROR;
        }
    }

    vaciar_abb(&indice);

    return EXITO;
}

/*
Escribe los resultados de la jornada en jornadas.bin.
El idJornada se calcula como la cantidad de registros ya existentes + 1.
Devuelve EXITO si se escribio correctamente, ERROR en caso contrario.
*/
int guardar_jornada(const tJornada* pJornada, const tOperador* pOperador) {

    FILE* pf;
    tRegistroJornada registro;
    long cantReg;

    pf = fopen(JORNADAS_ARCHIVO, "a+b");
    if(!pf) {
        printf("Advertencia: no se pudo guardar la jornada en %s\n", JORNADAS_ARCHIVO);
        return ERROR;
    }

    fseek(pf, 0, SEEK_END);
    cantReg = ftell(pf) / sizeof(tRegistroJornada);

    registro.idJornada              = (int)cantReg + 1;
    registro.idOperador             = pOperador->id;
    strncpy(registro.nombreOperador, pOperador->nombre, MAX_NOMBRE_OPERADOR - 1);
    registro.nombreOperador[MAX_NOMBRE_OPERADOR - 1] = '\0';
    registro.puntuacion             = pJornada->resumen.puntuacion;
    registro.contenedoresEntregados = pJornada->resumen.contenedoresEntregados;
    registro.buquesDescargados      = pJornada->resumen.buquesDescargados;
    registro.camionesPendientes     = pJornada->resumen.camionesPendientes;
    registro.reubicaciones          = pJornada->resumen.reubicaciones;
    registro.tiempoUtilizado        = pJornada->resumen.tiempoUtilizado;
    registro.motivoFin              = (int)pJornada->resumen.motivoFin;

    fwrite(&registro, sizeof(tRegistroJornada), 1, pf);
    fclose(pf);

    return EXITO;
}

/*
Lee jornadas.bin, acumula resultados por operador en una lista, la ordena
con cmp_ranking y la muestra desenlistando de a uno.
*/
void mostrar_ranking(void) {

    FILE* pf;
    tRegistroJornada reg;
    tLista lista;
    tEntradaRanking* pEnt;
    tEntradaRanking nueva;
    tEntradaRanking entrada;
    int pos;

    pf = fopen(JORNADAS_ARCHIVO, "rb");
    if(!pf) {
        printf("  Aun no hay jornadas registradas.\n");
        return;
    }

    crear_lista(&lista);

    fread(&reg, sizeof(tRegistroJornada), 1, pf);
    while(!feof(pf)) {

        pEnt = (tEntradaRanking*)buscar_en_lista(&lista, &reg.idOperador, cmp_entrada_ranking_por_id);

        if(pEnt) {
            pEnt->totalPuntuacion        += reg.puntuacion;
            pEnt->jornadasJugadas++;
            pEnt->totalEntregados        += reg.contenedoresEntregados;
            pEnt->totalBuquesDescargados += reg.buquesDescargados;
        } else {
            nueva.idOperador             = reg.idOperador;
            strncpy(nueva.nombre, reg.nombreOperador, MAX_NOMBRE_OPERADOR - 1);
            nueva.nombre[MAX_NOMBRE_OPERADOR - 1] = '\0';
            nueva.totalPuntuacion        = reg.puntuacion;
            nueva.jornadasJugadas        = PRIMERA_JORNADA;
            nueva.totalEntregados        = reg.contenedoresEntregados;
            nueva.totalBuquesDescargados = reg.buquesDescargados;

            enlistar_final(&lista, &nueva, sizeof(tEntradaRanking));
        }

        fread(&reg, sizeof(tRegistroJornada), 1, pf);
    }

    fclose(pf);

    if(lista_vacia(&lista)) {
        printf("  No se pudo leer el historial de jornadas.\n");
        return;
    }

    ordenar_lista(&lista, cmp_ranking);

    printf("\n");
    printf("#==========================================================#\n");
    printf("  RANKING DE OPERADORES\n");
    printf("#==========================================================#\n");
    printf("  %-4s %-22s %10s %9s %9s\n",
           "Pos", "Operador", "Puntuacion", "Jornadas", "Entregas");
    printf("  ----+----------------------+----------+---------+---------\n");

    pos = PRIMERA_POSICION;
    while(!lista_vacia(&lista)) {
        desenlistar_inicio(&lista, &entrada, sizeof(tEntradaRanking));
        printf("  %-4d %-22s %10d %9d %9d\n",
               pos++,
               entrada.nombre,
               entrada.totalPuntuacion,
               entrada.jornadasJugadas,
               entrada.totalEntregados);
    }

    printf("#==========================================================#\n");
    printf("  Criterios: 1) Mayor puntuacion total\n");
    printf("             2) Menor jornadas jugadas (desempate)\n");
    printf("             3) Mayor entregas totales (desempate)\n");
    printf("#==========================================================#\n");
}
