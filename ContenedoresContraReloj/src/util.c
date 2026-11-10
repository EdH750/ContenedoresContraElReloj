#include "util.h"

/*
Convierte una cadena a entero de forma estricta usando strtol.
Devuelve EXITO si la conversion fue exitosa, ERROR en caso contrario.
Rechaza cadenas vacias, con letras, con basura al final o fuera del rango de int.
*/
int leer_entero(const char* texto, int* valor) {

    char* fin;
    long numero;

    errno = 0;
    numero = strtol(texto, &fin, BASE_DECIMAL);

    if(fin == texto) {
        return ERROR;
    }

    while(isspace((unsigned char)*fin)) {
        fin++;
    }

    if(*fin != '\0') {
        return ERROR;
    }

    if(errno == ERANGE || numero < INT_MIN || numero > INT_MAX) {
        return ERROR;
    }

    *valor = (int)numero;

    return EXITO;
}
