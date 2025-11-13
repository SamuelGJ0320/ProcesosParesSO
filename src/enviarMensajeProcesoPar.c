/**
 * @file enviarMensajeProcesoPar.c
 * @brief Implementación de la función para enviar mensajes al proceso hijo
 */

#include "../include/ProcesoPar.h"
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

/**
 * @brief Envía un mensaje al proceso par (hijo)
 */
Estado_t enviarMensajeProcesoPar(
    ProcesoPar_t *procesoPar,
    const char *mensaje,
    int longitud
) {
    /* Validar parámetros */
    if (procesoPar == NULL || mensaje == NULL || longitud <= 0) {
        return E_PAR_INC;
    }

    /* Verificar que el proceso esté activo */
    if (!procesoPar->activo) {
        return E_PROCESO_INACT;
    }

#ifdef _WIN32
    /* ========================================
     * IMPLEMENTACIÓN PARA WINDOWS
     * ======================================== */
    
    DWORD bytesEscritos;
    BOOL resultado;

    /* Escribir en la tubería de salida */
    resultado = WriteFile(
        procesoPar->hTuberiaSalida,  /* Handle de la tubería */
        mensaje,                      /* Buffer con el mensaje */
        longitud,                     /* Número de bytes a escribir */
        &bytesEscritos,              /* Bytes escritos */
        NULL                         /* Sin operación asíncrona */
    );

    if (!resultado || bytesEscritos != (DWORD)longitud) {
        return E_ENVIO_FALLO;
    }

    /* Forzar el envío del buffer (flush) */
    FlushFileBuffers(procesoPar->hTuberiaSalida);

#else
    /* ========================================
     * IMPLEMENTACIÓN PARA LINUX
     * ======================================== */
    
    ssize_t bytesEscritos;

    /* Escribir en la tubería de salida
     * pipeSalida[1] es el extremo de escritura que usa el padre
     */
    bytesEscritos = write(procesoPar->pipeSalida[1], mensaje, longitud);

    if (bytesEscritos == -1 || bytesEscritos != longitud) {
        return E_ENVIO_FALLO;
    }

#endif

    return E_OK;
}
