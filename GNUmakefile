
# This is a modified version of this GNUmakefile: https://github.com/Limine-Bootloader/limine-c-template/blob/trunk/GNUmakefile

# Clear suffix list
.SUFFIXES:

SHELL = /bin/sh

ARCH := x86_64
QEMUFLAGS := -m 2G

ifeq ($(filter $(ARCH), x86_64),)
    $(error Architecture $(ARCH) not supported)
endif

override IMAGENAME := featherOS-$(ARCH)
override KERNELBUILDDIR := kernel/build
override KERNELOUTPUT := kernel.elf

# Toolchain for building the 'limine' executable for the host
HOSTCC := cc
HOSTCFLAGS := -g -O2 -pipe
HOSTCPPFLAGS :=
HOSTLDFLAGS :=
HOSTLIBS :=

.PHONY: all all-hdd run getKernelDeps kernel clean distclean

all: $(IMAGENAME).iso
all-hdd: $(IMAGENAME).hdd

# Limine stuff
limine/limine:
	rm -rf limine
	git clone https://github.com/Limine-Bootloader/Limine.git limine --branch=v11.x-binary --depth=1
	$(MAKE) -C limine \
		CC="$(HOSTCC)" \
		CFLAGS="$(HOSTCFLAGS)" \
		CPPFLAGS="$(HOSTCPPFLAGS)" \
		LDFLAGS="$(HOSTLDFLAGS)" \
		LIBS="$(HOSTLIBS)"


getKernelDeps:
	$(MAKE) -C kernel getDeps


# I think we should compile the kernel, right?
kernel:
	$(MAKE) -C kernel


$(IMAGENAME).iso: limine/limine kernel
	rm -rf isoRoot
	mkdir -p isoRoot/boot/limine
	mkdir -p isoRoot/EFI/BOOT
	cp -v $(KERNELBUILDDIR)/bin-$(ARCH)/$(KERNELOUTPUT) isoRoot/boot
	cp -v limine.conf isoRoot/boot/limine

ifeq ($(ARCH), x86_64)
	cp -v limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin isoRoot/boot/limine/
	cp -v limine/BOOTX64.EFI isoRoot/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI isoRoot/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		isoRoot -o $(IMAGENAME).iso
	./limine/limine bios-install $(IMAGENAME).iso
endif

	rm -rf isoRoot

$(IMAGENAME).hdd: limine/limine kernel
	rm -f $(IMAGENAME).hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(IMAGENAME).hdd

ifeq ($(ARCH), x86_64)
	PATH=$$PATH:/usr/sbin:/sbin sgdisk $(IMAGENAME).hdd -n 1:2048 -t 1:ef00 -m 1
	./limine/limine bios-install $(IMAGENAME).hdd
else
	PATH=$$PATH:/usr/sbin:/sbin sgdisk $(IMAGENAME).hdd -n 1:2048 -t 1:ef00
endif

	mformat -i $(IMAGENAME).hdd@@1M
	mmd -i $(IMAGENAME).hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i $(IMAGENAME).hdd@@1M $(KERNELBUILDDIR)/bin-$(ARCH)/$(KERNELOUTPUT) ::/boot
	mcopy -i $(IMAGENAME).hdd@@1M limine.conf ::/boot/limine

ifeq ($(ARCH), x86_64)
	mcopy -i $(IMAGENAME).hdd@@1M limine/limine-bios.sys ::/boot/limine
	mcopy -i $(IMAGENAME).hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i $(IMAGENAME).hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT
endif


run:
	qemu-system-$(ARCH) -M q35 \
		-cdrom $(IMAGENAME).iso \
		$(QEMUFLAGS)

clean:
	$(MAKE) -C kernel clean
	rm -rf isoRoot

distclean: clean
	rm -f $(IMAGENAME).iso $(IMAGENAME).hdd