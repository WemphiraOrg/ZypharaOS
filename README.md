# ZyphraOS - Sistema Operativo Educativo

## Descripción

ZyphraOS es un sistema operativo x86 de 32 bits desarrollado desde cero, sin dependencias externas. Proyecto educativo para enseñar fundamentos de sistemas operativos a nivel bajo.

## Características

- Boot desde GRUB con soporte Multiboot
- Modo protegido 32-bit
- Gestión de memoria física (PMM)
- Paginación
- Heap del kernel
- Scheduler de procesos
- Driver de video VBE
- Driver de teclado PS/2
- Driver de disco ATA
- Sistema de archivos FAT16
- Shell interactivo
- Juego Snake
- Monitor del sistema

## Capturas de pantalla

### Shell interactivo
![Shell de ZyphraOS](screenshots/ZypharaTest1.png)

## Requisitos

### Para Compilar
- GCC con soporte para 32-bit
- LD (GNU Linker)
- GRUB (para crear ISO)
- QEMU (para pruebas)

### Para Ejecutar
- Computadora x86 compatible
- Mínimo 64MB de RAM
- Tarjeta de video compatible VBE

## Instalación

### 1. Compilar el Kernel

```bash
cd zyphraos
make clean
make
```

### 2. Crear ISO

```bash
make iso
```

### 3. Probar en QEMU

```bash
# Modo VGA texto (64MB RAM)
make run

# Modo VBE 800x600 (64MB RAM)
make run-vbe

# Modo VBE con más RAM (256MB)
make run-big
```

### 4. Probar en Hardware Real

1. Grabar `zyphraos.iso` en USB con Rufus
2. Boot desde USB
3. Seleccionar modo VBE en menú GRUB

## Estructura del Proyecto

```
zyphraos/
├── src/
│   ├── boot.S          # Punto de entrada del kernel
│   ├── kernel.c        # Función main del kernel
│   ├── vbe.c           # Driver de video VBE
│   ├── vbe.h           # Header del driver VBE
│   ├── multiboot.h     # Estructura Multiboot
│   ├── paging.c        # Gestión de paginación
│   ├── pmm.c           # Gestor de memoria física
│   ├── heap.c          # Heap del kernel
│   ├── process.c       # Scheduler de procesos
│   ├── keyboard.c      # Driver de teclado
│   ├── ata.c           # Driver de disco ATA
│   ├── fat16.c         # Sistema de archivos FAT16
│   ├── shell.c         # Shell interactivo
│   ├── snake.c         # Juego Snake
│   ├── sysmon.c        # Monitor del sistema
│   └── ...
├── iso/
│   └── boot/
│       ├── grub/
│       │   └── grub.cfg    # Configuración de GRUB
│       └── zyphraos.elf    # Kernel compilado
├── screenshots/        # Capturas de pantalla del sistema
├── Makefile            # Script de compilación
├── LICENSE             # Licencia del proyecto
└── README.md           # Este archivo
```

## Documentación

- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Guía para contribuir al proyecto
- **[CREDITS.md](CREDITS.md)** - Reconocimiento a colaboradores

## Licencia

Este proyecto está licenciado bajo la **ZyphraOS License v1.0**.

### Resumen de la Licencia

Puedes:
- Usar el código libremente para cualquier propósito
- Modificar el código para tus proyectos
- Redistribuir el código a otros
- Aprender de él y contribuir al proyecto

Restricciones:
- El código es original de ZyphraOS
- No se hace responsable de daños al hardware
- Es software experimental, úsalo bajo tu propio riesgo

**Ver archivo [LICENSE](LICENSE) para los términos completos.**

## Advertencia

**ESTE SOFTWARE ES EXPERIMENTAL Y SE PROPORCIONA "TAL CUAL".**

El uso en hardware real conlleva riesgos:
- Daños potenciales al hardware
- Pérdida de datos en discos duros
- Inestabilidad del sistema
- Comportamiento impredecible

**EL USUARIO ES EL ÚNICO RESPONSABLE DE CUALQUIER DAÑO QUE PUEDA OCURRIR.**

## Contribuciones

Las contribuciones son bienvenidas. Para contribuir:

1. Haz un fork del proyecto
2. Crea una rama para tu feature (`git checkout -b feature/nueva-funcionalidad`)
3. Haz commit de tus cambios (`git commit -am 'Agregar nueva funcionalidad'`)
4. Push a la rama (`git push origin feature/nueva-funcionalidad`)
5. Abre un Pull Request

### Guía de Contribución

- Mantén el código limpio y bien documentado
- Sigue el estilo de código existente
- Agrega comentarios explicativos
- Prueba tus cambios antes de enviar
- Respeta la licencia del proyecto

## Contacto

- **Canal de YouTube:** [BrunoEnBits](https://www.youtube.com/@BrunoEnBits)
- **Email:** juanlopezdfu@gmail.com
- **Discord:** [Únete a nuestro servidor](https://discord.gg/KVFBpEN3Gs)

## Preguntas Frecuentes

### ¿Puedo usar este sistema operativo como mi sistema principal?
No recomendado. Es software experimental diseñado para fines educativos.

### ¿Funciona en hardware real?
Sí, pero con precaución. Usa un USB booteable y no lo instales en tu disco principal.

### ¿Puedo modificar el código?
Sí, esa es la gracia de ser open source. Modifica, aprende y comparte.

### ¿Necesito permiso para usarlo?
No, la licencia te otorga permiso automáticamente.

### ¿Qué pasa si daño mi hardware?
Ese es tu responsabilidad. La licencia es clara sobre eso.

### ¿Cómo puedo contribuir?
Lee [CONTRIBUTING.md](CONTRIBUTING.md) para la guía completa.

### ¿Cómo obtengo reconocimiento por mi contribución?
Lee [CREDITS.md](CREDITS.md) para ver cómo los colaboradores obtienen reconocimiento.

---

**Última actualización:** 28 de marzo de 2026
**Versión:** 1.0
**Licencia:** ZyphraOS License v1.0
