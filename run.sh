find . \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 clang-format -i
meson setup build
meson compile -C build
./build/src/multiclip
