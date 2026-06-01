
# LeOS
A WIP x86_64 hobby operating system made in C and Assembly.

## 1. Why this project?
Why not? I like challenges so I wanted to give my shot at creating an operating system. I want to have deeper insights on how computers work and create projects to sharpen my skills and abilities.

## 2. Compiling
Before compiling you need:
 - a GCC cross compiler(x86_64)
 - NASM
 - internet connection

 An internet connection is needed to download needed dependencies.

 It is assumed your toolchain is in `/usr/local/gcc-cross`. To use a toolchain in a different path change `PREFIX` and `TOOLCHAINPREFIX` variables like so:
 ```sh
make targetHere PREFIX=yourPathHere TOOLCHAINPREFIX=yourPrefixHere
 ```

By default they are set like so: `PREFIX=/usr/local/gcc-cross` `TOOLCHAINPREFIX=x86_64-elf-`

### 2.1 Generating OS images
By running the following commands everything will be compiled and included in the os image.

To generate a .iso image:
```sh
make all
```

To generate a .hdd image:
```sh
make all-hdd
```

### 2.2 Compiling only the kernel
From the main project directory:
```sh
make kernel
```

or from the `kernel` directory run:
```sh
make
```