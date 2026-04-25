
# This is a modified version of this GNUmakefile: https://github.com/Limine-Bootloader/limine-c-template/blob/trunk/kernel/GNUmakefile

# * NOTE TO SELF: It's probably a good idea to use $(CURDIR) to get absolute file paths.

# Clear suffix list
.SUFFIXES:

SHELL = /bin/sh

# Name of final executable
override OUTPUT := kernel.elf

override BUILDDIR := build
override INCLUDEDIR := include

ARCH := x86_64
PREFIX := $HOME/gcc-cross

ifeq ($(filter $(ARCH), x86_64),)
    $(error Architecture $(ARCH) not supported)
endif

CFLAGS := -g -O2 -pipe
CPPFLAGS :=
LDFLAGS :=

ifeq ($(ARCH), x86_64)
    NASMFLAGS := -g
endif

# Do not change these 3
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections

override CPPFLAGS := \
    -I . \
    -I limine-protocol/include \
    -isystem freestnd-c-hdrs-0bsd/include \
    $(CPPFLAGS) \
    -MMD \
    -MP

override LDFLAGS += \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    --gc-sections \
    -T linker.lds

# Internal flags specific to an architecture
ifeq ($(ARCH), x86_64)
    override NASMFLAGS := \
        $(patsubst -g, -g -F dwarf, $(NASMFLAGS)) \
        -Wall

	override CFLAGS += \
        -m64 \
        -march=x86-64 \
        -mabi=sysv \
        -mno-80387 \
        -mno-mmx \
        -mno-red-zone \
        -mcmodel=kernel

    override LDFLAGS += \
        -m elf_x86_64

    override NASMFLAGS := \
        -f elf64 \
        $(NASMFLAGS)
endif


# Get all files to be compiled minus architecture specific files and clean up the result
# * Note to self: the first -not -path doesn't exclude the second one!!!
override ALLFILES := $(shell find -L . \( \
		-path "*/arch" -o \
		-path "arch" -o \
		-path "$(BUILDDIR)" -o \
		-path "freestnd-c-hdrs-0bsd" -o \
		-path "limine-protocol" \) -prune \
	-o -type f -print 2>/dev/null | sed "s|^\./||" | LC_ALL=C sort -u)

# Add architecture specific files
override ALLFILES += $(shell find -L arch/$(ARCH) -type f 2>/dev/null | sed "s|^\./||" | LC_ALL=C sort -u)

# Filter by file type
override CFILES := $(filter %.c, $(ALLFILES))
override ASFILES := $(filter %.S, $(ALLFILES))
ifeq ($(ARCH), x86_64)
	override NASMFILES := $(filter %.asm, $(ALLFILES))
endif

# We only append .o so we can set specific compilation rules to distinct file types
override OBJ := $(addprefix $(BUILDDIR)/obj-$(ARCH)/, $(CFILES:.c=.c.o) $(ASFILES:.S=.S.o))
override HEADERDEPS := $(addprefix $(BUILDDIR)/obj/, $(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))
ifeq ($(ARCH), x86_64)
	override OBJ += $(addprefix $(BUILDDIR)/obj-$(ARCH)/, $(NASMFILES:.asm=.asm.o))
endif

# Get all needed directories
override ALLOBJDIRS := $(sort $(dir $(OBJ)))


.PHONY: all createObjDirs getDeps clean
all: $(BUILDDIR)/bin-$(ARCH)/$(OUTPUT)

createObjDirs: $(ALLOBJDIRS)

-include $(HEADER_DEPS)

$(ALLOBJDIRS):
	mkdir -p "$@"

$(BUILDDIR)/bin-$(ARCH)/$(OUTPUT): GNUmakefile linker.lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(LD) $(LDFLAGS) $(OBJ) -o $@

$(BUILDDIR)/obj-$(ARCH)/%.c.o: %.c GNUmakefile createObjDirs
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/obj-$(ARCH)/%.S.o: %.S GNUmakefile createObjDirs
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

ifeq ($(ARCH), x86_64)
$(BUILDDIR)/obj-$(ARCH)/%.asm.o: %.asm GNUmakefile createObjDirs
	nasm $(NASMFLAGS) $< -o $@
endif


getDeps:
	rm -rf cc-runtime limine-protocol freestnd-c-hdrs-0bsd
	mkdir -p cc-runtime limine-protocol freestnd-c-hdrs-0bsd
	git clone --depth 1 https://github.com/osdev0/cc-runtime.git
	git clone --depth 1 https://github.com/Limine-Bootloader/limine-protocol.git
	git clone --depth 1 https://github.com/osdev0/freestnd-c-hdrs-0bsd.git


clean:
	rm -rf $(BUILDDIR)/bin-$(ARCH) $(BUILDDIR)/obj-$(ARCH)
