# TGE

TGE is a C++ library for drawing images and graphics primitives in a terminal using RGB colors. The project also includes `img_to_cimg.html`, a local browser page that converts images into `.cimg` binary files that the library can read.

## Why TGE?

TGE began as a personal passion project for building applications in the terminal. Its broader goal is to make small graphical interfaces useful in programs running in environments without a desktop GUI, where a terminal is available but a compact visual interface is still valuable.

## Project files

- [`TGE.hpp`](TGE.hpp): the library, containing the `Color`, `Canvas`, and `Font` classes.
- [`img_to_cimg.html`](img_to_cimg.html): the image-to-`.cimg` converter.

In the current version, `CImg` is an alias for `Canvas`; in earlier versions, it was a separate class.

## Requirements and inclusion

The library requires **C++17**. It is distributed as a header; include it in your source file:

```cpp
#include "TGE.hpp"
```

Example compilation with GCC or Clang:

```sh
g++ main.cpp -std=c++17 -O2 -o program
```

With MSVC on Windows, enable C++17 (for example, `/std:c++17`). The library uses 24-bit ANSI escape sequences and the Unicode character `▀`, so the terminal must support true-color and UTF-8. On Windows, the library enables VT processing and UTF-8 output when it initializes the terminal.

## Quick start: draw in the terminal

`Canvas` dimensions are measured in **pixels**, not terminal characters. The renderer displays two vertical pixels in each terminal cell: the top pixel sets the foreground color of `▀`, and the bottom pixel sets its background color. Therefore, an 80-column by 24-row terminal can display about 80 × 48 pixels.

```cpp
#include "TGE.hpp"
#include <iostream>

int main() {
    Canvas::initializeTerminal();

    auto [width, height] = Canvas::getScreenSize();
    Canvas canvas(width, height, "TGE demo");
    canvas.setBG(Color::black);
    canvas.drawFilledRect(2, 2, 24, 12, Color::blue);
    canvas.drawCircle(14, 8, 5, Color::yellow);
    canvas.print();

    std::cin.get(); // Keep the screen visible until Enter is pressed.
    Canvas::resetTerminal();
}
```

Call `Canvas::initializeTerminal()` before `print()` and `Canvas::resetTerminal()` when finished. The first function opens the alternate screen and hides the cursor; the second restores the previous screen and shows the cursor again. When possible, `print()` updates only the cells that changed since the previous frame.

## Load and display an image

```cpp
CImg image("photo.cimg");

Canvas::initializeTerminal();
auto [width, height] = Canvas::getScreenSize();
Canvas screen(width, height);
screen.drawImg(image, 0, 0); // Pixel coordinates; areas outside the canvas are clipped.
screen.print();
std::cin.get();
Canvas::resetTerminal();
```

The filename constructor loads a `.cimg` file. It throws `std::runtime_error` if the file cannot be opened or is invalid. Alternatively, create a `Canvas` and call `load()`, which returns `true` on success and `false` on failure.

## Convert an image with `img_to_cimg.html`

1. Open `img_to_cimg.html` in a modern browser.
2. Drag an image into the drop area, or click the area to select a file.
3. Check the preview, dimensions, and estimated `.cimg` size.
4. Click **Convert and Download .cimg**. The browser downloads a file with the same base name and the `.cimg` extension.

The page runs locally in the browser and does not upload images to a server. It accepts image files the browser can decode (the file picker uses `image/*`; the page mentions PNG, JPG, BMP, GIF, WEBP, and TIFF). Conversion preserves the image dimensions and writes one RGBA pixel per source pixel; there are no resizing or other conversion options.

## `.cimg` format

The file is uncompressed binary data with this layout:

| Offset | Length | Contents |
| --- | ---: | --- |
| 0 | 4 bytes | width (`uint32_t`, little-endian) |
| 4 | 4 bytes | height (`uint32_t`, little-endian) |
| 8 | 4 bytes per pixel | pixels in row order: red, green, blue, alpha (one byte each) |

The first pixel is at the top-left. Pixels then run left to right and top to bottom. The total size is `8 + width × height × 4` bytes. The converter writes dimensions in little-endian order; `TGE.hpp` reads the two integers in the machine's native byte order, so the format is directly compatible with common little-endian platforms.

## Public API

### `Color`

Represents a color with RGBA components from 0 to 255. It does not provide getters for individual channels.

| Constructor / function | Description |
| --- | --- |
| `Color()` | Opaque black (`0,0,0,255`). |
| `Color(r, g, b, a = 255)` | Creates a color. Values outside `0-255` cause `std::invalid_argument`. |
| `setRGB(r, g, b)` | Sets RGB and makes alpha 255. |
| `setRGBA(r, g, b, a)` | Sets all four channels. |
| `overlapColor(Color above)` | Returns the result of drawing `above` over the current color, accounting for alpha. |
| `toString()` | Returns the channels as `r,g,b,a` text. |
| `toChars(char* buf)` | Writes RGB to the buffer as `r;g;b` and returns the character count; used by the ANSI renderer. |
| `operator==`, `operator=` | Compare and assign all four channels. |

Built-in colors, available as `Color::name`:

| Name | RGB | Name | RGB |
| --- | --- | --- | --- |
| `white` | `255,255,255` | `black` | `0,0,0` |
| `red` | `255,0,0` | `green` | `0,255,0` |
| `blue` | `0,0,255` | `yellow` | `255,255,0` |
| `cyan` | `0,255,255` | `magenta` | `255,0,255` |
| `light_grey` | `200,200,200` | `grey` | `128,128,128` |
| `dark_grey` | `50,50,50` | `orange` | `255,165,0` |
| `purple` | `128,0,128` | `pink` | `255,192,203` |
| `brown` | `139,69,19` | `lime` | `50,205,50` |
| `navy` | `0,0,128` | `teal` | `0,128,128` |
| `olive` | `128,128,0` | `gold` | `255,215,0` |
| `silver` | `192,192,192` | `bronze` | `205,127,50` |
| `sky_blue` | `135,206,235` | `violet` | `238,130,238` |
| `indigo` | `75,0,130` | `turquoise` | `64,224,208` |

To change the initial canvas background color, define `CONSOLE_BG_COLOR` **before** including `TGE.hpp`. Its default value is `Color(12,12,12)`.

### `Canvas` / `CImg`

#### Constructors

- `Canvas()` creates an uninitialized canvas; call `resize()` before drawing on it.
- `Canvas(width, height)` creates a surface filled with `CONSOLE_BG_COLOR`.
- `Canvas(width, height, title)` creates the same surface and sets the terminal window title.
- `Canvas(width, height, pixels)` creates a surface from the supplied `std::vector<Color>` in row order; the vector must contain `width × height` pixels.
- `Canvas(file_name)` loads a `.cimg` file.

Zero dimensions or a pixel vector with the wrong size cause `std::invalid_argument`. Operations that require a valid surface cause `std::logic_error` if the canvas has not been initialized.

#### Terminal and dimensions

| Function | Description |
| --- | --- |
| `Canvas::initializeTerminal()` | Activates the alternate screen, clears it, and hides the cursor. |
| `Canvas::resetTerminal()` | Restores the previous terminal mode, cursor, and screen. |
| `Canvas::isTerminalInitialized()` | Reports whether terminal initialization is active. |
| `Canvas::setTitle(title)` | Sets the terminal window title. This is a static function. |
| `Canvas::getScreenSize()` | Returns `{width, height}` in graphics pixels; terminal row count is doubled for the two-pixel renderer. |
| `canvas.print()` | Displays the frame. Requires `initializeTerminal()`; otherwise throws `std::logic_error`. |
| `Canvas::sleep(ms)` | Waits for the specified number of milliseconds. |

#### State and images

| Function | Description |
| --- | --- |
| `load(file_name)` | Loads a `.cimg` file and returns `bool`. |
| `getWidth()`, `getHeight()` | Return the dimensions in pixels. |
| `isInitialized()` | Reports whether the canvas has valid dimensions and pixels. |
| `setImageMode(bool)`, `isImage()` | Set or read the flag that identifies the canvas as an image; `print()` sets it to `false`. |
| `scaled(width, height)` | Returns a new image resized with nearest-neighbor sampling. Dimensions must be positive. |
| `screenshot()` | Returns a copy of the current pixels in a new `Canvas`. |
| `resize(width, height)` | Resizes and fills with `CONSOLE_BG_COLOR`. Returns `false` if the dimensions were already the requested size, otherwise `true`. Zero dimensions cause `std::invalid_argument`. |
| `clear()` | Releases the buffers, resets the dimensions, and marks the canvas uninitialized; it can be reused with `resize()`. |
| `operator=(Canvas)`, `operator==(Canvas)` | Copy a canvas and compare its dimensions and pixels. |

#### Drawing

Coordinates start at `(0, 0)` in the top-left corner. Drawing outside the canvas is clipped; pixels outside its area are not drawn.

| Function | Description |
| --- | --- |
| `setBG(Color)` | Fills the entire surface with the specified color. |
| `drawPixel(x, y, Color)` | Draws one pixel. |
| `drawLine(x0, y0, x1, y1, thickness, Color)` | Draws a line with the specified thickness; non-positive thickness draws nothing. |
| `drawCircle(cx, cy, r, Color)` | Draws a circle outline. |
| `drawFilledCircle(cx, cy, r, Color)` | Draws a filled circle. |
| `drawRect(x, y, w, h, thickness, Color)` | Draws a rectangle outline. |
| `drawFilledRect(x, y, w, h, Color)` | Draws a filled rectangle. |
| `drawImg(img, x, y)` | Draws an image at the given position, clipping it at the canvas edges; returns `bool`. |
| `drawText(text, x, y, Color)` | Draws a wide string left-to-right using `Font`'s bitmap glyphs. Text outside the canvas is clipped. |
| `drawTexture(img, x0, y0, x1, y1, x2, y2, x3, y3)` | Projects a texture onto the four supplied vertices by rasterizing the quadrilateral as two triangles and sampling the nearest pixels; returns `bool`. |

Pixel and color alpha values are used when drawing over existing pixels. `drawImg()` and the drawing primitives work on the canvas's current pixels. To clear or cover a frame, use `setBG()` or call `clear()` followed by `resize()`.

### Text rendering and `Font`

Use `Canvas::drawText()` to draw a string on a canvas:

```cpp
canvas.drawText(L"Hello, TGE!", 2, 2, Color::white);
```

Its signature is `void drawText(const std::wstring& text, int x, int y, Color color)`. The string is drawn on one line from `(x, y)`, advancing by the bitmap width of each character. Unsupported characters are rendered as `?`; line breaks are not interpreted specially. `Font::basic_char(wchar_t ch, Color color)` is also available when a bitmap for a single glyph is needed.

`drawText()` supports these characters:

- Lowercase letters: `a-z`, plus `à è é ì ò ù`.
- Uppercase letters: `A-Z`.
- Digits: `0-9`.
- Symbols: space, `! ? . , : ; ' " + - * / \ | ( ) [ ] { } = & % £ $ € _ # @ < > ^`.

Example: `CImg letter = Font::basic_char(L'A', Color::white);`.
