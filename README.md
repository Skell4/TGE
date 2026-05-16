# TGE — Terminal Graphics Engine

A C++ single-header graphics engine for terminals. Renders pixels via Unicode half-block characters (`▀`) and ANSI true-color escape codes, achieving double vertical resolution per terminal row.

## Requirements

- C++17 or later
- A terminal with true-color (24-bit) and UTF-8 support
- On Windows: the console output codepage is set to UTF-8 automatically

## Installation

Just drop `TGE.hpp` into your project and include it:

```cpp
#include "TGE.hpp"
```

## Classes

### `Color`

Represents an RGBA color (8 bits per channel, alpha defaults to 255).

```cpp
Color red(255, 0, 0);
Color semi(0, 255, 0, 128); // 50% transparent green
```

Alpha blending is handled by `overlapColor()`, used internally by all drawing methods.

**Predefined colors:** `Color::white`, `Color::black`, `Color::red`, `Color::green`, `Color::blue`, `Color::yellow`, `Color::cyan`, `Color::magenta`, `Color::orange`, `Color::purple`, `Color::pink`, `Color::gold`, `Color::silver`, `Color::teal`, and more.

---

### `CImg`

A pixel buffer loaded from a custom binary image format (RGBA, width × height).

```cpp
CImg img("texture.cimg");
if (img.isLoaded()) {
    // use it
}
```

Can also be constructed programmatically from a `std::vector<Color>`.

---

### `Canvas`

The main rendering surface. Takes a width and height in pixels.

```cpp
Canvas canvas(160, 80);
// or with a window title:
Canvas canvas(160, 80, "My App");
```

Each two rows of pixels are merged into one terminal row using `▀`, so a 160×80 canvas occupies 80 terminal rows.

#### Drawing methods

```cpp
canvas.setBG(Color::black);                          // fill background
canvas.drawPixel(x, y, color);                       // single pixel
canvas.drawLine(x0, y0, x1, y1, thickness, color);  // thick line
canvas.drawRect(x, y, w, h, thickness, color);       // hollow rectangle
canvas.drawFilledRect(x, y, w, h, color);            // filled rectangle
canvas.drawCircle(cx, cy, r, color);                 // hollow circle
canvas.drawFilledCircle(cx, cy, r, color);           // filled circle
canvas.drawImg(img, x, y);                           // blit a CImg
canvas.drawTexture(img, x0,y0, x1,y1, x2,y2, x3,y3); // textured quad (barycentric UV)
canvas.drawText(L"Hello!", x, y, color);             // bitmap text
```

#### Rendering

```cpp
canvas.print(); // flush the frame to the terminal (dirty-pixel optimized)
```

Only pixels that changed since the last `print()` are redrawn.

#### Other

```cpp
canvas.sleep(16);        // sleep in milliseconds
canvas.setTitle("TGE"); // set terminal window title
```

---

### `Font`

Internal class used by `drawText()`. Provides a pixel-art bitmap font covering:

- Uppercase and lowercase letters (a–z, A–Z)
- Digits (0–9)
- Italian accented characters (à, è, é, ì, ò, ù)
- Common punctuation and symbols (`!`, `?`, `.`, `,`, `+`, `-`, `*`, `/`, `@`, `#`, `€`, `£`, `$`, `&`, `%`, brackets, etc.)

Unknown characters fall back to `?`.

---

## Example

```cpp
#include "TGE.hpp"

int main() {
    Canvas canvas(120, 60, "TGE Demo");

    // Resize/zoom the terminal window before starting the loop,
    // otherwise pixels may render incorrectly.
    canvas.sleep(2000);

    while (true) {
        canvas.setBG(Color::black);
        canvas.drawFilledCircle(60, 30, 20, Color::cyan);
        canvas.drawText(L"Hello, TGE!", 10, 10, Color::white);
        canvas.print();
        canvas.sleep(16);
    }
}
```

## Compilation

```bash
g++ -std=c++17 -O2 main.cpp -o app
```


## Custom background color

Override the default background before including the header:

```cpp
#define CONSOLE_BG_COLOR Color(20, 20, 30)
#include "TGE.hpp"
```
