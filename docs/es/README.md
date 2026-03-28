# Documentación de ZyphraOS

## Índice

1. [Introducción](#introducción)
2. [Requisitos del Sistema](#requisitos-del-sistema)
3. [Compilación](#compilación)
4. [Instalación](#instalación)
5. [Uso](#uso)
6. [Arquitectura](#arquitectura)
7. [Contribuir](#contribuir)
8. [Licencia](#licencia)

## Introducción

ZyphraOS es un sistema operativo educativo desarrollado desde cero, diseñado para enseñar los fundamentos de la programación de sistemas y el desarrollo de kernels. El proyecto incluye un bootloader personalizado, un kernel básico con gestión de memoria, manejo de interrupciones, sistema de archivos FAT16 y una shell interactiva.

> **⚠️ NOTA IMPORTANTE:**
>
> **Objetivo de Ejecución:** ZyphraOS está diseñado para ejecutarse tanto en emuladores (QEMU, VirtualBox) como en hardware real. El objetivo es crear un sistema operativo funcional que pueda arrancar en computadoras físicas.
>
> **Error Crítico de VirtualBox:** Se ha detectado un error crítico al ejecutar ZyphraOS en VirtualBox. El archivo `VBox.log` contiene los detalles del fallo. **Se busca ayuda de la comunidad para resolver este problema.**
>
> **Recompensa:** Quien contribuya a resolver este error crítico recibirá:
> - Créditos especiales en el proyecto
> - Rol de colaborador destacado
> - Reconocimiento en la documentación
>
> Si tienes experiencia con VirtualBox, desarrollo de kernels o debugging de sistemas operativos, tu ayuda es invaluable.

## Requisitos del Sistema

- **Sistema Operativo**: Linux, macOS o Windows con WSL
- **Compilador**: GCC con soporte para i686-elf
- **Herramientas**:
  - NASM (Netwide Assembler)
  - Make
  - QEMU o VirtualBox para pruebas
  - GRUB (para arranque desde ISO)

## Compilación

Para compilar ZyphraOS, ejecute el siguiente comando en el directorio raíz del proyecto:

```bash
make
```

Esto generará el archivo `zyphraos.iso` que puede ser usado para arrancar el sistema.

### Opciones de Compilación

- `make clean`: Limpia los archivos compilados
- `make iso`: Genera únicamente la imagen ISO
- `make run`: Compila y ejecuta en QEMU

## Instalación

### En QEMU

```bash
make run
```

### En VirtualBox

1. Cree una nueva máquina virtual
2. Seleccione "Other" como tipo y "Other/Unknown" como versión
3. Asigne al menos 512 MB de RAM
4. Cree un disco virtual nuevo (VDI, dinámicamente asignado)
5. Monte la imagen `zyphraos.iso` en la unidad de CD
6. Inicie la máquina virtual

## Uso

### Shell de ZyphraOS

El sistema incluye una shell interactiva con los siguientes comandos:

| Comando | Descripción |
|---------|-------------|
| `help` | Muestra la lista de comandos disponibles |
| `clear` | Limpia la pantalla |
| `ls` | Lista archivos en el directorio actual |
| `cat <archivo>` | Muestra el contenido de un archivo |
| `touch <archivo>` | Crea un nuevo archivo |
| `rm <archivo>` | Elimina un archivo |
| `mkdir <directorio>` | Crea un directorio |
| `rmdir <directorio>` | Elimina un directorio |
| `cp <origen> <destino>` | Copia un archivo |
| `mv <origen> <destino>` | Mueve o renombra un archivo |
| `sysinfo` | Muestra información del sistema |
| `meminfo` | Muestra información de memoria |
| `cpuinfo` | Muestra información del CPU |
| `snake` | Ejecuta el juego Snake |
| `sysmon` | Monitor del sistema |
| `exit` | Apaga el sistema |

## Arquitectura

### Componentes Principales

1. **Bootloader** (`boot.S`, `boot2.S`)
   - Carga el kernel desde el disco
   - Configura el modo protegido
   - Transfiere control al kernel

2. **Kernel** (`kernel.c`)
   - Inicializa subsistemas del kernel
   - Configura la IDT (Interrupt Descriptor Table)
   - Inicializa la gestión de memoria

3. **Gestión de Memoria**
   - **PMM** (`pmm.c`): Physical Memory Manager
   - **Paging** (`paging.c`): Paginación de memoria virtual
   - **Heap** (`heap.c`): Gestión del heap del kernel

4. **Sistema de Archivos** (`fat16.c`)
   - Implementación de FAT16
   - Operaciones de lectura/escritura
   - Gestión de directorios y archivos

5. **Drivers**
   - **VGA** (`vbe.c`): Driver de video
   - **Teclado** (`keyboard.c`): Driver de teclado PS/2
   - **ATA** (`ata.c`): Driver de disco ATA
   - **PIT** (`pit.c`): Programmable Interval Timer

6. **Shell** (`shell.c`)
   - Interfaz de línea de comandos
   - Parsing de comandos
   - Ejecución de programas

### Mapa de Memoria

```
0x00000000 - 0x000FFFFF: Memoria baja (1 MB)
0x00100000 - 0x00FFFFFF: Kernel (15 MB)
0x01000000 - 0x01FFFFFF: Heap del kernel (16 MB)
0x02000000 - 0x0FFFFFFF: Memoria de usuario
```

## Contribuir

Consulte el archivo [CONTRIBUTING.md](../../CONTRIBUTING.md) para obtener información sobre cómo contribuir al proyecto.

## Licencia

ZyphraOS está licenciado bajo la Licencia MIT. Consulte el archivo [LICENSE](../../LICENSE) para más detalles.

## Contacto

- **GitHub**: [ZyphraOS Repository](https://github.com/zyphraos/zyphraos)
- **Email**: contact@zyphraos.org

---

*Documentación actualizada: Marzo 2026*
