# The Firework Kernel [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/github/Firework-OS/Kernel.svg?logo=lgtm&logoWidth=18&style=for-the-badge)](https://lgtm.com/projects/g/Firework-OS/Kernel/context:cpp)
# ![Automagically bundle Release ISO](https://img.shields.io/github/workflow/status/Firework-OS/Kernel/Automagically%20bundle%20Release%20ISO?logo=github&label=Automagically%20bundle%20Release%20ISO&logoWidth=18&style=for-the-badge) ![Automagically bundle Canary ISO](https://img.shields.io/github/workflow/status/Firework-OS/Kernel/Automagically%20bundle%20Canary%20ISO?logo=github&label=Automagically%20bundle%20Canary%20ISO&logoWidth=18&style=for-the-badge)

The kernel for Firework, for the x86_64 (IA-32e) CPU architecture.

## Features
- [x] Bitmap Physical Memory Manager with O(1)-like optimization
- [x] 4-Level Paging
- [x] Heap allocation (malloc & co)
- [x] C++ support (internal use only!)
- [x] Linear framebuffer drawing
- [x] Framebuffer terminal (incomplete; needs scrolling support)
- [x] Lean & Mean font drawing
- [x] Serial debugging
- [x] Exception handlers (incomplete; missing handlers; see issue [#16](https://github.com/Firework-OS/Kernel/issues/16))
- [ ] ACPI (incomplete; missing features)
- [X] PCI (no PCI-Express *yet*)
- [x] HPET (required; used by ksleep)
- [x] Symmetric multiprocessing
- [x] MSR reading & writing
- [x] Local APIC
- [x] I/O APIC
- [x] Task scheduler, uses the APIC Timer, completely unfair with O(1)-like optimization (incomplete; works)
- [x] Progress bar while it's starting (fancy!)
- [ ] SATA AHCI (incomplete; doesn't work; see issue [#7](https://github.com/Firework-OS/Kernel/issues/7))
- [ ] Networking (hasn't been worked on; see issue [#12](https://github.com/Firework-OS/Kernel/issues/12))
- [x] GRUB Multiboot 1

## Goals
- [ ] Create a simple and efficient kernel with a good API that learns from the other kernel's mistakes
- [ ] Make the kernel asynchronous

# Known bugs
Kernel is slow on real hardware; see issue [#13](https://github.com/Firework-OS/Kernel/issues/13)
