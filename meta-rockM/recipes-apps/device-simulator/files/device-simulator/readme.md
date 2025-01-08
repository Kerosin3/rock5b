# CPP EXAMPLE PROJECT

## REQUIREMENTS

1. meson > 0.58
2. ninja
3. clang-format
4. clang-tidy
5. cppcheck

# meson setup and run

1.  Setup build `meson setup builddir --buildtype=release`
2.  Compile build `meson compile -C builddir`

# good coding

1. ninja -C builddir cppcheck
2. ninja -C builddir clang-tidy
2. ninja -C builddir clang-format
