## Luci
A simple and lightweight graphics library for C
## Basic example
```C
int main()
{
    LUCI_init();
    while (!LUCI_window_should_close()) {
        LUCI_begin_drawing();
        LUCI_draw_rectangle(32,32,64,64,LUCI_COLOR_BLUE);
        LUCI_end_drawing();
    }
    LUCI_close();
    return 0;
}
```
## Installation
```bash
git clone https://github.com/WibuKa/luci.git
cd luci
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
This produces a single self contained `build/libluci.a`.

## Linking
`libluci.a` already contains LUCI, glad, FreeType, RGFW and stb_image, so only
`luci.h` and `libluci.a` are needed. The operating system libraries used by the
window backend still have to be linked:

| Platform | Libraries |
| --- | --- |
| Linux | `-lX11 -lXrandr -lm` (`-ldl` on glibc &lt; 2.34) |
| Windows | `-lgdi32` |
| macOS | `-framework Cocoa -framework CoreVideo -framework IOKit` |

```bash
cc main.c -Iinclude -Llib -l:libluci.a -lX11 -lXrandr -lm -o app
```

From CMake, link the `luci` target; the system libraries come with it.
