
# Clear suffix list
.SUFFIXES:

SHELL = /bin/sh
ARCH := x86_64

ifeq ($(filter $(ARCH), x86_64),)
    $(error Architecture $(ARCH) not supported)
endif

# Change PREFIX if the cross compiler is in another directory
PREFIX := /usr/local/gcc-cross
TOOLCHAINPREFIX := $(ARCH)-elf-

CC := $(PREFIX)/bin/$(TOOLCHAINPREFIX)gcc
LD := $(PREFIX)/bin/$(TOOLCHAINPREFIX)ld
AS := $(PREFIX)/bin/$(TOOLCHAINPREFIX)as
AR := $(PREFIX)/bin/$(TOOLCHAINPREFIX)ar
STRIP := $(PREFIX)/bin/$(TOOLCHAINPREFIX)strip

CFLAGS := -g -O2 -pipe
CPPFLAGS :=
LDFLAGS :=

# Name of final executable
override OUTPUT := kernel.elf

BUILDDIR := build
INCLUDEDIR := include

ifeq ($(ARCH), x86_64)
    NASMFLAGS := -g
endif

override CFLAGS += \
    -Wall \
    -Wextra \
	-Wpedantic \
    -std=gnu17 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -ffunction-sections \
    -fdata-sections \
    -fexec-charset=cp437

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
        -Wall \
        -f elf64

	override CFLAGS += \
        -m64 \
        -march=x86-64 \
        -mabi=sysv \
        -mno-80387 \
        -mgeneral-regs-only \
        -mno-red-zone \
        -mcmodel=kernel

    override LDFLAGS += \
        -m elf_x86_64

    override NASMFLAGS := \
        -f elf64 \
        $(NASMFLAGS)
endif


# Get all files to be compiled minus architecture specific files and clean up the result
# * Note to self: the first -path doesn't exclude the second one!!!
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
override ALLOBJDIRS := $(sort $(dir $(OBJ))) $(BUILDDIR)/bin-$(ARCH)


.PHONY: all createObjDirs getDeps clean distclean
all: $(BUILDDIR)/bin-$(ARCH)/$(OUTPUT)

createObjDirs: $(ALLOBJDIRS)

-include $(HEADER_DEPS)

$(ALLOBJDIRS): GNUmakefile getDeps
	mkdir -p "$@"

$(BUILDDIR)/bin-$(ARCH)/$(OUTPUT): GNUmakefile linker.lds $(OBJ)
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
ifeq ("$(wildcard limine-protocol freestnd-c-hdrs-0bsd)", "")
	rm -rf limine-protocol freestnd-c-hdrs-0bsd
	mkdir -p limine-protocol freestnd-c-hdrs-0bsd
	git clone --depth 1 https://github.com/Limine-Bootloader/limine-protocol.git
	git clone --depth 1 https://github.com/osdev0/freestnd-c-hdrs-0bsd.git
endif

clean:
	rm -rf $(BUILDDIR)/bin-$(ARCH) $(BUILDDIR)/obj-$(ARCH)

distclean: clean
	rm -rf freestnd-c-hdrs-0bsd limine-protocol
