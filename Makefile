CXXPARAMS := -target x86_64-unknown-elf -Isrc -ffreestanding -fno-use-cxa-atexit -fno-pic -nostdlib -fno-pie -mno-sse -mno-sse2 -fno-builtin -fno-rtti -fno-exceptions -fsigned-char -fno-stack-protector -mno-red-zone -mcmodel=kernel -std=c++17 -Wall -Wextra -Werror -static -DSTB_SPRINTF_NOFLOAT -m64
NASMPARAMS := -felf64 -F dwarf
LINKERPARAMS := -nostdlib -Wl,--build-id=none -Wl,-z,max-page-size=0x1000,-n,-T,src/linker.ld -fuse-ld=lld
OBJECTS := $(patsubst src/%.cpp, ${OUTPUTDIR}/objects/%.cpp.o, $(shell find src -name *.cpp))
OBJECTS += $(patsubst src/%.asm, ${OUTPUTDIR}/objects/%.asm.o, $(shell find src -name *.asm))
TARGET ?= DEBUG

ifeq (${TARGET}, RELEASE)
	CXXPARAMS += -O3
	LINKERPARAMS += -O3
endif

ifeq (${TARGET}, DEBUG)
	CXXPARAMS += -g
	NASMPARAMS += -g
	LINKERPARAMS += -g
endif

.PHONY: all mkdirs bin

all: mkdirs bin

mkdirs:
	mkdir -p ${OUTPUTDIR}/objects/userland
	mkdir -p ${OUTPUTDIR}/objects/lib
	mkdir -p ${OUTPUTDIR}/objects/hardware/acpi
	mkdir -p ${OUTPUTDIR}/objects/hardware/cpu/smp
	mkdir -p ${OUTPUTDIR}/objects/hardware/devices
	mkdir -p ${OUTPUTDIR}/objects/hardware/mm

${OUTPUTDIR}/objects/%.cpp.o: src/%.cpp
	clang++ $(CXXPARAMS) -o $@ -c $<

${OUTPUTDIR}/objects/%.asm.o: src/%.asm
	nasm $(NASMPARAMS) $< -o $@

bin: ${OBJECTS}
	clang++ ${CXXPARAMS} ${LINKERPARAMS} -o ${OUTPUTDIR}/Kernel.bin $^