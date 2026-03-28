# Guía de Contribución para ZyphraOS

## Introducción

Esta guía establece los estándares y procedimientos para contribuir al desarrollo de ZyphraOS. Todas las contribuciones deben seguir estas directrices para mantener la calidad y consistencia del proyecto.

## Tabla de Contenidos

1. [Tipos de Contribución](#tipos-de-contribución)
2. [Requisitos del Entorno de Desarrollo](#requisitos-del-entorno-de-desarrollo)
3. [Proceso de Contribución](#proceso-de-contribución)
4. [Estándares de Código](#estándares-de-código)
5. [Reporte de Errores](#reporte-de-errores)
6. [Solicitud de Características](#solicitud-de-características)
7. [Soporte Técnico](#soporte-técnico)

## Tipos de Contribución

### Contribuciones de Código

- Corrección de errores y bugs
- Implementación de nuevas funcionalidades
- Optimización de rendimiento
- Refactorización de código existente

### Contribuciones de Documentación

- Actualización de documentación técnica
- Creación de guías y tutoriales
- Traducción de documentación
- Mejora de ejemplos de código

### Contribuciones de Pruebas

- Desarrollo de casos de prueba
- Ejecución de pruebas de regresión
- Validación de funcionalidades

## Requisitos del Entorno de Desarrollo

### Sistema Operativo

- Linux (Ubuntu 20.04 o superior recomendado)
- macOS 10.15 o superior
- Windows 10/11 con WSL2

### Herramientas Requeridas

| Herramienta | Versión Mínima | Propósito |
|-------------|----------------|-----------|
| GCC | 9.0 | Compilación del kernel |
| NASM | 2.14 | Ensamblador |
| Make | 4.0 | Sistema de construcción |
| Git | 2.25 | Control de versiones |
| QEMU | 5.0 | Emulación y pruebas |

### Configuración del Cross-Compiler

```bash
# Instalar dependencias (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential nasm make git qemu-system-x86

# Compilar cross-compiler i686-elf
wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.xz

# Compilar binutils
tar -xf binutils-2.40.tar.xz
cd binutils-2.40
./configure --target=i686-elf --prefix=/usr/local/cross --disable-nls
make
sudo make install

# Compilar GCC
tar -xf gcc-12.2.0.tar.xz
cd gcc-12.2.0
./configure --target=i686-elf --prefix=/usr/local/cross --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
sudo make install-gcc
sudo make install-target-libgcc
```

## Proceso de Contribución

### Paso 1: Preparación del Entorno

```bash
# Clonar el repositorio
git clone https://github.com/zyphraos/zyphraos.git
cd zyphraos

# Crear rama de desarrollo
git checkout -b feature/nombre-caracteristica
```

### Paso 2: Desarrollo

1. Implementar los cambios siguiendo los estándares de código
2. Realizar pruebas unitarias y de integración
3. Verificar que el código compila sin errores
4. Ejecutar el sistema en QEMU para validación

### Paso 3: Validación

```bash
# Limpiar compilaciones anteriores
make clean

# Compilar el proyecto
make

# Ejecutar en QEMU
make run
```

### Paso 4: Documentación

- Actualizar documentación relevante
- Incluir comentarios en el código cuando sea necesario
- Crear o actualizar archivos de prueba

### Paso 5: Envío

```bash
# Agregar cambios
git add .

# Crear commit con mensaje descriptivo
git commit -m "feat: descripción de la característica implementada"

# Subir cambios
git push origin feature/nombre-caracteristica
```

### Paso 6: Pull Request

1. Crear Pull Request en GitHub
2. Completar la plantilla de PR
3. Esperar revisión del equipo
4. Realizar correcciones solicitadas

## Estándares de Código

### Convenciones de Nomenclatura

| Elemento | Convención | Ejemplo |
|----------|------------|---------|
| Funciones | snake_case | `memory_init()` |
| Variables | snake_case | `current_process` |
| Constantes | UPPER_SNAKE_CASE | `MAX_PROCESSES` |
| Estructuras | PascalCase | `ProcessControlBlock` |
| Archivos | snake_case | `memory_manager.c` |

### Formato de Código

- Indentación: 4 espacios
- Línea máxima: 80 caracteres
- Sin espacios en blanco al final de línea
- Línea en blanco al final del archivo

### Estructura de Archivos

```c
/**
 * @file nombre_archivo.c
 * @brief Descripción breve del archivo
 * @author Nombre del autor
 * @date Fecha
 */

#include "header.h"

// Definiciones de constantes
#define MAX_VALUE 100

// Variables globales estáticas
static int global_counter = 0;

// Implementación de funciones
int function_name(int param) {
    // Implementación
    return 0;
}
```

## Reporte de Errores

### Información Requerida

Para reportar un error, incluya la siguiente información:

1. **Título**: Descripción concisa del problema
2. **Descripción**: Explicación detallada del error
3. **Pasos de Reproducción**: Secuencia exacta para reproducir el problema
4. **Comportamiento Esperado**: Resultado esperado según la especificación
5. **Comportamiento Actual**: Resultado observado
6. **Entorno**:
   - Sistema operativo y versión
   - Versión del compilador
   - Versión de QEMU/VirtualBox
7. **Evidencia**: Capturas de pantalla, logs, traces

### Ejemplo de Reporte

```
Título: Fallo de memoria al ejecutar comando ls con más de 100 archivos

Descripción:
El sistema genera un fallo de página al intentar listar directorios
con más de 100 archivos debido a desbordamiento del buffer de nombres.

Pasos de Reproducción:
1. Crear directorio con 150 archivos
2. Ejecutar comando 'ls' en el directorio
3. Observar el comportamiento del sistema

Comportamiento Esperado:
Listar todos los archivos sin errores de memoria.

Comportamiento Actual:
El sistema genera una excepción de página y se detiene.

Entorno:
- OS: Ubuntu 22.04 LTS
- GCC: 11.3.0
- QEMU: 7.0.0
```

## Solicitud de Características

### Criterios de Aceptación

Las solicitudes de características deben cumplir los siguientes criterios:

1. **Relevancia**: Alineada con los objetivos del proyecto
2. **Factibilidad**: Técnicamente viable dentro de las restricciones del sistema
3. **Documentación**: Incluir especificación técnica detallada
4. **Pruebas**: Definir casos de prueba esperados

### Plantilla de Solicitud

```
Título: [Nombre de la característica]

Descripción:
[Explicación detallada de la funcionalidad]

Justificación:
[Por qué esta característica es necesaria]

Especificación Técnica:
[Detalles de implementación]

Casos de Uso:
[Ejemplos de uso práctico]

Criterios de Aceptación:
[Lista de requisitos verificables]
```

## Soporte Técnico

### Canales de Comunicación

- **GitHub Issues**: Para errores y solicitudes de características
- **Email**: contact@zyphraos.org
- **Documentación**: Consultar la documentación en `docs/`

### Tiempos de Respuesta

- Errores críticos: 24-48 horas
- Errores menores: 3-5 días hábiles
- Solicitudes de características: 5-10 días hábiles

---

*Documento de referencia para contribuidores de ZyphraOS*
*Última actualización: Marzo 2026*
