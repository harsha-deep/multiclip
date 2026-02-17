# multiclip

## How to run
```bash
sudo apt update
sudo apt install build-essential meson ninja-build pkg-config
sudo apt install libgtkmm-3.0-dev
```

```bash
meson setup build
meson compile -C build
./build/multiclip
```