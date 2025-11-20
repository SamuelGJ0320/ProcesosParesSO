# Makefile para Linux
# Compila la biblioteca ProcesoPar y los programas de ejemplo

# Compilador
CC = gcc

# Flags de compilación
CFLAGS = -Wall -Wextra -I./include -pthread

# Directorios
SRC_DIR = src
INC_DIR = include
LIB_DIR = lib
EXAMPLES_DIR = examples

# Archivos fuente de la biblioteca
LIB_SOURCES = $(SRC_DIR)/lanzarProcesoPar.c \
              $(SRC_DIR)/enviarMensajeProcesoPar.c \
              $(SRC_DIR)/establecerFuncionDeEscucha.c \
              $(SRC_DIR)/destruirProcesoPar.c

# Archivos objeto de la biblioteca
LIB_OBJECTS = $(LIB_DIR)/lanzarProcesoPar.o \
              $(LIB_DIR)/enviarMensajeProcesoPar.o \
              $(LIB_DIR)/establecerFuncionDeEscucha.o \
              $(LIB_DIR)/destruirProcesoPar.o

# Nombre de la biblioteca estática
LIBRARY = $(LIB_DIR)/libprocesopar.a

# Ejecutables de ejemplo
EJEMPLO_HIJO           = $(EXAMPLES_DIR)/proceso_hijo
EJEMPLO_PADRE_COMPLETO = $(EXAMPLES_DIR)/proceso_padre_completo

# Target por defecto: compilar todo
all: $(LIBRARY) $(EJEMPLO_HIJO) $(EJEMPLO_PADRE_COMPLETO)
	@echo ""
	@echo "==================================="
	@echo "  Compilación completada!"
	@echo "==================================="
	@echo "Biblioteca: $(LIBRARY)"
	@echo "Ejemplos: $(EJEMPLO_HIJO) y $(EJEMPLO_PADRE_COMPLETO)"
	@echo ""
	@echo "Para ejecutar el programa principal:"
	@echo "  cd examples && ./proceso_padre_completo"
	@echo ""
	@echo ""

# Crear directorio lib si no existe
$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Compilar archivos objeto de la biblioteca
$(LIB_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/ProcesoPar.h | $(LIB_DIR)
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Crear biblioteca estática
$(LIBRARY): $(LIB_OBJECTS)
	@echo "Creando biblioteca estática..."
	ar rcs $@ $^
	@echo "Biblioteca creada: $@"

# Compilar proceso hijo
$(EJEMPLO_HIJO): $(EXAMPLES_DIR)/proceso_hijo.c
	@echo "Compilando proceso hijo..."
	$(CC) $(CFLAGS) $< -o $@

# Compilar proceso padre completo (enlazando con la biblioteca)
$(EJEMPLO_PADRE_COMPLETO): $(EXAMPLES_DIR)/proceso_padre_completo.c $(LIBRARY)
	@echo "Compilando proceso padre completo..."
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -lprocesopar

# Limpiar archivos generados
clean:
	@echo "Limpiando archivos generados..."
	rm -f $(LIB_OBJECTS) $(LIBRARY)
	rm -f $(EJEMPLO_HIJO) $(EJEMPLO_PADRE_COMPLETO)
	@echo "Limpieza completada."

# Ejecutar el nuevo ejemplo principal
run: all
	@echo ""
	@echo "==================================="
	@echo "  Ejecutando programa principal..."
	@echo "==================================="
	@echo ""
	cd $(EXAMPLES_DIR) && ./proceso_padre_completo

# Target para recompilar todo
rebuild: clean all

# Mostrar ayuda
help:
	@echo "Makefile para ProcesoPar - Linux"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  all      - Compilar biblioteca y ejemplos (default)"
	@echo "  clean    - Eliminar archivos generados"
	@echo "  rebuild  - Limpiar y recompilar todo"
	@echo "  run      - Compilar y ejecutar el ejemplo"
	@echo "  help     - Mostrar esta ayuda"
	@echo ""
	@echo "Ejemplos de uso:"
	@echo "  make           # Compilar todo"
	@echo "  make clean     # Limpiar"
	@echo "  make run       # Compilar y ejecutar"
	@echo ""

.PHONY: all clean rebuild run help
