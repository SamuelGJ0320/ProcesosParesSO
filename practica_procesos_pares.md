# Práctica 01 — Procesos Pares

**Fecha:** 14 de Octubre de 2025  
**Autores:** Juan Francisco Cardona Mc’Cormick et al.

---

## 1. Introducción

En algunas ocasiones, es necesario implementar mecanismos para monitorear procesos e hilos, como es el caso de la observabilidad.  
En esta práctica implementaremos un mecanismo simple que emula un entorno más complejo: **el proceso par**, que permite que un proceso principal controle y se comunique con otro proceso auxiliar.

---

## 2. Proceso Par

Un **proceso par** representa la ejecución y monitorización de procesos externos.  
Este se implementa usando una biblioteca que permite:

- Crear un proceso hijo (B) desde un proceso principal (A).
- Comunicarse en modo **full-duplex** a través de dos tuberías:
  - **Tubería de salida:** A → B  
  - **Tubería de entrada:** B → A
- Procesar mensajes entrantes mediante una función de escucha `f()`.

---

## 3. API

### 3.1. Lanzar un proceso par

```c
Estado_t lanzarProcesoPar(
    const char *nombreArchivoEjecutable,
    const char **listaLineaComando,
    ProcesoPar_t **procesoPar
);
```

---

### 3.2. Destruir un proceso par

```c
Estado_t destruirProcesoPar(ProcesoPar_t *procesoPar);
```

---

### 3.3. Enviar un mensaje al proceso par

```c
Estado_t enviarMensajeProcesoPar(
    ProcesoPar_t *procesoPar,
    const char *mensaje,
    int longitud
);
```

---

### 3.4. Establecer una función de escucha

```c
Estado_t establecerFuncionDeEscucha(
    ProcesoPar_t *procesoPar,
    Estado_t (*f)(const char *, int)
);
```

La función debe cumplir:

```c
Estado_t funcion(const char* mensaje, int longitud);
```

---

### 3.5. Tipo `ProcesoPar_t`

```c
typedef struct ProcesoPar {
    /* Definido por cada grupo */
    ...
} ProcesoPar_t;
```

---

### 3.6. Tipo `Estado_t`

```c
typedef unsigned int Estado_t;
```

---

### 3.7. Archivo de encabezados `ProcesoPar.h`

```c
#ifndef PROCESOPAR_H
#define PROCESOPAR_H

typedef unsigned int Estado_t;

typedef struct ProcesoPar {
    /* Definido por cada aplicación */
    ...
} ProcesoPar_t;

#define E_OK 0        /* Todo bien */
#define E_PAR_INC 1   /* Parámetro incorrecto */

/* Estados adicionales definidos por el grupo */
...

#endif
```

---

## 4. Requerimientos

1. Implementar una biblioteca con cada función en un archivo fuente independiente.  
2. Grupos de máximo dos personas.  
   - En grupo → deben implementar Windows + Linux  
   - Individual → solo uno (definido por el profesor)
3. Lenguajes permitidos: **C y C++** usando **GCC** en ambos sistemas.  
4. Usar llamadas al sistema propias de cada SO:  
   - Windows x64  
   - Linux POSIX  
5. Entregar en GitHub o GitLab  
6. Fecha límite: **Sábado 8 de Noviembre, 8 a.m.**  
7. Sustentaciones durante el mismo día (8 a.m. a 5 p.m.)

---

## 5. Estructura del Repositorio

```
+ windows/
    + src/
    + lib/
    + include/
    + readme.txt

+ linux/
    + src/
    + lib/
    + include/
    + readme.txt

+ readme.txt
```

---

## Referencias

- Johnson M. Hart — *Windows System Programming*, Addison-Wesley, 2010.  
- Richter & Nasarre — *Windows via C/C++*, Microsoft Press, 2008.  
- Robbins & Robbins — *Unix Programación Práctica*, Prentice-Hall, 1997.  
- Stevens & Rago — *Advanced Programming in the Unix Environment*, Addison-Wesley, 2013.  
- Kurt Wall — *Programación en Linux con Ejemplos*, Prentice-Hall, 2000.

