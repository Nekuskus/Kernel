# The Firework Operating System [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/github/Firework-OS/Kernel.svg?logo=lgtm&logoWidth=18&style=for-the-badge)](https://lgtm.com/projects/g/Firework-OS/Kernel/context:cpp)

![Automagically bundle Release ISO](https://img.shields.io/github/workflow/status/Firework-OS/Kernel/Automagically%20bundle%20Release%20ISO?logo=github&label=Automagically%20bundle%20Release%20ISO&logoWidth=18&style=for-the-badge) ![Automagically bundle Canary ISO](https://img.shields.io/github/workflow/status/Firework-OS/Kernel/Automagically%20bundle%20Canary%20ISO?logo=github&label=Automagically%20bundle%20Canary%20ISO&logoWidth=18&style=for-the-badge)

# 

## What is this about?

The main repository of Firework, a Monolithic operating system.

### Is this a Linux distro?

No.

### Is this Unix based?

No!

### What makes Firework special?

Right now? Nothing
But, This operating system will not use crusty stuff like POSIX, everything is going to be written from scratch, it will also be more secure with an application permission system and other security and quality features.

## Trying out Firework

We provide canary and release builds in this repository's release page.

To run this OS in Qemu you can run `qemu-system-x86_64 -drive format=raw,file=path/to/Firework.img -no-reboot -no-shutdown -m 512M -M q35 -serial stdio` 

## Supported Hardware

### Graphics

* Generic VBE

## Building Firework

While this repository contains Firework's kernel, it isn't enough to build a full Firework distribution.

Instead, look at the [Bootstrap](https://github.com/Firework-OS/Bootstrap) repository.

## Known bugs

Kernel is slow on real hardware; see issue [#13](https://github.com/Firework-OS/Kernel/issues/13)
