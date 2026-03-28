# ZyphraOS Documentation

Welcome to the ZyphraOS documentation. This documentation is available in multiple languages.

## Available Languages

- [English](en/README.md)
- [Español](es/README.md)

## Quick Start

### Building ZyphraOS

```bash
make
```

### Running in QEMU

```bash
make run
```

### Running in VirtualBox

1. Create a new virtual machine
2. Mount the `zyphraos.iso` image
3. Start the virtual machine

## Project Structure

```
zyphraos/
├── docs/              # Documentation (multilingual)
│   ├── en/           # English documentation
│   └── es/           # Spanish documentation
├── src/              # Source code
│   ├── boot.S        # Bootloader stage 1
│   ├── boot2.S       # Bootloader stage 2
│   ├── kernel.c      # Main kernel file
│   ├── shell.c       # Shell implementation
│   ├── fat16.c       # FAT16 file system
│   └── ...           # Other source files
├── iso/              # ISO image files
├── Makefile          # Build configuration
└── README.md         # Main README file
```

## Features

- Custom bootloader
- Protected mode kernel
- Memory management (PMM, Paging, Heap)
- FAT16 file system
- Interactive shell
- VGA graphics driver
- PS/2 keyboard driver
- ATA disk driver
- System monitor
- Snake game

## License

ZyphraOS is licensed under the MIT License. See the [LICENSE](../LICENSE) file for details.

---

*Documentation updated: March 2026*
