/**
 * @file ProcesoPar.h
 * @brief Biblioteca para crear y gestionar procesos pares con comunicación bidireccional
 * 
 * Esta biblioteca permite crear un proceso hijo desde un proceso padre y establecer
 * comunicación full-duplex a través de tuberías (pipes).
 * 
 * Soporta tanto Windows como Linux mediante compilación condicional.
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

/* ============================================================================
 * DEFINICIÓN DE TIPOS
 * ============================================================================ */

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
 * @brief Estructura que representa un proceso par
 * 
 * Contiene toda la información necesaria para gestionar un proceso hijo
 * y su comunicación bidireccional con el proceso padre.
 */
typedef struct ProcesoPar {
    #ifdef _WIN32
        /* === WINDOWS === */
        HANDLE hProceso;              /* Handle del proceso hijo */
        HANDLE hHilo;                 /* Handle del hilo principal del proceso hijo */
        HANDLE hTuberiaEntrada;       /* Handle para leer desde el proceso hijo */
        HANDLE hTuberiaSalida;        /* Handle para escribir al proceso hijo */
        HANDLE hHiloEscucha;          /* Handle del hilo de escucha */
        DWORD dwProcesoId;            /* ID del proceso hijo */
    #else
        /* === LINUX === */
        pid_t pid;                    /* ID del proceso hijo */
        int pipeEntrada[2];           /* Tubería para leer desde el hijo: [0]=lectura, [1]=escritura */
        int pipeSalida[2];            /* Tubería para escribir al hijo: [0]=lectura, [1]=escritura */
        pthread_t hiloEscucha;        /* Hilo que escucha mensajes del proceso hijo */
    #endif
    
    /* === COMÚN A AMBOS SISTEMAS === */
    FuncionEscucha_t funcionEscucha;  /* Función callback para procesar mensajes */
    int activo;                       /* 1 si el proceso está activo, 0 si no */
} ProcesoPar_t;

/* ============================================================================
 * CÓDIGOS DE ESTADO
 * ============================================================================ */

#define E_OK            0    /* Operación exitosa */
#define E_PAR_INC       1    /* Parámetro incorrecto */
#define E_NO_MEMORIA    2    /* No hay memoria disponible */
#define E_CREAR_PIPE    3    /* Error al crear tubería */
#define E_CREAR_PROCESO 4    /* Error al crear proceso hijo */
#define E_ENVIO_FALLO   5    /* Error al enviar mensaje */
#define E_PROCESO_INACT 6    /* El proceso no está activo */
#define E_CREAR_HILO    7    /* Error al crear hilo de escucha */

/* ============================================================================
 * PROTOTIPOS DE FUNCIONES
 * ============================================================================ */

/**
 * @brief Lanza un nuevo proceso par (proceso hijo)
 * 
 * Crea un proceso hijo y establece comunicación bidireccional mediante tuberías.
 * 
 * @param nombreArchivoEjecutable Ruta al ejecutable del proceso hijo
 * @param listaLineaComando Array de argumentos (terminado en NULL). El primer
 *                          argumento debe ser el nombre del programa
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

#endif /* PROCESOPAR_H */
