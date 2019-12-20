cmake_minimum_required (VERSION 3.9)
project ("Spark Kernel")

set (CMAKE_CXX_FLAGS "-target x86_64-unknown-elf -ffreestanding -fno-use-cxa-atexit -fno-pic -nostdlib -fno-pie -mno-sse -mno-sse2 -fno-builtin -fno-rtti -fno-exceptions -fsigned-char -fno-stack-protector -mno-red-zone -mcmodel=kernel -std=c++17 -Wall -Wextra -Werror -static -g -DSTB_SPRINTF_NOFLOAT -m64 -DKERNEL")
set (CMAKE_EXE_LINKER_FLAGS "-target x86_64-linux-elf -nostdlib -Wl,--build-id=none -Wl,-z,max-page-size=0x1000,-n,-T,${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld -fuse-ld=lld")
enable_language (ASM_NASM)

file (GLOB_RECURSE CPP_SOURCES "src/*.cpp")
file (GLOB_RECURSE ASM_SOURCES "src/*.asm")

include_directories ("src")

add_executable (kernel.bin ${CPP_SOURCES} ${ASM_SOURCES})