# "Majestic" C Runtime

## Requirements

### Debian/Ubuntu

Required system packages (`apt install ...`):
```
autoconf
automake
libtool
make
g++
```

### macOS

- Install XCode from App Store
- Install XCode command line tools: `xcode-select --install`
- Required system packages (`brew install ...`):
```
autoconf
automake
libtool
```

### Windows

- Install MSYS2
- MSYS2 provides two shells MSYS2 MSYS and MSYS2 MinGW. Open MSYS2 MinGW.
- Use pacman to install additional packages:
```
pacman -Syu
pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-libtool
```
By default the runtime compiled with MinGW will depend on libgcc_s_seh-1.dll,
libstdc++-6.dll and libwinpthread-1.dll which are non-standard on Windows. Although
the compiler can in principle link those statically, there is a bug in libtool which
prevents us from using that option. If you want to force static linking, the workaround
is to hack your installation by doing:
```
mv /mingw64/lib/gcc/x86_64-w64-mingw32/10.3.0/libgcc_eh.a /mingw64/lib/gcc/x86_64-w64-mingw32/10.3.0/libgcc_s.a
rm /mingw64/lib/gcc/x86_64-w64-mingw32/10.3.0/libstdc++.dll.a
rm /mingw64/x86_64-w64-mingw32/lib/libpthread.dll.a
```
after the configure step shown bellow.

## Installation

**Note for macOS**: you should first run `glibtoolize`, followed by the commands below.

```
autoreconf -i
./configure
make
make install
```
The shared libraries are installed in `/usr/local/lib`. You can instead use:

## Using

- Compiling GF with this runtime will require flag `--extra-lib-dirs=/usr/local/lib`.
- Running GF with this runtime will require environment variable `LD_LIBRARY_PATH=/usr/local/lib`

This can be avoided by configuring the C runtime with:

```
./configure --prefix=/usr
```

On Windows, the easiest way to let GHC know about the library is to use:

```
./configure --prefix=<GHC-root-directory>/mingw
```

after that move libpgf-0.dll from `<GHC-root-directory>/mingw/bin` to `<GHC-root-directory>/bin`.

## Uninstalling

To remove the _old_ C runtime from your system, do:
```
rm /usr/local/lib/libpgf.*
rm /usr/local/lib/libgu.*
rm /usr/local/lib/libsg.*
rm -rf /usr/local/include/pgf
```

To remove _this_ version of the runtime from your system, do:
```
rm /usr/local/lib/libpgf.*
rm -rf /usr/local/include/pgf
```

To clean all generated build files from this directory, use:
```
git clean -Xdf  
```
