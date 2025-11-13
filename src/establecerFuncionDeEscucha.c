/**
 * @file establecerFuncionDeEscucha.c
 * @brief Implementación de la función para establecer un callback de escucha
 */

#include "../include/ProcesoPar.h"
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
    #include <unistd.h>
#endif

/* Función del hilo que escucha mensajes del proceso hijo */
#ifdef _WIN32
DWORD WINAPI hiloEscucha(LPVOID param) {
    ProcesoPar_t *pp = (ProcesoPar_t*)param;
    char buffer[4096];
    DWORD bytesLeidos;

    while (pp->activo && pp->funcionEscucha != NULL) {
        /* Leer de la tubería de entrada */
        BOOL resultado = ReadFile(
            pp->hTuberiaEntrada,
            buffer,
            sizeof(buffer) - 1,
            &bytesLeidos,
            NULL
        );

        if (resultado && bytesLeidos > 0) {
            buffer[bytesLeidos] = '\0';  /* Terminar la cadena */
            /* Llamar a la función de escucha del usuario */
            pp->funcionEscucha(buffer, bytesLeidos);
        } else {
            /* Error o fin de archivo */
            break;
        }
    }

    return 0;
}
#else
void* hiloEscucha(void* param) {
    ProcesoPar_t *pp = (ProcesoPar_t*)param;
    char buffer[4096];
    ssize_t bytesLeidos;

    while (pp->activo && pp->funcionEscucha != NULL) {
        /* Leer de la tubería de entrada
         * pipeEntrada[0] es el extremo de lectura que usa el padre
         */
        bytesLeidos = read(pp->pipeEntrada[0], buffer, sizeof(buffer) - 1);

        if (bytesLeidos > 0) {
            buffer[bytesLeidos] = '\0';  /* Terminar la cadena */
            /* Llamar a la función de escucha del usuario */
            pp->funcionEscucha(buffer, bytesLeidos);
        } else if (bytesLeidos == 0) {
            /* Fin de archivo - el hijo cerró su extremo */
            break;
        } else {
            /* Error en la lectura */
            break;
        }
    }

    return NULL;
}
#endif

/**
 * @brief Establece la función de escucha para mensajes entrantes
 */
Estado_t establecerFuncionDeEscucha(
    ProcesoPar_t *procesoPar,
    Estado_t (*f)(const char *, int)
) {
    /* Validar parámetros */
    if (procesoPar == NULL || f == NULL) {
        return E_PAR_INC;
    }

    /* Verificar que el proceso esté activo */
    if (!procesoPar->activo) {
        return E_PROCESO_INACT;
    }

    /* Guardar la función de escucha */
    procesoPar->funcionEscucha = f;

#ifdef _WIN32
    /* ========================================
     * IMPLEMENTACIÓN PARA WINDOWS
     * ======================================== */
    
    /* Crear un hilo que escuche mensajes del proceso hijo */
    procesoPar->hHiloEscucha = CreateThread(
        NULL,              /* Atributos de seguridad por defecto */
        0,                 /* Tamaño de pila por defecto */
        hiloEscucha,       /* Función del hilo */
        procesoPar,        /* Parámetro para la función */
        0,                 /* Flags de creación */
        NULL               /* No necesitamos el ID del hilo */
    );

    if (procesoPar->hHiloEscucha == NULL) {
        return E_CREAR_HILO;
    }

#else
    /* ========================================
     * IMPLEMENTACIÓN PARA LINUX
     * ======================================== */
    
    /* Crear un hilo que escuche mensajes del proceso hijo */
    int resultado = pthread_create(
        &procesoPar->hiloEscucha,  /* ID del hilo */
        NULL,                       /* Atributos por defecto */
        hiloEscucha,               /* Función del hilo */
        procesoPar                 /* Parámetro para la función */
    );

    if (resultado != 0) {
        return E_CREAR_HILO;
    }

    /* Hacer el hilo "detached" para que se limpie automáticamente */
    pthread_detach(procesoPar->hiloEscucha);

#endif

    return E_OK;
}
