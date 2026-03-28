# ZyphraOS Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [System Requirements](#system-requirements)
3. [Building](#building)
4. [Installation](#installation)
5. [Usage](#usage)
6. [Architecture](#architecture)
7. [Contributing](#contributing)
8. [License](#license)

## Introduction

ZyphraOS is an educational operating system built from scratch, designed to teach the fundamentals of systems programming and kernel development. The project includes a custom bootloader, a basic kernel with memory management, interrupt handling, FAT16 file system, and an interactive shell.

> **⚠️ IMPORTANT NOTE:**
>
> **Execution Goal:** ZyphraOS is designed to run on both emulators (QEMU, VirtualBox) and real hardware. The goal is to create a functional operating system that can boot on physical computers.
>
> **Critical VirtualBox Error:** A critical error has been detected when running ZyphraOS on VirtualBox. The `VBox.log` file contains the failure details. **Community help is sought to resolve this issue.**
>
> **Reward:** Anyone who helps resolve this critical error will receive:
> - Special credits in the project
> - Distinguished collaborator role
> - Recognition in the documentation
>
> If you have experience with VirtualBox, kernel development, or operating system debugging, your help is invaluable.

## System Requirements

- **Operating System**: Linux, macOS, or Windows with WSL
- **Compiler**: GCC with i686-elf support
- **Tools**:
  - NASM (Netwide Assembler)
  - Make
  - QEMU or VirtualBox for testing
  - GRUB (for ISO boot)

## Building

To build ZyphraOS, run the following command in the project root directory:

```bash
make
```

This will generate the `zyphraos.iso` file that can be used to boot the system.

### Build Options

- `make clean`: Cleans compiled files
- `make iso`: Generates only the ISO image
- `make run`: Builds and runs in QEMU

## Installation

### On QEMU

```bash
make run
```

### On VirtualBox

1. Create a new virtual machine
2. Select "Other" as type and "Other/Unknown" as version
3. Allocate at least 512 MB of RAM
4. Create a new virtual disk (VDI, dynamically allocated)
5. Mount the `zyphraos.iso` image in the CD drive
6. Start the virtual machine

## Usage

### ZyphraOS Shell

The system includes an interactive shell with the following commands:

| Command | Description |
|---------|-------------|
| `help` | Displays the list of available commands |
| `clear` | Clears the screen |
| `ls` | Lists files in the current directory |
| `cat <file>` | Displays the contents of a file |
| `touch <file>` | Creates a new file |
| `rm <file>` | Deletes a file |
| `mkdir <directory>` | Creates a directory |
| `rmdir <directory>` | Deletes a directory |
| `cp <source> <destination>` | Copies a file |
| `mv <source> <destination>` | Moves or renames a file |
| `sysinfo` | Displays system information |
| `meminfo` | Displays memory information |
| `cpuinfo` | Displays CPU information |
| `snake` | Runs the Snake game |
| `sysmon` | System monitor |
| `exit` | Shuts down the system |

## Architecture

### Main Components

1. **Bootloader** (`boot.S`, `boot2.S`)
   - Loads the kernel from disk
   - Sets up protected mode
   - Transfers control to the kernel

2. **Kernel** (`kernel.c`)
   - Initializes kernel subsystems
   - Sets up the IDT (Interrupt Descriptor Table)
   - Initializes memory management

3. **Memory Management**
   - **PMM** (`pmm.c`): Physical Memory Manager
   - **Paging** (`paging.c`): Virtual memory paging
   - **Heap** (`heap.c`): Kernel heap management

4. **File System** (`fat16.c`)
   - FAT16 implementation
   - Read/write operations
   - Directory and file management

5. **Drivers**
   - **VGA** (`vbe.c`): Video driver
   - **Keyboard** (`keyboard.c`): PS/2 keyboard driver
   - **ATA** (`ata.c`): ATA disk driver
   - **PIT** (`pit.c`): Programmable Interval Timer

6. **Shell** (`shell.c`)
   - Command-line interface
   - Command parsing
   - Program execution

### Memory Map

```
0x00000000 - 0x000FFFFF: Low memory (1 MB)
0x00100000 - 0x00FFFFFF: Kernel (15 MB)
0x01000000 - 0x01FFFFFF: Kernel heap (16 MB)
0x02000000 - 0x0FFFFFFF: User memory
```

## Contributing

See the [CONTRIBUTING.md](../../CONTRIBUTING.md) file for information on how to contribute to the project.

## License

ZyphraOS is licensed under the MIT License. See the [LICENSE](../../LICENSE) file for more details.

## Contact

- **GitHub**: [ZyphraOS Repository](https://github.com/zyphraos/zyphraos)
- **Email**: contact@zyphraos.org

---

*Documentation updated: March 2026*
