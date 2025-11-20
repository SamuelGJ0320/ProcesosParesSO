/**
 * @file
 * @brief
 */

#ifndef PROCESOPAR_H
#define PROCESOPAR_H

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <pthread.h>
#endif

#include <stddef.h>

/*
 * DEFINICIÓN DE TIPOS
*/

/**
 * @brief Tipo para códigos de estado/error
 */
typedef unsigned int Estado_t;

/**
 * @brief Tipo de función callback para procesar mensajes entrantes
 * @param mensaje Puntero al mensaje recibido
 * @param longitud Longitud del mensaje en bytes
 * @return Estado_t código de estado (E_OK si todo va bien)
 */
typedef Estado_t (*FuncionEscucha_t)(const char *mensaje, int longitud);

/**
 * @brief Estructura de un proceso par
 */
typedef struct ProcesoPar {
    #ifdef _WIN32
        /* === WINDOWS === */
        HANDLE hProceso;
        HANDLE hHilo;
        HANDLE hTuberiaEntrada;
        HANDLE hTuberiaSalida;
        HANDLE hHiloEscucha;
        DWORD dwProcesoId;   
    #else
        /* === LINUX === */
        pid_t pid;
        int pipeEntrada[2];
        int pipeSalida[2];
        pthread_t hiloEscucha;
    #endif
    
    /* === AMBOS SISTEMAS === */
    FuncionEscucha_t funcionEscucha;
    int activo;
} ProcesoPar_t;


#define E_OK            0
#define E_PAR_INC       1
#define E_NO_MEMORIA    2
#define E_CREAR_PIPE    3
#define E_CREAR_PROCESO 4
#define E_ENVIO_FALLO   5
#define E_PROCESO_INACT 6
#define E_CREAR_HILO    7


// FUNCIONES:
/**
 * @brief Lanza un nuevo proceso par (proceso hijo)
 * 
 * Crea un proceso hijo y establece comunicación bidireccional mediante tuberías.
 * 
 * @param nombreArchivoEjecutable Ruta al ejecutable del proceso hijo
 * @param listaLineaComando Array de argumentos (terminado en NULL). El primer argumento debe ser el nombre del programa
 * @param procesoPar Puntero a puntero donde se almacenará la estructura creada
 * @return Estado_t E_OK si tiene éxito, código de error en caso contrario
 * 
 * Ejemplo de uso:
 * @code
 * const char* args[] = {"programa_hijo", "arg1", "arg2", NULL};
 * ProcesoPar_t* pp = NULL;
 * Estado_t estado = lanzarProcesoPar("./programa_hijo", args, &pp);
 * @endcode
 */
Estado_t lanzarProcesoPar(
    const char *nombreArchivoEjecutable,
    const char **listaLineaComando,
    ProcesoPar_t **procesoPar
);

/**
 * @brief Destruye un proceso par
 * 
 * Termina el proceso hijo, cierra todas las tuberías y libera recursos.
 * 
 * @param procesoPar Puntero a la estructura del proceso par
 * @return Estado_t E_OK si tiene éxito, código de error en caso contrario
 */
Estado_t destruirProcesoPar(ProcesoPar_t *procesoPar);

/**
 * @brief Envía un mensaje al proceso par (hijo)
 * 
 * Escribe un mensaje en la tubería de salida hacia el proceso hijo.
 * 
 * @param procesoPar Puntero a la estructura del proceso par
 * @param mensaje Puntero al mensaje a enviar
 * @param longitud Longitud del mensaje en bytes
 * @return Estado_t E_OK si tiene éxito, código de error en caso contrario
 */
Estado_t enviarMensajeProcesoPar(
    ProcesoPar_t *procesoPar,
    const char *mensaje,
    int longitud
);

/**
 * @brief Establece la función de escucha para mensajes entrantes
 * 
 * Configura una función callback que será llamada cada vez que el proceso
 * hijo envíe un mensaje al proceso padre.
 * 
 * @param procesoPar Puntero a la estructura del proceso par
 * @param f Puntero a la función de escucha
 * @return Estado_t E_OK si tiene éxito, código de error en caso contrario
 * 
 * La función de escucha debe tener la siguiente firma:
 * @code
 * Estado_t miFuncionEscucha(const char* mensaje, int longitud) {
 *     // Procesar el mensaje aquí
 *     return E_OK;
 * }
 * @endcode
 */
Estado_t establecerFuncionDeEscucha(
    ProcesoPar_t *procesoPar,
    Estado_t (*f)(const char *, int)
);

#endif