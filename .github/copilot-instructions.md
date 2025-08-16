# Copilot Coding Agent Instructions for konstruct

## Project Overview
- **konstruct** is a simple educational operating system with a custom kernel, C library (libc), graphics subsystem, and filesystem. It is designed for 32-bit x86, booted via GRUB, and runs in QEMU for development.
- Major components: `kernel.c` (main kernel), `libc/` (freestanding C library), `fs/` (filesystem), `drivers/` (device drivers), `grub/` (bootloader config), `samples/` (user programs), `syscalls/` (system call interface).

## Architecture & Data Flow
- **Boot**: GRUB loads the kernel (see `grub/grub.cfg`, `linker.ld`).
- **Kernel**: Handles interrupts, memory, system calls, and program execution. Entry: `kernel.c`.
- **libc**: Implements standard C functions for a freestanding environment. System calls are routed via `syscalls/`.
- **Filesystem**: Hierarchical, mountable, supports CD-ROM, HDD, USB. Key: `fs/fs.c`, `fs/bootdev.c`. Mount points: `/cdrom`, `/media`.
- **Graphics**: Text mode (80x25), VGA (320x200), VESA. Font rendering via BMP files. See `libc/drivers/font_bmp.c`, `libc/drivers/font_data.c`.
- **Shell**: Command-line interface in `programs/shell.c`.

## Developer Workflows
- **Build all**: `./build_all.sh` (compiles kernel, libc, creates ISO)
- **Test BIOS ASM**: `./test_build_bios.sh`
- **Run in QEMU**: `qemu-system-i386 -cdrom build/konstruct.iso`
- **Debug**: QEMU with GDB: `qemu-system-i386 -cdrom build/konstruct.iso -s -S` + `gdb -ex "target remote localhost:1234" -ex "symbol-file build/kernel.bin"`
- **libc only**: `cd libc && make`

## Project-Specific Conventions
- **Freestanding C**: No reliance on host OS; libc is custom and minimal.
- **System Calls**: Implemented in `syscalls/`, invoked via libc wrappers.
- **Filesystem API**: Use `fs_*` functions for file/dir ops. Mount points are fixed (`/cdrom`, `/media`).
- **Graphics API**: Use `vga_*`, `font_*` functions for drawing. Font data is in BMP or C arrays.
- **Shell Commands**: `ls`, `cd`, `pwd`, `mkdir`, `touch`, `rm`, `cat`, `echo > file`.
- **No dynamic linking**: All code is statically linked.

## Integration Points
- **GRUB**: Bootloader config in `grub/grub.cfg`.
- **QEMU**: Used for emulation/testing.
- **GCC/NASM**: Cross-compilation for i386.
- **VESA/VGA**: Graphics via BIOS/driver calls.

## Key Patterns & Examples
- **Add libc function**: Implement in `libc/`, declare in header, update Makefile.
- **Filesystem usage**: `fs_read`, `fs_write`, `fs_mount` (see `fs/fs.c`).
- **Graphics usage**: `vga_set_mode`, `font_draw_string` (see `libc/drivers/`).
- **Shell extension**: Add command in `programs/shell.c`.

## References
- [README.md](../README.md) for build/run/dev info
- [docs/LIBC.md](../docs/LIBC.md) for libc details
- [docs/FILESYSTEM.md](../docs/FILESYSTEM.md) for filesystem
- [docs/GRAPHICS.md](../docs/GRAPHICS.md) for graphics
- [CONTRIBUTING.md](../CONTRIBUTING.md) for contribution process

---

**If unclear about a workflow or pattern, check the referenced docs or ask for clarification.**
