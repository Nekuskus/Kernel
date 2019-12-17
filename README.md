# Spark Kernel
The kernel for the Spark Operating System.

## Features
- VBE Framebuffer graphics with lean-and-mean text drawing
- Bitmap physical memory management
- 4-level paging
- Interrupts
- Panic
- Terminal
- Symmetric Multiprocessing (SMP)

## Planned features
- ACPI (currently being worked on)
- (U)EFI
- VFS
- R/W from disk
- FS
- Drivers
- API
- Userspace
- Multitasking


(this list is not sorted by priority)

## Compiling the kernel
### Compiling the kernel - the dependencies
- CLang
- CMake
- Nasm

Building is easy, just use CMake like you would with any other project.