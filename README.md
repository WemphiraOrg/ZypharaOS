# ZyphraOS - Sistema Operativo Educativo

## Descripción

ZyphraOS es un sistema operativo x86 de 32 bits desarrollado desde cero, sin usar Linux, libc o cualquier otra dependencia. Es un proyecto educativo diseñado para enseñar cómo funcionan los sistemas operativos a nivel bajo nivel.

## Características

- ✅ Boot desde GRUB con soporte Multiboot
- ✅ Modo protegido 32-bit
- ✅ Gestión de memoria física (PMM)
- ✅ Paginación
- ✅ Heap del kernel
- ✅ Scheduler de procesos
- ✅ Driver de video VBE
- ✅ Driver de teclado PS/2
- ✅ Driver de disco ATA
- ✅ Sistema de archivos FAT16
- ✅ Shell interactivo
- ✅ Juego Snake
- ✅ Monitor del sistema

## Licencia

Este proyecto está licenciado bajo la **ZyphraOS License v1.0**.

### Resumen de la Licencia

Puedes:
- ✅ **Usar** el código libremente para cualquier propósito
- ✅ **Modificar** el código para tus proyectos
- ✅ **Redistribuir** el código a otros
- ✅ **Aprender** de él y contribuir al proyecto

Pero ten en cuenta:
- ⚠️ El código es original de ZyphraOS
- ⚠️ No me hago responsable si dañas tu hardware
- ⚠️ Es software experimental, úsalo bajo tu propio riesgo

**Ver archivo [LICENSE](LICENSE) para los términos completos.**

## Advertencia Importante

**⚠️ ESTE SOFTWARE ES EXPERIMENTAL Y SE PROPORCIONA "TAL CUAL".**

El uso en hardware real conlleva riesgos inherentes, incluyendo pero no limitado a:
- Daños potenciales al hardware
- Pérdida de datos en discos duros
- Inestabilidad del sistema
- Comportamiento impredecible

**EL USUARIO ES EL ÚNICO RESPONSABLE DE CUALQUIER DAÑO QUE PUEDA OCURRIR.**

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
# Modo VGA texto
make run

# Modo VBE 800x600
make run-vbe

# Modo VBE con más RAM
make run-big
```

### 4. Probar en Hardware Real

1. Grabar `zyphraos.iso` en USB con Rufus
2. Boot desde USB
3. Seleccionar modo VBE en menú GRUB
4. ¡Disfrutar!

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
├── Makefile            # Script de compilación
├── LICENSE             # Licencia del proyecto
└── README.md           # Este archivo
```

## Documentación

- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Guía completa para contribuir al proyecto
- **[CREDITS.md](CREDITS.md)** - Reconocimiento a colaboradores y roles especiales
- **[ANALISIS_VIDEO.md](ANALISIS_VIDEO.md)** - Análisis detallado del driver de video
- **[CORRECCIONES_VIDEO.md](CORRECCIONES_VIDEO.md)** - Resumen de correcciones realizadas
- **[INSTRUCCIONES_PRUEBA.md](INSTRUCCIONES_PRUEBA.md)** - Guía para probar el sistema

## Contribuciones

Las contribuciones son bienvenidas. Si quieres contribuir:

### 🚀 Únete a la Comunidad

**[Únete a nuestro servidor de Discord](https://discord.gg/KVFBpEN3Gs)** - ¡Aquí puedes:
- Hacer preguntas sobre el código
- Proponer nuevas funcionalidades
- Colaborar con otros desarrolladores
- Compartir tus experimentos
- Obtener ayuda con problemas técnicos

### 📝 Proceso de Contribución

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

## Contacto y Colaboración

- **Canal de YouTube:** [Tu Canal]
- **Repositorio:** [Tu Repositorio]
- **Email:** [Tu Email]
- **Discord:** [Únete a nuestro servidor](https://discord.gg/KVFBpEN3Gs) - ¡Colabora con el proyecto, haz preguntas y comparte tus ideas!

## Agradecimientos

Gracias a toda la comunidad de desarrollo de sistemas operativos por su inspiración y apoyo.

---

**Recuerda:** Este es un proyecto educativo. Úsalo para aprender y experimentar. ¡Diviértete creando tu propio sistema operativo!

---

## Ver Créditos en el Sistema Operativo

ZyphraOS reconoce a sus colaboradores dentro del propio sistema:

### Comando `credits`
En el shell de ZyphraOS, escribe `credits` para ver todos los colaboradores:

```
zyphraos> credits

╔══════════════════════════════════════════════════════════════╗
║                    ZYPHRAOS CREDITS                         ║
╠══════════════════════════════════════════════════════════════╣
║  Project Creator:                                          ║
║    - ZyphraOS Team                                         ║
║                                                            ║
║  Core Contributors:                                        ║
║    - (Tu nombre podría estar aquí!)                        ║
║                                                            ║
║  Contributors:                                             ║
║    - (Tu nombre podría estar aquí!)                        ║
║                                                            ║
║  Únete: https://discord.gg/KVFBpEN3Gs                      ║
╚══════════════════════════════════════════════════════════════╝
```

### Comando `sysmon`
El monitor del sistema muestra información del sistema incluyendo el número de colaboradores.

### Pantalla de Boot
Los colaboradores principales se muestran durante el arranque del sistema.

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

### ¿Puedo usarlo comercialmente?
Sí, siempre que respetes los términos de atribución.

### ¿Cómo puedo contribuir?
Lee [CONTRIBUTING.md](CONTRIBUTING.md) para la guía completa. ¡Las contribuciones son bienvenidas!

### ¿Cómo obtengo reconocimiento por mi contribución?
Lee [CREDITS.md](CREDITS.md) para ver cómo los colaboradores obtienen reconocimiento en el sistema operativo.

---

**Última actualización:** 26 de febrero de 2026
**Versión:** 1.0
**Licencia:** ZyphraOS License v1.0
