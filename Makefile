CXXPARAMS := -target x86_64-unknown-elf -Isrc -ffreestanding -fno-use-cxa-atexit -fno-pic -nostdlib -fno-pie -mno-sse -mno-sse2 -fno-builtin -fno-rtti -fno-exceptions -fsigned-char -fno-stack-protector -mno-red-zone -mcmodel=kernel -std=c++17 -Wall -Wextra -Werror -static -g -DSTB_SPRINTF_NOFLOAT -m64
NASMPARAMS := -felf64 -F dwarf
LINKERPARAMS := -target x86_64-linux-elf -nostdlib -Wl,--build-id=none -Wl,-z,max-page-size=0x1000,-n,-T,src/linker.ld -fuse-ld=lld
OBJECTS := $(patsubst src/%.cpp, ${OUTPUTDIR}/objects/%.o, $(shell find src -name *.cpp))
OBJECTS += $(patsubst src/%.asm, ${OUTPUTDIR}/objects/%.o, $(shell find src -name *.asm))
TARGET := DEBUG

ifeq (${TARGET}, RELEASE)
	CLANGPARAMS += -O3
	LDPARAMS += -O3
endif

ifeq (${TARGET}, DEBUG)
	CLANGPARAMS += -g
	NASMPARAMS += -g
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

${OUTPUTDIR}/objects/%.o: src/%.cpp
	clang++ $(CXXPARAMS) -o $@ -c $<

${OUTPUTDIR}/objects/%.o: src/%.asm
	nasm $(NASMPARAMS) $< -o $@

bin: ${OBJECTS}
	clang++ ${LINKERPARAMS} -o ${OUTPUTDIR}/Kernel.bin $^