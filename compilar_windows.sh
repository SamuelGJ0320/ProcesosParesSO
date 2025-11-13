#!/bin/bash

# Script para compilar el proyecto para Windows desde WSL usando MinGW

echo "==============================================="
echo "  Compilando ProcesoPar para Windows (MinGW)"
echo "==============================================="
echo ""

# Verificar si MinGW está instalado
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "[ERROR] MinGW-w64 no está instalado"
    echo ""
    echo "Instalando MinGW-w64..."
    sudo apt update
    sudo apt install mingw-w64 -y
    echo ""
fi

# Limpiar compilaciones anteriores de Windows
echo "[1/6] Limpiando archivos anteriores..."
rm -f lib/*_win.o lib/libprocesopar_win.a
rm -f examples/*.exe

# Crear directorio lib si no existe
mkdir -p lib

echo "[2/6] Compilando archivos fuente..."
x86_64-w64-mingw32-gcc -Wall -Wextra -I./include -c src/lanzarProcesoPar.c -o lib/lanzarProcesoPar_win.o
if [ $? -ne 0 ]; then echo "Error compilando lanzarProcesoPar.c"; exit 1; fi

x86_64-w64-mingw32-gcc -Wall -Wextra -I./include -c src/destruirProcesoPar.c -o lib/destruirProcesoPar_win.o
if [ $? -ne 0 ]; then echo "Error compilando destruirProcesoPar.c"; exit 1; fi

x86_64-w64-mingw32-gcc -Wall -Wextra -I./include -c src/enviarMensajeProcesoPar.c -o lib/enviarMensajeProcesoPar_win.o
if [ $? -ne 0 ]; then echo "Error compilando enviarMensajeProcesoPar.c"; exit 1; fi

x86_64-w64-mingw32-gcc -Wall -Wextra -I./include -c src/establecerFuncionDeEscucha.c -o lib/establecerFuncionDeEscucha_win.o
if [ $? -ne 0 ]; then echo "Error compilando establecerFuncionDeEscucha.c"; exit 1; fi

echo "[3/6] Creando biblioteca estática..."
x86_64-w64-mingw32-ar rcs lib/libprocesopar_win.a lib/lanzarProcesoPar_win.o lib/destruirProcesoPar_win.o lib/enviarMensajeProcesoPar_win.o lib/establecerFuncionDeEscucha_win.o
if [ $? -ne 0 ]; then echo "Error creando biblioteca"; exit 1; fi

echo "[4/6] Compilando proceso_hijo.exe..."
x86_64-w64-mingw32-gcc -Wall -Wextra -I./include examples/proceso_hijo.c -o examples/proceso_hijo.exe
if [ $? -ne 0 ]; then echo "Error compilando proceso_hijo"; exit 1; fi

echo "[5/6] Compilando proceso_padre.exe..."
x86_64-w64-mingw32-gcc -Wall -Wextra -I./include examples/proceso_padre.c -o examples/proceso_padre.exe -L./lib -lprocesopar_win
if [ $? -ne 0 ]; then echo "Error compilando proceso_padre"; exit 1; fi

echo "[6/6] Verificando archivos generados..."
if [ -f "examples/proceso_padre.exe" ] && [ -f "examples/proceso_hijo.exe" ]; then
    echo ""
    echo "==============================================="
    echo "  ¡Compilación exitosa!"
    echo "==============================================="
    echo ""
    echo "Archivos generados:"
    echo "  - lib/libprocesopar_win.a"
    echo "  - examples/proceso_hijo.exe"
    echo "  - examples/proceso_padre.exe"
    echo ""
    echo "Para ejecutar en Windows PowerShell:"
    echo "  cd examples"
    echo "  .\\proceso_padre.exe"
    echo ""
    echo "Para ejecutar desde WSL (usando Wine):"
    echo "  cd examples"
    echo "  wine proceso_padre.exe"
    echo ""
else
    echo ""
    echo "==============================================="
    echo "  ERROR: Fallo en la compilación"
    echo "==============================================="
    exit 1
fi
