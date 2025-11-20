#include "../include/ProcesoPar.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

/**
 * Función de escucha para mensajes del hijo.
 * Se llamará cada vez que el proceso hijo envíe algo al padre.
 */
static Estado_t funcionEscucha(const char *mensaje, int longitud) {
    printf("\n[PADRE] Mensaje desde o hijo (%d bytes): \"%.*s\"\n",
           longitud, longitud, mensaje);
    return E_OK;
}

int main(void) {
    int opcion;
    Estado_t estado;
    ProcesoPar_t *proceso = NULL;   /* será rellenado por lanzarProcesoPar */

    printf("===== MENÚ PROCESO PADRE =====\n");

    /* IMPORTANTE:
     * Este programa se ejecuta desde la carpeta examples/ (make run lo hace así),
     * por eso la ruta al ejecutable del hijo es ./proceso_hijo
     */
    const char *lineaComando[] = {
        "./proceso_hijo",
        NULL
    };

    /* Lanzar proceso hijo */
    estado = lanzarProcesoPar("./proceso_hijo", lineaComando, &proceso);
    if (estado != E_OK) {
        fprintf(stderr,
                "[ERROR] No se pudo lanzar el proceso hijo. Código: %u\n",
                estado);
        return EXIT_FAILURE;
    }

    /* DEBUG: mostrar pid y fds en la estructura (Linux) */
#ifndef _WIN32
    fprintf(stderr, "[DEBUG][padre_menu] hijo pid=%d pipeEntrada[0]=%d pipeEntrada[1]=%d pipeSalida[0]=%d pipeSalida[1]=%d\n",
            (int)proceso->pid,
            proceso->pipeEntrada[0], proceso->pipeEntrada[1],
            proceso->pipeSalida[0], proceso->pipeSalida[1]);
#endif

    /* Establecer función de escucha para respuestas del hijo */
    estado = establecerFuncionDeEscucha(proceso, funcionEscucha);
    if (estado != E_OK) {
        fprintf(stderr,
                "[ERROR] No se pudo establecer la función de escucha. Código: %u\n",
                estado);
        destruirProcesoPar(proceso);
        return EXIT_FAILURE;
    }

    do {
        printf("\n¿Qué deseas hacer?\n");
        printf("1. Enviar mensaje al proceso hijo\n");
        printf("2. Salir\n");
        printf("Opción: ");

        if (scanf("%d", &opcion) != 1) {
            fprintf(stderr, "[ERROR] Entrada inválida\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            opcion = 0;
            continue;
        }

        if (opcion == 1) {
            char mensaje[256];

            printf("Escribe tu mensaje (ej. HOLA, PING, SALIR): ");

            /* Limpiar el salto de línea que deja scanf */
            int c;
            while ((c = getchar()) == '\n') { /* limpiar salto previo */ }
            if (c != EOF) {
                ungetc(c, stdin);
            }

            /* Leer la línea completa, incluyendo el '\n' */
            if (fgets(mensaje, sizeof(mensaje), stdin) == NULL) {
                fprintf(stderr, "[ERROR] No se pudo leer el mensaje.\n");
                continue;
            }

            /* NO quitamos el '\n': el hijo lee líneas con fgets */
            int len = (int)strlen(mensaje);

            estado = enviarMensajeProcesoPar(proceso, mensaje, len);
            if (estado != E_OK) {
                fprintf(stderr,
                        "[ERROR] No se pudo enviar el mensaje. Código: %u\n",
                        estado);
            } else {
                printf("[INFO] Mensaje enviado correctamente.\n");
            }
        }

    } while (opcion != 2);

    /* Destruir proceso par */
    estado = destruirProcesoPar(proceso);
    if (estado != E_OK) {
        fprintf(stderr,
                "[ERROR] Error al destruir el proceso par. Código: %u\n",
                estado);
    }

    printf("\nProceso padre finalizado.\n");
    return EXIT_SUCCESS;
}