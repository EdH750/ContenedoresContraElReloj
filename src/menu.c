#include "menu.h"

/*
Muestra las opciones del menu principal en pantalla.
*/
void menu(void) {

    printf("============================PUERTO Y CONTENEDORES============================\n");
    printf("1) Comenzar jornada\n");
    printf("2) Ver ranking\n");
    printf("3) Salir\n");
}

/*
Lee una linea del teclado y deja en *opcion el numero elegido.
Si la entrada es invalida deja OPCION_INVALIDA.
Si se detecta EOF deja OPCION_SALIR para no quedar en un loop infinito.
*/
void seleccionar_opcion(int* opcion) {

    char buffer[MAX_BUFFER];
    int c;

    printf("Ingrese opcion: ");

    if(fgets(buffer, MAX_BUFFER, stdin) == NULL) {
        *opcion = OPCION_SALIR;
        return;
    }

    if(strchr(buffer, '\n') == NULL) {
        while((c = getchar()) != '\n' && c != EOF);
        buffer[0] = '\0';
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';

    if(validar_opcion(buffer, opcion) == ERROR) {
        *opcion = OPCION_INVALIDA;
    }
}

/*
Verifica que 'texto' represente un entero valido dentro del rango de opciones del menu.
Devuelve EXITO si es valido y deja el valor en *opcion, ERROR en caso contrario.
Rechaza entradas vacias, con letras, con signos o fuera del rango OPCION_MINIMA-OPCION_MAXIMA.
*/
int validar_opcion(const char* texto, int* opcion) {

    if(texto[0] == '\0') {
        return ERROR;
    }

    if(strspn(texto, "0123456789") != strlen(texto)) {
        return ERROR;
    }

    if(leer_entero(texto, opcion) == ERROR) {
        return ERROR;
    }

    return *opcion >= OPCION_MINIMA && *opcion <= OPCION_MAXIMA;
}
