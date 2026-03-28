# Contributing to ZyphraOS

## Introduction

This guide establishes the standards and procedures for contributing to ZyphraOS development. All contributions must follow these guidelines to maintain project quality and consistency.

## Table of Contents

1. [Contribution Types](#contribution-types)
2. [Development Environment Requirements](#development-environment-requirements)
3. [Contribution Process](#contribution-process)
4. [Code Standards](#code-standards)
5. [Bug Reporting](#bug-reporting)
6. [Feature Requests](#feature-requests)
7. [Technical Support](#technical-support)

## Contribution Types

### Code Contributions

- Bug fixes and corrections
- New feature implementation
- Performance optimization
- Code refactoring

### Documentation Contributions

- Technical documentation updates
- Guide and tutorial creation
- Documentation translation
- Code example improvements

### Testing Contributions

- Test case development
- Regression testing execution
- Feature validation

## Development Environment Requirements

### Operating System

- Linux (Ubuntu 20.04 or later recommended)
- macOS 10.15 or later
- Windows 10/11 with WSL2

### Required Tools

| Tool | Minimum Version | Purpose |
|------|-----------------|---------|
| GCC | 9.0 | Kernel compilation |
| NASM | 2.14 | Assembler |
| Make | 4.0 | Build system |
| Git | 2.25 | Version control |
| QEMU | 5.0 | Emulation and testing |

### Cross-Compiler Setup

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential nasm make git qemu-system-x86

# Compile i686-elf cross-compiler
wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.xz

# Compile binutils
tar -xf binutils-2.40.tar.xz
cd binutils-2.40
./configure --target=i686-elf --prefix=/usr/local/cross --disable-nls
make
sudo make install

# Compile GCC
tar -xf gcc-12.2.0.tar.xz
cd gcc-12.2.0
./configure --target=i686-elf --prefix=/usr/local/cross --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
sudo make install-gcc
sudo make install-target-libgcc
```

## Contribution Process

### Step 1: Environment Preparation

```bash
# Clone repository
git clone https://github.com/zyphraos/zyphraos.git
cd zyphraos

# Create development branch
git checkout -b feature/feature-name
```

### Step 2: Development

1. Implement changes following code standards
2. Perform unit and integration testing
3. Verify code compiles without errors
4. Execute system in QEMU for validation

### Step 3: Validation

```bash
# Clean previous builds
make clean

# Compile project
make

# Run in QEMU
make run
```

### Step 4: Documentation

- Update relevant documentation
- Include code comments when necessary
- Create or update test files

### Step 5: Submission

```bash
# Add changes
git add .

# Create commit with descriptive message
git commit -m "feat: description of implemented feature"

# Push changes
git push origin feature/feature-name
```

### Step 6: Pull Request

1. Create Pull Request on GitHub
2. Complete PR template
3. Wait for team review
4. Make requested corrections

## Code Standards

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Functions | snake_case | `memory_init()` |
| Variables | snake_case | `current_process` |
| Constants | UPPER_SNAKE_CASE | `MAX_PROCESSES` |
| Structures | PascalCase | `ProcessControlBlock` |
| Files | snake_case | `memory_manager.c` |

### Code Formatting

- Indentation: 4 spaces
- Maximum line length: 80 characters
- No trailing whitespace
- Blank line at end of file

### File Structure

```c
/**
 * @file filename.c
 * @brief Brief file description
 * @author Author name
 * @date Date
 */

#include "header.h"

// Constant definitions
#define MAX_VALUE 100

// Static global variables
static int global_counter = 0;

// Function implementations
int function_name(int param) {
    // Implementation
    return 0;
}
```

## Bug Reporting

### Required Information

To report a bug, include the following information:

1. **Title**: Concise problem description
2. **Description**: Detailed error explanation
3. **Reproduction Steps**: Exact sequence to reproduce the problem
4. **Expected Behavior**: Expected result according to specification
5. **Current Behavior**: Observed result
6. **Environment**:
   - Operating system and version
   - Compiler version
   - QEMU/VirtualBox version
7. **Evidence**: Screenshots, logs, traces

### Report Example

```
Title: Memory failure when executing ls command with more than 100 files

Description:
The system generates a page fault when attempting to list directories
with more than 100 files due to name buffer overflow.

Reproduction Steps:
1. Create directory with 150 files
2. Execute 'ls' command in directory
3. Observe system behavior

Expected Behavior:
List all files without memory errors.

Current Behavior:
System generates page exception and stops.

Environment:
- OS: Ubuntu 22.04 LTS
- GCC: 11.3.0
- QEMU: 7.0.0
```

## Feature Requests

### Acceptance Criteria

Feature requests must meet the following criteria:

1. **Relevance**: Aligned with project objectives
2. **Feasibility**: Technically viable within system constraints
3. **Documentation**: Include detailed technical specification
4. **Testing**: Define expected test cases

### Request Template

```
Title: [Feature name]

Description:
[Detailed functionality explanation]

Justification:
[Why this feature is necessary]

Technical Specification:
[Implementation details]

Use Cases:
[Practical usage examples]

Acceptance Criteria:
[Verifiable requirements list]
```

## Technical Support

### Communication Channels

- **GitHub Issues**: For bugs and feature requests
- **Email**: contact@zyphraos.org
- **Documentation**: See documentation in `docs/`

### Response Times

- Critical bugs: 24-48 hours
- Minor bugs: 3-5 business days
- Feature requests: 5-10 business days

---

*Reference document for ZyphraOS contributors*
*Last updated: March 2026*
