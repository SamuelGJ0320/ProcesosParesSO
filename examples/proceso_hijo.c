/**
 * @file proceso_hijo.c
 * @brief Programa de ejemplo que actúa como proceso hijo
 * 
 * Este programa:
 * - Lee mensajes desde stdin (enviados por el padre)
 * - Procesa los mensajes
 * - Envía respuestas a stdout (que el padre leerá)
 * 
 * NOTA: En Windows, los mensajes de debug se escriben en hijo_debug.log
 *       En Linux, se escriben directamente en stderr (consola)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
    #include <process.h>
    #include <windows.h>
    FILE* log_file = NULL;
    #define DEBUG_INIT() log_file = fopen("hijo_debug.log", "w"); \
                         if (log_file) setvbuf(log_file, NULL, _IONBF, 0)
    #define DEBUG_PRINT(fmt, ...) if (log_file) { fprintf(log_file, fmt, ##__VA_ARGS__); fflush(log_file); }
    #define DEBUG_CLOSE() if (log_file) fclose(log_file)
    #define GETPID() _getpid()
#else
    #include <unistd.h>
    #define DEBUG_INIT()
    #define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr)
    #define DEBUG_CLOSE()
    #define GETPID() getpid()
#endif

int main(int argc, char *argv[]) {
    (void)argc;  /* Parámetro no usado */
    (void)argv;  /* Parámetro no usado */
    char buffer[1024];
    
    #ifdef _WIN32
    /* En Windows, configurar stdin/stdout en modo binario */
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    #endif
    
    /* Inicializar sistema de debug */
    DEBUG_INIT();
    
    /* Mensaje de inicio */
    DEBUG_PRINT("[HIJO] Proceso hijo iniciado, PID: %d\n", GETPID());
    DEBUG_PRINT("[HIJO] Esperando mensajes...\n");

    /* Bucle principal: leer mensajes del padre */
    while (1) {
        /* Leer desde stdin (conectado al padre mediante tubería) */
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            /* EOF o error - el padre cerró la tubería */
            DEBUG_PRINT("[HIJO] Fin de entrada detectado, terminando...\n");
            break;
        }

        /* Eliminar el salto de línea si existe */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }

        /* Log del mensaje recibido */
        DEBUG_PRINT("[HIJO] Mensaje recibido: '%s' (%zu bytes)\n", buffer, len);

        /* Generar una respuesta según el mensaje recibido */
        char respuesta[1024];
        
        if (strcmp(buffer, "HOLA") == 0) {
            strcpy(respuesta, "HOLA PADRE\n");
        } else if (strcmp(buffer, "PING") == 0) {
            strcpy(respuesta, "PONG\n");
        } else if (strcmp(buffer, "SALIR") == 0) {
            strcpy(respuesta, "ADIOS\n");
            /* Enviar respuesta y terminar */
            fputs(respuesta, stdout);
            fflush(stdout);
            DEBUG_PRINT("[HIJO] Respuesta enviada: '%s'\n", respuesta);
            DEBUG_PRINT("[HIJO] Comando SALIR recibido, terminando...\n");
            break;
        } else {
            /* Eco: devolver el mensaje con prefijo */
            snprintf(respuesta, sizeof(respuesta), "ECO: %s\n", buffer);
        }

        /* Enviar respuesta al padre a través de stdout */
        fputs(respuesta, stdout);
        fflush(stdout);  /* Importante: forzar el envío inmediato */
        
        DEBUG_PRINT("[HIJO] Respuesta enviada: '%s'\n", respuesta);
    }

    DEBUG_PRINT("[HIJO] Proceso hijo terminando normalmente\n");
    DEBUG_CLOSE();
    
    return 0;
}
