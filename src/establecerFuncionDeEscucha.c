/**
* @file
* @brief
*/

#include "../include/ProcesoPar.h"
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
    #include <unistd.h>
#endif

// Función del hilo que escucha mensajes del proceso hijo
#ifdef _WIN32
DWORD WINAPI hiloEscucha(LPVOID param) {
    ProcesoPar_t *pp = (ProcesoPar_t*)param;
    char buffer[4096];
    DWORD bytesLeidos;

    while (pp->activo && pp->funcionEscucha != NULL) {
        BOOL resultado = ReadFile(
            pp->hTuberiaEntrada,
            buffer,
            sizeof(buffer) - 1,
            &bytesLeidos,
            NULL
        );

        if (resultado && bytesLeidos > 0) {
            buffer[bytesLeidos] = '\0';
            pp->funcionEscucha(buffer, bytesLeidos);
        } else {
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
        bytesLeidos = read(pp->pipeEntrada[0], buffer, sizeof(buffer) - 1);

        if (bytesLeidos > 0) {
            buffer[bytesLeidos] = '\0';
            pp->funcionEscucha(buffer, bytesLeidos);
        } else if (bytesLeidos == 0) {
            break;
        } else {
            break;
        }
    }

    return NULL;
}
#endif

/**
* @brief
*/
Estado_t establecerFuncionDeEscucha(
    ProcesoPar_t *procesoPar,
    Estado_t (*f)(const char *, int)
) {
    if (procesoPar == NULL || f == NULL) {
        return E_PAR_INC;
    }

    if (!procesoPar->activo) {
        return E_PROCESO_INACT;
    }
    procesoPar->funcionEscucha = f;

#ifdef _WIN32
// IMPLEMENTACIÓN PARA WINDOWS:
    
    // Crear un hilo que escuche mensajes del proceso hijo
    procesoPar->hHiloEscucha = CreateThread(
        NULL,
        0,
        hiloEscucha,
        procesoPar,
        0,
        NULL
    );

    if (procesoPar->hHiloEscucha == NULL) {
        return E_CREAR_HILO;
    }

#else
// IMPLEMENTACIÓN PARA LINUX
    
    // Crear un hilo que escuche mensajes del proceso hijo
    int resultado = pthread_create(
        &procesoPar->hiloEscucha,
        NULL,
        hiloEscucha,
        procesoPar
    );

    if (resultado != 0) {
        return E_CREAR_HILO;
    }

    /* Hacer el hilo "detached" para que se limpie automáticamente */
    pthread_detach(procesoPar->hiloEscucha);

#endif

    return E_OK;
}