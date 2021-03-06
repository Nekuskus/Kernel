CXXPARAMS = -target x86_64-unknown-elf -Isrc -ffreestanding -fno-use-cxa-atexit -fno-pic -nostdlib -fno-pie -mno-sse -mno-sse2 -fno-builtin -fno-rtti -fno-exceptions -fsigned-char -fno-stack-protector -mno-red-zone -mcmodel=kernel -std=c++17 -Wall -Wextra -Werror -static -DSTB_SPRINTF_NOFLOAT -m64
NASMPARAMS = -felf64 -F dwarf
REALPARAMS = -f bin
LINKERPARAMS = -nostdlib -Wl,--build-id=none -Wl,-z,max-page-size=0x1000,-n,-T,src/linker.ld -fuse-ld=lld
OBJ = $(patsubst src/%.cpp, $(OUTPUTDIR)/objects/%.cpp.o, $(shell find src -type f -name '*.cpp')) $(patsubst src/%.asm, $(OUTPUTDIR)/objects/%.asm.o, $(shell find src -type f -name '*.asm')) 
REALFILES = $(shell find src -type f -name '*.real')
BINS = $(REALFILES:.real=.bin)
TARGET ?= Debug

ifeq ($(TARGET), Release)
	CXXPARAMS += -O3
	LINKERPARAMS += -O3
endif

ifeq ($(TARGET), Debug)
	CXXPARAMS += -g
	NASMPARAMS += -g
	REALPARAMS += -g
	LINKERPARAMS += -g
endif

.PHONY: all

$(OUTPUTDIR)/objects/%.cpp.o: src/%.cpp
	mkdir -p $(dir $@)
	clang++ $(CXXPARAMS) -o $@ -c $<

$(OUTPUTDIR)/objects/%.asm.o: src/%.asm
	mkdir -p $(dir $@)
	nasm $(NASMPARAMS) $< -o $@

src/%.bin: src/%.real
	nasm $(REALPARAMS) $< -o $@

all: $(BINS) $(OBJ)
	clang++ $(patsubst x86_64-unknown-elf, x86_64-linux-elf, $(CXXPARAMS)) $(LINKERPARAMS) -o $(OUTPUTDIR)/Kernel.bin $(OUTPUTDIR)/objects/crtbegin.asm.o $(patsubst src/%.bin, ,$(patsubst $(OUTPUTDIR)/objects/crt%, ,$^)) $(OUTPUTDIR)/objects/crtend.asm.o