find . \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 clang-format -i
meson setup build
meson compile -C build
./build/src/multiclip


# debug 1
# meson setup build --buildtype=debug
# valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./build/src/multiclip

# debug 2
# valgrind --vgdb=yes --vgdb-error=0 ./build/src/multiclip
# gdb ./build/src/multiclip
# target remote | vgdb
# continue