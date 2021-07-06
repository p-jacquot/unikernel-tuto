# unikernel-tuto
Small tutorial for unikernels and OpenMP applications, for the PRACE-training week of july 2021.

This tutorial has been made for running on ubuntu 18 & 20, and debian 9 & 10, with KVM enabled.
If you are working on a virtual machine, be sure to have the nested-kvm module enabled.

## Installation

### Required packages

Install the following packages. They will be required for installing and using unikernels.

```
apt-get update
apt-get install git build-essential cmake nasm apt-transport-https wget libgmp-dev bsdmainutils libseccomp-dev python libelf-dev
```

### HermitCore

In order to build your unikernel application, HermitCore comes with its own toolchain.

You can install the toolchain with the following shell code :

```
for dep in binutils-hermit_2.30.51-1_amd64.deb gcc-hermit_6.3.0-1_amd64.deb \
        libhermit_0.2.10_all.deb  newlib-hermit_2.4.0-1_amd64.deb; do \
    wget https://github.com/ssrg-vt/hermitux/releases/download/v1.0/$dep && \
    sudo dpkg -i $dep && \
    rm $dep;
done
```

Installing HermitCore this way will put the toolchain at `/opt/hermit/`. If you decide to compile it yourself, you will be able to choose where it will be installed.

> In this repo, we assume that the toolchain is located at `/opt/hermit`

### HermiTux

Before installing HermiTux, be sure that you have done the previous step for installing HermitCore on your machine.

To build Hermitux succesfully, you will need GCC 8 or an older version. If you try to use GCC 9, the compilation of the openMP runtime will fail. You can install GCC 8 with the following command :

```
apt-get install gcc-8
```

Clone and build the repo with a make command. Don't forget to export `CC=gcc-8` variable before starting the compilation, so that your gcc-8 will be used for compiling.

```
CC=gcc-8
git clone https://github.com/ssrg-vt/hermitux
cd hermitux
git submodule init && git submodule update
make
```

## Compiling applications with unikernels

### HermitCore

HermitCore require to compile the application and the HermitCore kernel together. This can be done thanks to its own compiler : `x86_64-hermit-gcc`.

To compile simple applications, use the following command:
```
/opt/hermit/bin/x86_64-hermit-gcc -o target *.c
```

To compile OpenMP applications, use the the following command:
```
/opt/hermit/bin/x86_64-hermit-gcc -o target -fopenmp *.c
```

### HermiTux

HermiTux is a binary compatible unikernel. It does not need to recompile the application, if it is a Linux binary executable.

HermiTux is able to execute dynamically linked executables, and statically linked executables. But to stay simple during this tutorial, we'll focus on the statically linked binaries executions. Fell free to look at the [HermiTux](https://github.com/ssrg-vt/hermitux) repository if you are more interested in dynamically linked applications.

Compiling simple applications is easy with HermiTux:
```
gcc -static -o target *.c
```

> You are free to use the compiler you want, as soon it produces binaries executable by the Linux kernel. When making this tutorial, we used GCC and Clang compiler successfully.

Commpiling OpenMP application is a bit more difficult for two reasons:
* If we want to build a static executable, we need to link our OpenMP runtime statically with our executable, which can be tricky sometimes.
* If we decide to execute a dynamically linked OpenMP application, we may experiences errors at loading time, when the OpenMP runtime is fetch by the loader (bad OpenMP runtime by default, not found, etc.)
This is why we advice to compile OpenMP with HermiTux' wrapper compiler, to avoid most of the troubles mentionned above:
```
$(PATH_TO_HERMITUX_FOLDER)/hermitux/musl/obj/musl-gcc -fopenmp -L$(PATH_TO_HERMITUX_FOLDER)/hermitux/libiomp/build/runtime/src -o target *.c
```

## Applications

### hello

Hello is juste a simple "Hello World" application. It is designed to see if a unikernel can be launched, and execute a program.

### omp-test

This program is a simple program to test if unikernels are able to execute successfully OpenMP applications. It justs performs some prints on the standard outputs, showing which threads is computing which iteration of a for loop.

### nqueens

Nqueens is a benchmarks coming from the [Bots benchmarks suite](https://github.com/bsc-pm/bots). There are two versions of this benchmark, because HermitCore cannot execute any of the bots benchmarks if they are not slightly modified.

* nqueens is the default nqueens benchmark, coming from the Bots benchmarks.
* nqueens-hermitcore is the nqueens benchmark, ported to be compilable by HermitCore's compiler.

## Executing applications

### HermitCore applications execution

For single threaded and single core applications, use the following command:

```
HERMIT_ISLE=uhyve /opt/hermit/bin/proxy application
```

> uhyve is a lightweight hypervisor developped for HermitCore. `HERMIT_ISLE=uhyve` tells the proxy tool to use uHyve hypervisor for executing HermitCore.

> proxy is a launcher that tweaks the launch of HermitCore by selecting the hypervisor to use, the amount of memory to allocate to the unikernel, etc.

For multi-thread and multi-core OpenMP applications, use the following command:

```
HERMIT_SILE=uhyve HERMIT_CPUS=[NUMBER] OMP_NUM_THREADS=[NUMBER] /opt/hermit/bin/proxy application
```


### HermiTux applications execution

HermiTux also use a proxy tool, as HermitCore, and can be virtualised with uHyve hypervisor.

The command for launching a **statically linked application** with HermiTux is the following:

```
HERMIT_ISLE=uhyve HERMIT_TUX=1 \
$(PATH_TO_HERMITUX_FOLDER)/hermitux/hermitux-kernel/prefix/bin/proxy \
$(PATH_TO_HERMITUX_FOLDER)/hermitux/hermitux-kernel/prefix/x86_64-hermit/extra/tests/hermitux \
application
```

For OpenMP applications, don't forget to specify `HERMIT_CPUS` and `OMP_NUM_THREADS` variables:
```
HERMIT_ISLE=uhyve HERMIT_TUX=1 \
HERMIT_CPUS=[NUMBER] OMP_NUM_THREADS=[NUMBER] \
$(PATH_TO_HERMITUX_FOLDER)/hermitux/hermitux-kernel/prefix/bin/proxy \
$(PATH_TO_HERMITUX_FOLDER)/hermitux/hermitux-kernel/prefix/x86_64-hermit/extra/tests/hermitux \
application
```

### proxy environment variables

With the proxy tool of HermitCore and HermiTux, you can tweak the execution of theses unikernels. Here is a non-exhaustive list of environment variables that the proxy use:

* HERMIT_ISLE: `uhyve`, `kvm` or `qemu`. Selects the hypervisor to use.
* HERMIT_CPUS: set the number of cores to allocate to the unikernel. (If you're executing an OpenMP application, make sure to set the number of threads, it is set to 1 by default)
* HERMIT_MEM: for example `4G`. Memory to allocate for the unikernel execution.
* HERMIT_VERBOSE: `1` or `0`. Print the kernel logs when the execution of the application is over.
