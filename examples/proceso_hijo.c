#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
    #include <process.h>
    #include <windows.h>
#else
    #include <unistd.h>
#endif

int main(int argc, char *argv[]) {
    (void)argc;  /* Parámetro no usado */
    (void)argv;  /* Parámetro no usado */
    char buffer[1024];
    
    #ifdef _WIN32
    /* En Windows, se configura stdin/stdout en modo binario */
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    #endif

    /* Leer mensajes del padre */
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }

        /* Verificar si es comando de salida */
        if (strcmp(buffer, "TERMINAR_PROCESO") == 0) {
            break;
        }
        
        /* Verificar si es una solicitud para que el hijo envíe una respuesta específica */
        else if (strncmp(buffer, "ENVIAR:", 7) == 0) {
            char* mensaje_respuesta = buffer + 7;
            printf("%s\n", mensaje_respuesta);
            fflush(stdout);
        }
        else {
            /* Mostrar el mensaje recibido del padre */
            printf("[HIJO] Mensaje recibido: '%s' (%zu bytes)\n", buffer, len);
            fflush(stdout);
        }
    }

    return 0;
}
