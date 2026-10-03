#include "ContenedoresContraReloj.h"

int main()
{

    int opcion;

    tConfiguracion configuracion;

    /* Semilla para numeros aleatorios, se llama una sola vez al inicio */
    srand((unsigned int)time(NULL));

    menu();

    do {

        seleccionar_opcion(&opcion);

        switch(opcion) {

            case 1:

                if(procesar_configuracion(CONFIG_ARCHIVO, &configuracion) == 1) {
                    printf("Archivo de configuracion procesado correctamente\n");
                    generar_archivo_puerto(&configuracion);
                } else {
                    printf("Error al procesar el archivo de configuracion\n");
                }

                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                printf("Saliendo...\n");
                break;
            default:
                printf("Opcion no valida, ingrese una correcta\n");
                break;

        }

    }while(opcion != 4);

    return 0;
}
