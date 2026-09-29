#ifndef TGE_HPP

#define TGE_HPP

//TGE 2.0

#include <vector>
#include <stdexcept>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <charconv>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <utility>

#ifdef _WIN32
    #include <windows.h>

    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#else
    #include <unistd.h>   
    #include <cerrno>
    #include <sys/ioctl.h> 
#endif

#ifndef CONSOLE_BG_COLOR
    #define CONSOLE_BG_COLOR Color(12,12,12)
#endif

class Color
{
private:
    uint8_t _r;
    uint8_t _g;
    uint8_t _b;
    uint8_t _a;
public:
    Color() : _r(0), _g(0), _b(0) ,_a(255) { }

    Color(unsigned int r,unsigned int g,unsigned int b,unsigned int a = 255){
        setRGBA(r,g,b,a);
    }

    Color(const Color& other) = default;

    void setRGB(unsigned int r,unsigned int g,unsigned int b){
        if(r > 255 || g > 255 || b > 255){
            throw std::invalid_argument("Error: rgb(0-255)!");
        } else {
            _r = r; 
            _g = g; 
            _b = b;
            _a = 255;
        }
    }
    
    void setRGBA(unsigned int r,unsigned int g,unsigned int b,unsigned int a){
        if(a > 255){
            throw std::invalid_argument("Error: rgba(0-255)!");
        } else {
            setRGB(r,g,b);
            _a = a;
        }
    }

    int toChars(char* buf){
        char* p = buf;
        p = std::to_chars(p, p+4, _r).ptr;  *p++ = ';';
        p = std::to_chars(p, p+4, _g).ptr;  *p++ = ';';
        p = std::to_chars(p, p+4, _b).ptr;
        return (int)(p - buf);
    }


    Color overlapColor(Color above){
        if(above._a == 0) return *this;
        uint8_t inv = 255 - above._a;
        uint8_t r = (uint8_t)((above._r * above._a + _r * inv) >> 8);
        uint8_t g = (uint8_t)((above._g * above._a + _g * inv) >> 8);
        uint8_t b = (uint8_t)((above._b * above._a + _b * inv) >> 8);
        return Color(r, g, b);
    }

    bool operator==(const Color& other) const {
        return _r == other._r &&
               _g == other._g &&
               _b == other._b &&
               _a == other._a;
    }

    Color & operator=(const Color & other) {
        _r = other._r;
        _g = other._g; 
        _b = other._b;
        _a = other._a;  
        return *this;
    }

    std::string toString() const {
        return std::to_string(_r)+','+std::to_string(_g)+','+std::to_string(_b)+','+std::to_string(_a);
    }

    static const Color white;
    static const Color black;
    static const Color red;
    static const Color green;
    static const Color blue;

    static const Color yellow;
    static const Color cyan;
    static const Color magenta;

    static const Color light_grey;
    static const Color grey;
    static const Color dark_grey;

    static const Color orange;
    static const Color purple;
    static const Color pink;

    static const Color brown;
    static const Color lime;
    static const Color navy;
    static const Color teal;
    static const Color olive;

    static const Color gold;
    static const Color silver;
    static const Color bronze;

    static const Color sky_blue;
    static const Color violet;
    static const Color indigo;
    static const Color turquoise;
};

inline const Color Color::white(255,255,255);
inline const Color Color::black(0,0,0);

inline const Color Color::red(255,0,0);
inline const Color Color::green(0,255,0);
inline const Color Color::blue(0,0,255);

inline const Color Color::yellow(255,255,0);
inline const Color Color::cyan(0,255,255);
inline const Color Color::magenta(255,0,255);

inline const Color Color::light_grey(200,200,200);
inline const Color Color::grey(128,128,128);
inline const Color Color::dark_grey(50,50,50);

inline const Color Color::orange(255,165,0);
inline const Color Color::purple(128,0,128);
inline const Color Color::pink(255,192,203);

inline const Color Color::brown(139,69,19);
inline const Color Color::lime(50,205,50);
inline const Color Color::navy(0,0,128);
inline const Color Color::teal(0,128,128);
inline const Color Color::olive(128,128,0);

inline const Color Color::gold(255,215,0);
inline const Color Color::silver(192,192,192);
inline const Color Color::bronze(205,127,50);

inline const Color Color::sky_blue(135,206,235);
inline const Color Color::violet(238,130,238);
inline const Color Color::indigo(75,0,130);
inline const Color Color::turquoise(64,224,208);



class Canvas {
private:
    size_t _width = 0;
    size_t _height = 0;
    std::vector<Color> _pixels;
    bool _imageMode = false;
    bool _initialized = false;
    std::string _title;
    std::vector<Color> _temp_pixels;
    std::vector<Color> _old_pixels;

    void requireInitialized() const {
        if (!_initialized)
            throw std::logic_error(
                "Canvas is not initialized. Call resize() before using it"
            );
    }

    void releaseAuxiliaryBuffers() noexcept {
        std::vector<Color>().swap(_temp_pixels);
        std::vector<Color>().swap(_old_pixels);
    }

    void prepareTempPixels() {
        if (_imageMode || _temp_pixels.size() != _pixels.size()) {
            _imageMode = false;
            _temp_pixels.assign(_pixels.size(), Color(0, 0, 0, 0));
        }
    }

    inline size_t index(size_t x, size_t y) const {
        return y * _width + x;
    }

    Color getPixelColor(size_t x, size_t y) const {
        requireInitialized();

        if (x >= _width || y >= _height)
            throw std::out_of_range("Error: Pixel out of range");

        return _pixels[index(x, y)];
    }

    Color getOldPixelColor(size_t x, size_t y) const {
        requireInitialized();

        if (x >= _width || y >= _height)
            throw std::out_of_range("Error: Pixel out of range");

        return _old_pixels[index(x, y)];
    }

    void setPixel(int x, int y, Color color) {
        if (x >= 0 && y >= 0 && x < (int)_width && y < (int)_height) {
            prepareTempPixels();
            _temp_pixels[index(x, y)] = color;
        }
    }

    static void writeAll(const char* data, size_t len) {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        size_t off = 0;

        while (off < len) {
            size_t remaining = len - off;
            DWORD chunk = (remaining > 0xFFFFFFFFu)
                        ? 0xFFFFFFFFu
                        : (DWORD)remaining;

            DWORD written = 0;
            if (!WriteFile(h, data + off, chunk, &written, nullptr))
                break;

            off += written;
        }
#else
        size_t off = 0;

        while (off < len) {
            ssize_t n = ::write(STDOUT_FILENO, data + off, len - off);

            if (n < 0) {
                if (errno == EINTR)
                    continue;

                break;
            }

            off += (size_t)n;
        }
#endif
    }

    static bool& terminalActive() noexcept {
        static bool active = false;
        return active;
    }

#ifdef _WIN32
    static DWORD& savedConsoleMode() noexcept {
        static DWORD mode = 0;
        return mode;
    }

    static bool& hasSavedConsoleMode() noexcept {
        static bool saved = false;
        return saved;
    }

    static UINT& savedOutputCodePage() noexcept {
        static UINT codePage = 0;
        return codePage;
    }

    static bool& hasSavedOutputCodePage() noexcept {
        static bool saved = false;
        return saved;
    }
#endif

    void compositeRegion(int x0, int y0, int x1, int y1) {
        if (_temp_pixels.size() != _pixels.size())
            return;

        x0 = std::max(0, x0);
        y0 = std::max(0, y0);
        x1 = std::min((int)_width, x1);
        y1 = std::min((int)_height, y1);

        if (x0 >= x1 || y0 >= y1)
            return;

        for (int y = y0; y < y1; y++) {
            size_t row = (size_t)y * _width;

            for (int x = x0; x < x1; x++) {
                size_t i = row + (size_t)x;
                _pixels[i] = _pixels[i].overlapColor(_temp_pixels[i]);
                _temp_pixels[i] = Color(0, 0, 0, 0);
            }
        }
    }

public:

    static void initializeTerminal() {
        if (terminalActive())
            return;

#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;

        if (GetConsoleMode(h, &mode)) {
            savedConsoleMode() = mode;
            hasSavedConsoleMode() = true;
            SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }

        UINT codePage = GetConsoleOutputCP();
        if (codePage != 0) {
            savedOutputCodePage() = codePage;
            hasSavedOutputCodePage() = true;
            SetConsoleOutputCP(CP_UTF8);
        }
#endif

        std::ios::sync_with_stdio(false);

        static constexpr char begin[] =
            "\033[?1049h"  
            "\033[2J\033[H"
            "\033[0m"       
            "\033[?25l";  

        writeAll(begin, sizeof(begin) - 1);
        terminalActive() = true;
    }

   
    static void resetTerminal() noexcept {
        if (!terminalActive())
            return;

        static constexpr char end[] =
            "\033[0m"      
            "\033[?25h"   
            "\033[?1049l"; 

        writeAll(end, sizeof(end) - 1);

#ifdef _WIN32
        if (hasSavedConsoleMode()) {
            HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleMode(h, savedConsoleMode());
            hasSavedConsoleMode() = false;
        }

        if (hasSavedOutputCodePage()) {
            SetConsoleOutputCP(savedOutputCodePage());
            hasSavedOutputCodePage() = false;
        }
#endif

        terminalActive() = false;
    }

    static bool isTerminalInitialized() noexcept {
        return terminalActive();
    }

    Canvas() = default;

    Canvas(size_t width, size_t height)
        : _width(width),
          _height(height),
          _pixels(width * height, CONSOLE_BG_COLOR),
          _imageMode(false),
          _initialized(true) {

        if (width == 0 || height == 0)
            throw std::invalid_argument("Error: screen size must be > 0");
    }

    Canvas(size_t width, size_t height, std::string title)
        : _width(width),
          _height(height),
          _pixels(width * height, CONSOLE_BG_COLOR),
          _imageMode(false),
          _initialized(true),
          _title(std::move(title)) {

        if (width == 0 || height == 0)
            throw std::invalid_argument("Error: screen size must be > 0");
    }

    Canvas(size_t width, size_t height, const std::vector<Color>& pixels)
        : _width(width),
          _height(height),
          _pixels(),
          _imageMode(true),
          _initialized(true) {

        if (width == 0 || height == 0 || width * height != pixels.size())
            throw std::invalid_argument("Error: Unable to initialize Canvas");

        _pixels = pixels;
    }

    Canvas(const Canvas& other)
        : _width(other._width),
          _height(other._height),
          _pixels(other._pixels),
          _imageMode(other._imageMode),
          _initialized(other._initialized),
          _title(other._title),
          _temp_pixels(other._temp_pixels),
          _old_pixels(other._old_pixels) { }

    Canvas(const std::string& file_name)
        : _width(0),
        _height(0),
        _pixels(),
        _imageMode(true),
        _initialized(false) {

        if (!load(file_name))
            throw std::runtime_error(
                "Error: Unable to load Canvas image: " + file_name
            );
    }
    
    bool load(const std::string& file_name) {
        std::ifstream fin(file_name, std::ios::binary);

        if (!fin)
            return false;

        uint32_t w;
        uint32_t h;

        if (!fin.read(reinterpret_cast<char*>(&w), sizeof(w)) ||
            !fin.read(reinterpret_cast<char*>(&h), sizeof(h))) {
            return false;
        }

        if (w == 0 || h == 0)
            return false;

        const size_t width = (size_t)w;
        const size_t height = (size_t)h;
        const size_t maxPixels = std::vector<Color>().max_size();

        if (width > maxPixels / height)
            return false;

        const size_t pixelCount = width * height;
        std::vector<Color> loadedPixels;
        loadedPixels.reserve(pixelCount);

        for (size_t i = 0; i < pixelCount; i++) {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;

            if (!fin.read(reinterpret_cast<char*>(&r), sizeof(r)) ||
                !fin.read(reinterpret_cast<char*>(&g), sizeof(g)) ||
                !fin.read(reinterpret_cast<char*>(&b), sizeof(b)) ||
                !fin.read(reinterpret_cast<char*>(&a), sizeof(a))) {
                return false;
            }

            loadedPixels.emplace_back(r, g, b, a);
        }

        _width = width;
        _height = height;
        _pixels.swap(loadedPixels);
        _initialized = true;

        setImageMode(true);

        return true;
    }

    size_t getHeight() const {
        return _height;
    }

    size_t getWidth() const {
        return _width;
    }

    bool isInitialized() const noexcept {
        return _initialized;
    }

    void setImageMode(bool imageMode) {
        requireInitialized();

        _imageMode = imageMode;

        if (_imageMode)
            releaseAuxiliaryBuffers();
    }

    bool isImage() const {
        return _imageMode;
    }

    Canvas scaled(size_t w, size_t h) const {
        requireInitialized();

        if (w == 0 || h == 0)
            throw std::invalid_argument("Error: scaled Canvas size must be > 0");

        size_t dw = (w > 1) ? w - 1 : 1;
        size_t dh = (h > 1) ? h - 1 : 1;
        std::vector<Color> outPixels(w * h);

        for (size_t y = 0; y < h; y++) {
            size_t sy = y * (_height - 1) / dh;
            size_t rowBase = sy * _width;

            for (size_t x = 0; x < w; x++) {
                size_t sx = x * (_width - 1) / dw;
                outPixels[y * w + x] = _pixels[rowBase + sx];
            }
        }

        return Canvas(w, h, outPixels);
    }

#ifdef _WIN32
    static void setTitle(const std::string& title) {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, NULL, 0);
        std::wstring wtitle(size_needed, 0);

        MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wtitle[0], size_needed);
        SetConsoleTitleW(wtitle.c_str());
    }
#else
    static void setTitle(const std::string& title) {
        std::string s = "\033]0;" + title + "\007";
        writeAll(s.data(), s.size());
    }
#endif

    static std::pair<int, int> getScreenSize() {
        int width;
        int height;

#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        width = w.ws_col;
        height = w.ws_row;
#endif

        return { width, height*2 };
    }

    bool resize(size_t width, size_t height) {
        if (width == 0 || height == 0)
            throw std::invalid_argument("Error: screen size must be > 0");

        if (_initialized && _width == width && _height == height)
            return false;

        _width = width;
        _height = height;
        _pixels.assign(width * height, CONSOLE_BG_COLOR);
        _initialized = true;

        if (_temp_pixels.empty()) {
            std::vector<Color>().swap(_temp_pixels);
        } else {
            _temp_pixels.assign(width * height, Color(0, 0, 0, 0));
        }

        std::vector<Color>().swap(_old_pixels);

        if (terminalActive()) {
            const std::string clear = "\033[3J\033[1;1H";
            writeAll(clear.data(), clear.size());
        }

        return true;
    }

    void print() {
        requireInitialized();

        if (!terminalActive())
            throw std::logic_error(
                "Call Canvas::initializeTerminal() before print()"
            );

        if (!_title.empty())
            setTitle(_title);

        _imageMode = false;

        const bool hasOldPixels = _old_pixels.size() == _pixels.size();

        std::string buffer;
        buffer.reserve(_width * (_height / 2) * 40);

        bool need_reposition = true;
        Color curFG, curBG;
        bool haveColor = false;
        char tmp[12];

        for (size_t y = 0; y < _height; y += 2) {
            for (size_t x = 0; x < _width; x++) {
                size_t i = y * _width + x;
                Color top = _pixels[i];
                Color bottom = (y + 1 < _height)
                    ? _pixels[i + _width]
                    : CONSOLE_BG_COLOR;

                if (hasOldPixels) {
                    Color old_top = _old_pixels[i];
                    Color old_bottom = (y + 1 < _height)
                        ? _old_pixels[i + _width]
                        : CONSOLE_BG_COLOR;

                    if (top == old_top && bottom == old_bottom) {
                        need_reposition = true;
                        continue;
                    }
                }

                if (need_reposition) {
                    buffer += "\033[";
                    buffer += std::to_string(y / 2 + 1);
                    buffer += ';';
                    buffer += std::to_string(x + 1);
                    buffer += 'H';
                    need_reposition = false;
                }

                bool fgChanged = !haveColor || !(top == curFG);
                bool bgChanged = !haveColor || !(bottom == curBG);

                if (fgChanged && bgChanged) {
                    buffer += "\033[38;2;";
                    buffer.append(tmp, top.toChars(tmp));
                    buffer += ";48;2;";
                    buffer.append(tmp, bottom.toChars(tmp));
                    buffer += 'm';
                } else if (fgChanged) {
                    buffer += "\033[38;2;";
                    buffer.append(tmp, top.toChars(tmp));
                    buffer += 'm';
                } else if (bgChanged) {
                    buffer += "\033[48;2;";
                    buffer.append(tmp, bottom.toChars(tmp));
                    buffer += 'm';
                }

                curFG = top;
                curBG = bottom;
                haveColor = true;
                buffer += "▀";
            }

            need_reposition = true;
        }

        if (!buffer.empty())
            writeAll(buffer.data(), buffer.size());

        _old_pixels = _pixels;
    }

    void setBG(Color c) {
        requireInitialized();

        _pixels.assign(_pixels.size(), c);
    }

    void clear() noexcept {
        std::vector<Color>().swap(_pixels);
        releaseAuxiliaryBuffers();

        _width = 0;
        _height = 0;
        _initialized = false;
        _imageMode = false;
        _title.clear();
    }

    void drawPixel(int x, int y, Color c) {
        requireInitialized();

        if (x >= 0 && y >= 0 && x < (int)_width && y < (int)_height)
            _pixels[index(x, y)] = _pixels[index(x, y)].overlapColor(c);
    }

    void drawLine(int x0, int y0, int x1, int y1, int thickness, Color c) {
        requireInitialized();

        if (thickness <= 0)
            return;

        float half = thickness / 2.0f;
        int minX = std::max(0, std::min(x0, x1) - thickness);
        int maxX = std::min((int)_width - 1, std::max(x0, x1) + thickness);
        int minY = std::max(0, std::min(y0, y1) - thickness);
        int maxY = std::min((int)_height - 1, std::max(y0, y1) + thickness);

        if (minX > maxX || minY > maxY)
            return;

        float ldx = (float)(x1 - x0);
        float ldy = (float)(y1 - y0);
        float lenSq = ldx * ldx + ldy * ldy;

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                float dist;

                if (lenSq == 0.0f) {
                    float ex = x - x0;
                    float ey = y - y0;
                    dist = std::sqrt(ex * ex + ey * ey);
                } else {
                    float t = ((x - x0) * ldx + (y - y0) * ldy) / lenSq;
                    t = std::clamp(t, 0.0f, 1.0f);

                    float ex = x - (x0 + t * ldx);
                    float ey = y - (y0 + t * ldy);
                    dist = std::sqrt(ex * ex + ey * ey);
                }

                if (dist <= half)
                    setPixel(x, y, c);
            }
        }

        compositeRegion(minX, minY, maxX + 1, maxY + 1);
    }

    Canvas& operator=(const Canvas& other) {
        if (this == &other)
            return *this;

        _pixels = other._pixels;
        _height = other._height;
        _width = other._width;
        _imageMode = other._imageMode;
        _initialized = other._initialized;
        _title = other._title;
        _temp_pixels = other._temp_pixels;
        _old_pixels = other._old_pixels;

        return *this;
    }

    bool operator==(const Canvas& other) const {
        return _pixels == other._pixels &&
               _height == other._height &&
               _width == other._width;
    }

    void drawFilledCircle(int cx, int cy, int r, Color c) {
        requireInitialized();

        if (r < 0)
            return;

        int x = 0;
        int y = r;
        int d = 1 - r;

        while (y >= x) {
            for (int i = cx - x; i <= cx + x; i++) {
                setPixel(i, cy + y, c);
                setPixel(i, cy - y, c);
            }

            if (x != y) {
                for (int i = cx - y; i <= cx + y; i++) {
                    setPixel(i, cy + x, c);
                    setPixel(i, cy - x, c);
                }
            }

            x++;

            if (d < 0) {
                d += 2 * x + 1;
            } else {
                y--;
                d += 2 * (x - y) + 1;
            }
        }

        compositeRegion(cx - r, cy - r, cx + r + 1, cy + r + 1);
    }

    void drawCircle(int cx, int cy, int r, Color c) {
        requireInitialized();

        if (r < 0)
            return;

        int x = 0;
        int y = r;
        int d = 1 - r;

        while (x <= y) {
            setPixel(cx + x, cy + y, c);
            setPixel(cx - x, cy + y, c);
            setPixel(cx + x, cy - y, c);
            setPixel(cx - x, cy - y, c);
            setPixel(cx + y, cy + x, c);
            setPixel(cx - y, cy + x, c);
            setPixel(cx + y, cy - x, c);
            setPixel(cx - y, cy - x, c);

            x++;

            if (d < 0) {
                d += 2 * x + 1;
            } else {
                y--;
                d += 2 * (x - y) + 1;
            }
        }

        compositeRegion(cx - r, cy - r, cx + r + 1, cy + r + 1);
    }

    void drawFilledRect(int x, int y, int w, int h, Color c) {
        requireInitialized();

        if (w <= 0 || h <= 0)
            return;

        int x0 = std::max(0, x);
        int y0 = std::max(0, y);
        int x1 = std::min((int)_width, x + w);
        int y1 = std::min((int)_height, y + h);

        for (int py = y0; py < y1; py++) {
            for (int px = x0; px < x1; px++)
                setPixel(px, py, c);
        }

        compositeRegion(x0, y0, x1, y1);
    }

    void drawRect(int x, int y, int w, int h, int thickness, Color c) {
        requireInitialized();

        if (w == 0 || h == 0 || thickness <= 0)
            return;

        if (w < 0) {
            x += w;
            w = -w;
        }

        if (h < 0) {
            y += h;
            h = -h;
        }

        for (int t = 0; t < thickness; t++) {
            for (int i = x; i < x + w; i++) {
                setPixel(i, y + t, c);
                setPixel(i, y + h - 1 - t, c);
            }

            for (int i = y; i < y + h; i++) {
                setPixel(x + t, i, c);
                setPixel(x + w - 1 - t, i, c);
            }
        }

        compositeRegion(x, y, x + w, y + h);
    }

    bool drawImg(const Canvas& img, int x, int y) {
        requireInitialized();
        img.requireInitialized();

        const int W = (int)_width;
        const int H = (int)_height;
        const int iw = (int)img.getWidth();
        const int ih = (int)img.getHeight();
        const Color* src = img._pixels.data();

        for (int ry = 0; ry < ih; ry++) {
            int py = y + ry;

            if (py < 0 || py >= H)
                continue;

            size_t dstRow = (size_t)py * _width;
            size_t srcRow = (size_t)ry * iw;

            for (int rx = 0; rx < iw; rx++) {
                int px = x + rx;

                if (px < 0 || px >= W)
                    continue;

                size_t di = dstRow + (size_t)px;
                _pixels[di] = _pixels[di].overlapColor(src[srcRow + (size_t)rx]);
            }
        }

        return true;
    }

    void drawText(const std::wstring& text, int x, int y, Color c);

    bool drawTexture(const Canvas& img, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
        requireInitialized();
        img.requireInitialized();

        auto sample = [&](float u, float v) -> Color {
            u = std::clamp(u, 0.0f, 1.0f);
            v = std::clamp(v, 0.0f, 1.0f);

            size_t tx = (size_t)(u * (img.getWidth() - 1));
            size_t ty = (size_t)(v * (img.getHeight() - 1));

            return img.getPixelColor(tx, ty);
        };

        auto rasterTri = [&](float ax, float ay, float au, float av, float bx, float by, float bu, float bv, float cx, float cy, float cu, float cv) {
            int minX = std::max(0, (int)std::floor(std::min({ ax, bx, cx })));
            int maxX = std::min((int)_width - 1, (int)std::ceil(std::max({ ax, bx, cx })));
            int minY = std::max(0, (int)std::floor(std::min({ ay, by, cy })));
            int maxY = std::min((int)_height - 1, (int)std::ceil(std::max({ ay, by, cy })));

            float denom = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy);

            if (std::abs(denom) < 1e-6f)
                return;

            for (int py = minY; py <= maxY; py++) {
                for (int px = minX; px <= maxX; px++) {
                    float w0 = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / denom;
                    float w1 = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / denom;
                    float w2 = 1.0f - w0 - w1;

                    if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                        float u = w0 * au + w1 * bu + w2 * cu;
                        float v = w0 * av + w1 * bv + w2 * cv;
                        setPixel(px, py, sample(u, v));
                    }
                }
            }
        };

        rasterTri((float)x0, (float)y0, 0.0f, 0.0f, (float)x1, (float)y1, 1.0f, 0.0f, (float)x2, (float)y2, 1.0f, 1.0f);
        rasterTri((float)x0, (float)y0, 0.0f, 0.0f, (float)x2, (float)y2, 1.0f, 1.0f, (float)x3, (float)y3, 0.0f, 1.0f);

        int bx0 = std::min(std::min(x0, x1), std::min(x2, x3));
        int bx1 = std::max(std::max(x0, x1), std::max(x2, x3));
        int by0 = std::min(std::min(y0, y1), std::min(y2, y3));
        int by1 = std::max(std::max(y0, y1), std::max(y2, y3));

        compositeRegion(bx0, by0, bx1 + 1, by1 + 1);

        return true;
    }

    static void sleep(int s) {
        std::this_thread::sleep_for(std::chrono::milliseconds(s));
    }

    Canvas screenshot() const {
        requireInitialized();

        return Canvas(_width, _height, _pixels);
    }

    ~Canvas() = default;

};

typedef Canvas CImg;

class Font{
    inline static Color t = Color(0,0,0,0);

    static CImg basic_a(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, c, c, c, t, t,
            t, t, t, t, c, t,
            t, c, c, c, c, t,
            c, t, t, t, c, t,
            c, t, t, c, c, t,
            t, c, c, t, c, t
        });
    }

    static CImg basic_a_grave(Color c){
        return CImg(6,8,{
            c, c, t, t, t, t,
            t, t, t, t, t, t,
            c, c, c, c, t, t,
            t, t, t, t, c, t,
            t, c, c, c, c, t,
            c, t, t, t, c, t,
            c, t, t, c, c, t,
            t, c, c, t, c, t
        });
    }

    static CImg basic_b(Color c){
        return CImg(6,8,{
            c, t, t, t, t, t,
            c, t, t, t, t, t,
            c, t, t, t, t, t, 
            c, t, c, c, t, t, 
            c, c, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_c(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_d(Color c){
        return CImg(6,8,{
            t, t, t, t, c, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t, 
            t, c, c, t, c, t, 
            c, t, t, c, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_e(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, c, c, c, c, t, 
            c, t, t, t, t, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_e_grave(Color c){
        return CImg(6,8,{
            c, c, t, t, t, t,
            t, t, t, t, t, t,
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, c, c, c, c, t, 
            c, t, t, t, t, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_e_acute(Color c){
        return CImg(6,8,{
            t, t, t, c, c, t,
            t, t, t, t, t, t,
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, c, c, c, c, t, 
            c, t, t, t, t, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_f(Color c){
        return CImg(5,8,{
            t, t, t, t, t, 
            t, t, c, c, t, 
            t, c, t, t, t,  
            c, c, c, c, t,  
            t, c, t, t, t,  
            t, c, t, t, t,  
            t, c, t, t, t,  
            t, c, t, t, t 
        });
    }

    static CImg basic_g(Color c){
        return CImg(6,9,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, c, c, c, c, t,
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            t, c, c, c, c, t, 
            t, t, t, t, c, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_h(Color c){
        return CImg(6,8,{
            c, t, t, t, t, t,
            c, t, t, t, t, t,
            c, t, t, t, t, t, 
            c, t, c, c, t, t, 
            c, c, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t
        });
    }

    static CImg basic_i(Color c){
        return CImg(2,8,{
            t, t,
            t, t,
            c, t,
            t, t, 
            c, t, 
            c, t, 
            c, t, 
            c, t
        });
    }

    static CImg basic_i_grave(Color c){
        return CImg(3,8,{
            c, c, t,
            t, t, t,
            t, c, t,
            t, t, t, 
            t, c, t, 
            t, c, t, 
            t, c, t, 
            t, c, t
        });
    }

    static CImg basic_j(Color c){
        return CImg(5,9,{
            t, t, t, t, t,
            t, t, t, t, t,
            t, t, t, c, t,
            t, t, t, t, t,
            t, t, t, c, t,
            t, t, t, c, t,
            t, t, t, c, t,
            c, t, t, c, t,
            t, c, c, t, t
        });
    }

    static CImg basic_k(Color c){
        return CImg(5,8,{
            t, t, t, t, t,
            c, t, t, t, t,
            c, t, t, t, t,
            c, t, t, c, t,
            c, t, c, t, t,
            c, c, t, t, t,
            c, t, c, t, t,
            c, t, t, c, t
        });
    }

    static CImg basic_l(Color c){
        return CImg(2,8,{
            c, t,
            c, t,
            c, t,
            c, t,
            c, t,
            c, t,
            c, t,
            c, t
        });
    }

    static CImg basic_m(Color c){
        return CImg(8,8,{
            t, t, t, t, t, t, t, t,
            t, t, t, t, t, t, t, t,
            c, c, c, t, c, c, t, t,
            c, t, t, c, t, t, c, t,
            c, t, t, c, t, t, c, t,
            c, t, t, c, t, t, c, t,
            c, t, t, c, t, t, c, t,
            c, t, t, c, t, t, c, t
        });
    }

    static CImg basic_n(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, t, c, c, t, t,
            c, c, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t
        });
    }

    static CImg basic_o(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, c, c, c, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }

    static CImg basic_o_grave(Color c){
        return CImg(6,8,{
            c, c, t, t, t, t,
            t, t, t, t, t, t,
            t, c, c, c, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }


    static CImg basic_p(Color c){
        return CImg(6,10,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, t, c, c, t, t,
            c, c, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, c, c, c, t, t,
            c, t, t, t, t, t,
            c, t, t, t, t, t,
            c, t, t, t, t, t
        });
    }

    static CImg basic_q(Color c){
        return CImg(6,10,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, c, c, t, c, t,
            c, t, t, c, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, c, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t
        });
    }

    static CImg basic_r(Color c){
        return CImg(5,8,{
            t, t, t, t, t,
            t, t, t, t, t,
            c, t, c, c, t,
            c, c, t, t, t,
            c, t, t, t, t,
            c, t, t, t, t,
            c, t, t, t, t,
            c, t, t, t, t
        });
    }

    static CImg basic_s(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, c, c, c, c, t,
            c, t, t, t, t, t,
            t, c, c, c, t, t,
            t, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }

    static CImg basic_t(Color c){
        return CImg(4,8,{
            t, t, t, t,
            t, c, t, t,
            t, c, t, t,
            c, c, c, t,
            t, c, t, t,
            t, c, t, t,
            t, c, t, t,
            t, t, c, t
        });
    }

    static CImg basic_u(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }

    static CImg basic_u_grave(Color c){
        return CImg(6,8,{
            c, c, t, t, t, t,
            t, t, t, t, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }

    static CImg basic_v(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, t, c, t, t,
            t, c, t, c, t, t,
            t, t, c, t, t, t
        });
    }

    static CImg basic_w(Color c){
        return CImg(8,8,{
            t, t, t, t, t, t, t, t,
            t, t, t, t, t, t, t, t,
            c, t, t, t, t, t, c, t,
            c, t, t, t, t, t, c, t,
            t, c, t, c, t, c, t, t,
            t, c, t, c, t, c, t, t,
            t, t, c, t, c, t, t, t,
            t, t, c, t, c, t, t, t
        });
    }

    static CImg basic_x(Color c){
        return CImg(6,8,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, t, t, t, c, t,
            t, c, t, c, t, t,
            t, t, c, t, t, t,
            t, c, t, c, t, t,
            c, t, t, t, c, t
        });
    }

    static CImg basic_y(Color c){
        return CImg(5,10,{
            t, t, t, t, t,
            t, t, t, t, t,
            c, t, t, c, t,
            c, t, t, c, t,
            c, t, t, c, t,
            c, t, t, c, t,
            t, c, c, c, t,
            t, t, t, c, t,
            t, t, t, c, t,
            c, c, c, t, t
        });
    }

    static CImg basic_z(Color c){
        return CImg(5,8,{
            t, t, t, t, t,
            t, t, t, t, t,
            c, c, c, c, t,
            t, t, t, c, t,
            t, t, c, t, t,
            t, t, c, t, t,
            t, c, t, t, t,
            c, c, c, c, t
        });
    }


    static CImg basic_A(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, c, c, c, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t
        });
    }

    static CImg basic_B(Color c){
        return CImg(6,8,{
            c, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, c, c, c, t, t
        });
    }

    static CImg basic_C(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_D(Color c){
        return CImg(6,8,{
            c, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, c, c, c, t, t
        });
    }

    static CImg basic_E(Color c){
        return CImg(6,8,{
            c, c, c, c, c, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, c, c, c, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, c, c, c, c, t
        });
    }

    static CImg basic_F(Color c){
        return CImg(6,8,{
            c, c, c, c, c, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, c, c, c, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t
        });
    }

    static CImg basic_G(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, t, t, 
            c, t, c, c, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            t, c, c, c, c, t
        });
    }

    static CImg basic_H(Color c){
        return CImg(6,8,{
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, c, c, c, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t
        });
    }

    static CImg basic_I(Color c){
        return CImg(4,8,{
            c, c, c, t, 
            t, c, t, t, 
            t, c, t, t, 
            t, c, t, t, 
            t, c, t, t, 
            t, c, t, t, 
            t, c, t, t, 
            c, c, c, t
        });
    }

    static CImg basic_J(Color c){
        return CImg(6,8,{
            t, t, c, c, c, t, 
            t, t, t, c, t, t, 
            t, t, t, c, t, t, 
            t, t, t, c, t, t, 
            c, t, t, c, t, t, 
            c, t, t, c, t, t, 
            c, t, t, c, t, t, 
            t, c, c, t, t, t
        });
    }

    static CImg basic_K(Color c){
        return CImg(5,8,{
            c, t, t, c, t, 
            c, t, t, c, t, 
            c, t, c, t, t, 
            c, c, t, t, t, 
            c, t, c, t, t, 
            c, t, c, t, t, 
            c, t, t, c, t, 
            c, t, t, c, t
        });
    }

    static CImg basic_L(Color c){
        return CImg(4,8,{
            c, t, t, t, 
            c, t, t, t, 
            c, t, t, t, 
            c, t, t, t, 
            c, t, t, t, 
            c, t, t, t, 
            c, t, t, t, 
            c, c, c, t
        });
    }

    static CImg basic_M(Color c){
        return CImg(8,8,{
            c, t, t, t, t, t, c, t,
            c, c, t, t, t, c, c, t,
            c, t, c, t, c, t, c, t,
            c, t, c, t, c, t, c, t,
            c, t, t, c, t, t, c, t,
            c, t, t, c, t, t, c, t,
            c, t, t, t, t, t, c, t,
            c, t, t, t, t, t, c, t
        });
    }

    static CImg basic_N(Color c){
        return CImg(7,8,{
            c, t, t, t, t, c, t,
            c, c, t, t, t, c, t,
            c, t, c, t, t, c, t,
            c, t, c, t, t, c, t,
            c, t, t, c, t, c, t,
            c, t, t, c, t, c, t,
            c, t, t, t, c, c, t,
            c, t, t, t, t, c, t
        });
    }

    static CImg basic_O(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_P(Color c){
        return CImg(6,8,{
            c, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, c, c, c, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t, 
            c, t, t, t, t, t
        });
    }

    static CImg basic_Q(Color c){
        return CImg(7,8,{
            t, c, c, c, t, t, t, 
            c, t, t, t, c, t, t, 
            c, t, t, t, c, t, t, 
            c, t, t, t, c, t, t, 
            c, t, t, t, c, t, t, 
            c, t, t, c, c, t, t, 
            c, t, t, t, c, t, t, 
            t, c, c, c, t, c, t
        });
    }

    static CImg basic_R(Color c){
        return CImg(6,8,{
            c, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, c, c, c, t, t, 
            c, t, c, t, t, t, 
            c, t, t, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t
        });
    }

    static CImg basic_S(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t, 
            c, t, t, t, c, t, 
            c, t, t, t, t, t, 
            t, c, c, c, t, t, 
            t, t, t, t, c, t, 
            t, t, t, t, c, t, 
            c, t, t, t, c, t, 
            t, c, c, c, t, t
        });
    }

    static CImg basic_T(Color c){
        return CImg(6,8,{
            c, c, c, c, c, t, 
            t, t, c, t, t, t, 
            t, t, c, t, t, t, 
            t, t, c, t, t, t, 
            t, t, c, t, t, t, 
            t, t, c, t, t, t, 
            t, t, c, t, t, t, 
            t, t, c, t, t, t
        });
    }

    static CImg basic_U(Color c){
        return CImg(6,8,{
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }

    static CImg basic_V(Color c){
        return CImg(6,8,{
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            c, t, t, t, c, t, 
            t, c, t, c, t, t, 
            t, c, t, c, t, t, 
            t, c, t, c, t, t, 
            t, t, c, t, t, t
        });
    }

    static CImg basic_W(Color c){
        return CImg(10,8,{
            c, t, t, t, t, t, t, t, c, t,
            c, t, t, t, t, t, t, t, c, t,
            c, t, t, t, c, t, t, t, c, t,
            t, c, t, t, c, t, t, c, t, t,
            t, c, t, c, t, c, t, c, t, t,
            t, c, t, c, t, c, t, c, t, t,
            t, t, c, t, t, t, c, t, t, t,
            t, t, c, t, t, t, c, t, t, t
        });
    }

    static CImg basic_X(Color c){
        return CImg(6,8,{
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, t, c, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, c, t, c, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t
        });
    }

    static CImg basic_Y(Color c){
        return CImg(6,8,{
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, t, c, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t
        });
    }

    static CImg basic_Z(Color c){
        return CImg(7,8,{
            c, c, c, c, c, c, t,
            t, t, t, t, t, c, t,
            t, t, t, t, c, t, t,
            t, t, t, c, t, t, t,
            t, t, c, t, t, t, t,
            t, c, t, t, t, t, t,
            c, t, t, t, t, t, t,
            c, c, c, c, c, c, t
        });
    }



    static CImg basic_0(Color c){
        return CImg(6,8,{
            t,c,c,c,t,t,
            c,t,t,t,c,t,
            c,t,t,c,c,t,
            c,t,c,t,c,t,
            c,c,t,t,c,t,
            c,t,t,t,c,t,
            c,t,t,t,c,t,
            t,c,c,c,t,t
        });
    }

    static CImg basic_1(Color c){
        return CImg(6,8,{
            t, t, c, t, t, t,
            t, c, c, t, t, t,
            c, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            c, c, c, c, c, t
        });
    }

    static CImg basic_2(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t,
            c, t, t, t, c, t,
            t, t, t, t, c, t,
            t, t, t, c, t, t,
            t, t, c, t, t, t,
            t, c, t, t, t, t,
            c, t, t, t, t, t,
            c, c, c, c, c, t
        });
    }

    static CImg basic_3(Color c){
        return CImg(6,8,{
            c, c, c, c, t, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t,
            t, c, c, c, t, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t,
            c, c, c, c, t, t
        });
    }

    static CImg basic_4(Color c){
        return CImg(6,8,{
            t, t, t, c, t, t,
            t, t, c, c, t, t,
            t, c, t, c, t, t,
            c, t, t, c, t, t,
            c, c, c, c, c, t,
            t, t, t, c, t, t,
            t, t, t, c, t, t,
            t, t, t, c, t, t
        });
    }

    static CImg basic_5(Color c){
        return CImg(6,8,{
            c, c, c, c, c, t,
            c, t, t, t, t, t,
            c, t, t, t, t, t,
            c, c, c, c, t, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t,
            c, c, c, c, t, t
        });
    }

    static CImg basic_6(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t,
            c, t, t, t, c, t,
            c, t, t, t, t, t,
            c, t, c, c, t, t,
            c, c, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }

    static CImg basic_7(Color c){
        return CImg(6,8,{
            c, c, c, c, c, t,
            t, t, t, t, c, t,
            t, t, t, c, t, t,
            t, t, t, c, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t
        });
    }

    static CImg basic_8(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }

    static CImg basic_9(Color c){
        return CImg(6,8,{
            t, c, c, c, t, t,
            c, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, c, t,
            t, t, t, t, c, t,
            t, t, t, t, c, t,
            c, t, t, t, c, t,
            t, c, c, c, t, t
        });
    }


    static CImg basic_space(){
        return CImg(4,1,{
            t,  t,  t,  t
        });
    }

    static CImg basic_period(Color c){
        return CImg(2,9,{
            t, t,
            t, t,
            t, t,
            t, t,
            t, t,
            t, t,
            t, t,
            t, t,
            c, t
        });
    }

    static CImg basic_comma(Color c){
        return CImg(2,9,{
            t, t,
            t, t,
            t, t,
            t, t,
            t, t,
            t, t,
            t, t,
            c, t,
            c, t
        });
    }

    static CImg basic_exclamation_mark(Color c){
        return CImg(2,9,{
            t, t, 
            c, t, 
            c, t, 
            c, t, 
            c, t, 
            c, t, 
            c, t, 
            t, t, 
            c, t
        });
    }

    static CImg basic_question_mark(Color c){
        return CImg(6,9,{
            t, t, t, t, t, t,
            t, c, c, c, t, t,
            c, t, t, t, c, t,
            t, t, t, t, c, t,
            t, t, t, c, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            t, t, t, t, t, t,
            t, t, c, t, t, t
        });
    }

    static CImg basic_colon(Color c){
        return CImg(2,8,{
            t, t,
            t, t,
            c, t,
            c, t,
            t, t,
            t, t,
            c, t,
            c, t
        });
    }

    static CImg basic_semicolon(Color c){
        return CImg(2,9,{
            t, t, 
            t, t, 
            c, t, 
            c, t, 
            t, t, 
            t, t, 
            c, t, 
            c, t, 
            c, t
        });
    }

    static CImg basic_apostrophe(Color c){
        return CImg(3,3,{
            c, t, t,
            c, t, t,
            t, c, t
        });
    }

    static CImg basic_quotation_marks(Color c){
        return CImg(4,2,{
            c, t, c, t,
            c, t, c, t,
        });
    }

    static CImg basic_minus_sign(Color c){
        return CImg(6,5,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, c, c, c, c, t

        });
    }

    static CImg basic_plus_sign(Color c){
        return CImg(6,7,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            c, c, c, c, c, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t

        });
    }

    static CImg basic_asterisk(Color c){
        return CImg(4,3,{
            c, t, c, t,
            t, c, t, t,
            c, t, c, t
        });
    }

    static CImg basic_slash(Color c){
        return CImg(5,8,{
            t, t, t, c, t, 
            t, t, t, c, t, 
            t, t, c, t, t, 
            t, t, c, t, t, 
            t, c, t, t, t, 
            t, c, t, t, t, 
            c, t, t, t, t, 
            c, t, t, t, t 
        });
    }

    static CImg basic_backslash(Color c){
        return CImg(5,8,{
            c, t, t, t, t, 
            c, t, t, t, t, 
            t, c, t, t, t, 
            t, c, t, t, t, 
            t, t, c, t, t, 
            t, t, c, t, t, 
            t, t, t, c, t, 
            t, t, t, c, t 
        });
    }

    static CImg basic_pipe(Color c){
        return CImg(2,8,{
            c, t,
            c, t,
            c, t,
            c, t,
            c, t,
            c, t,
            c, t,
            c, t
        });
    }

    static CImg basic_left_parenthesis(Color c){
        return CImg(4,8,{
            t, c, c, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, t, t, t,  
            t, c, c, t
        });
    }

    static CImg basic_right_parenthesis(Color c){
        return CImg(4,8,{
            c, c, t, t, 
            t, t, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            c, c, t, t
        });
    }

    static CImg basic_left_square_bracket(Color c){
        return CImg(4,8,{
            c, c, c, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, t, t, t,  
            c, c, c, t
        });
    }

    static CImg basic_right_square_bracket(Color c){
        return CImg(4,8,{
            c, c, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            t, t, c, t, 
            c, c, c, t
        });
    }

    static CImg basic_left_curly_brace(Color c){
        return CImg(3,8,{
            t, c, t,  
            c, t, t,  
            c, t, t,  
            t, c, t,  
            t, c, t,  
            c, t, t,  
            c, t, t,  
            t, c, t
        });
    }

    static CImg basic_right_curly_brace(Color c){
        return CImg(3,8,{
            c, t, t, 
            t, c, t, 
            t, c, t, 
            c, t, t, 
            c, t, t, 
            t, c, t, 
            t, c, t, 
            c, t, t
        });
    }

    static CImg basic_equals_sign(Color c){
        return CImg(5,6,{
            t, t, t, t, t, 
            t, t, t, t, t, 
            c, c, c, c, t, 
            t, t, t, t, t, 
            t, t, t, t, t, 
            c, c, c, c, t
        });
    }

    static CImg basic_ampersand(Color c){ 
        return CImg(5,8,{
            t, t, t, t, t, 
            t, t, c, t, t, 
            t, c, t, c, t, 
            t, t, c, t, t, 
            t, c, c, t, t, 
            c, t, t, c, t, 
            c, t, t, c, c, 
            t, c, c, t, t 
        });
    }

    static CImg basic_percent_sign(Color c){ 
        return CImg(7,8,{
            t, c, t, t, c, t, t,
            c, t, c, t, c, t, t, 
            t, c, t, c, t, t, t, 
            t, t, t, c, t, t, t, 
            t, t, c, t, t, t, t, 
            t, t, c, t, c, t, t, 
            t, c, t, c, t, c, t, 
            t, c, t, t, c, t, t
        });
    }

    static CImg basic_pound_sign(Color c){ 
        return CImg(6,9,{
            t, t, t, c, t, t, 
            t, t, c, t, t, t, 
            t, t, c, t, t, t, 
            t, c, c, c, t, t, 
            t, t, c, t, t, t, 
            t, t, c, t, t, t, 
            t, c, t, t, t, t, 
            c, t, t, t, t, t, 
            c, c, c, c, c, t
        });
    }

    static CImg basic_dollar_sign(Color c){ 
        return CImg(6,9,{
            t, t, c, t, t, t, 
            t, c, c, c, t, t, 
            c, t, c, t, c, t, 
            c, t, c, t, t, t, 
            t, c, c, c, t, t, 
            t, t, c, t, c, t, 
            c, t, c, t, c, t, 
            t, c, c, c, t, t,
            t, t, c, t, t, t
        });
    }

    static CImg basic_euro_sign(Color c){ 
        return CImg(6,9,{
            t, t, c, c, c, t, 
            t, c, t, t, t, t, 
            t, c, t, t, t, t, 
            c, c, c, c, c, t, 
            t, c, t, t, t, t, 
            c, c, c, c, c, t, 
            t, c, t, t, t, t, 
            t, c, t, t, t, t,
            t, t, c, c, c, t
        });
    }

    static CImg basic_underscore(Color c){ 
        return CImg(6,9,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, c, c, c, c, t
        });
    }

    static CImg basic_hash(Color c){ 
        return CImg(6,8,{
            t, c, t, c, t, t,
            t, c, t, c, t, t,
            c, c, c, c, c, t,
            t, c, t, c, t, t,
            t, c, t, c, t, t,
            c, c, c, c, c, t,
            t, c, t, c, t, t,
            t, c, t, c, t, t
        });
    }

    static CImg basic_at_sign(Color c){ 
        return CImg(7,8,{
            t, c, c, c, c, t, t,
            c, t, t, t, t, c, t,
            c, t, c, c, t, c, t,
            c, t, c, t, t, c, t,
            c, t, c, t, t, c, t,
            c, t, c, c, c, t, t,
            c, t, t, t, t, t, t,
            t, c, c, c, c, t, t
        });
    }


    static CImg basic_greater_sign(Color c){ 
        return CImg(5,8,{
            t, t, t, t, t, 
            c, t, t, t, t, 
            t, c, t, t, t, 
            t, t, c, t, t, 
            t, t, t, c, t, 
            t, t, c, t, t, 
            t, c, t, t, t, 
            c, t, t, t, t
        });
    }

    static CImg basic_less_sign(Color c){ 
        return CImg(5,8,{
            t, t, t, t, t, 
            t, t, t, c, t, 
            t, t, c, t, t, 
            t, c, t, t, t, 
            c, t, t, t, t, 
            t, c, t, t, t, 
            t, t, c, t, t, 
            t, t, t, c, t
        });
    }

    static CImg basic_caret(Color c){ 
        return CImg(6,3,{
            t, t, c, t, t, t,
            t, c, t, c, t, t,
            c, t, t, t, c, t
        });
    }

public:

    static CImg basic_char(wchar_t ch, Color c){
            switch (ch)
            {
                case L'a': return basic_a(c); 
                case L'à': return basic_a_grave(c); 
                case L'b': return basic_b(c); 
                case L'c': return basic_c(c); 
                case L'd': return basic_d(c); 
                case L'e': return basic_e(c); 
                case L'è': return basic_e_grave(c); 
                case L'é': return basic_e_acute(c); 
                case L'f': return basic_f(c); 
                case L'g': return basic_g(c); 
                case L'h': return basic_h(c); 
                case L'i': return basic_i(c); 
                case L'ì': return basic_i_grave(c); 
                case L'j': return basic_j(c); 
                case L'k': return basic_k(c); 
                case L'l': return basic_l(c); 
                case L'm': return basic_m(c); 
                case L'n': return basic_n(c); 
                case L'o': return basic_o(c); 
                case L'ò': return basic_o_grave(c); 
                case L'p': return basic_p(c); 
                case L'q': return basic_q(c); 
                case L'r': return basic_r(c); 
                case L's': return basic_s(c); 
                case L't': return basic_t(c); 
                case L'u': return basic_u(c); 
                case L'ù': return basic_u_grave(c); 
                case L'v': return basic_v(c); 
                case L'w': return basic_w(c); 
                case L'x': return basic_x(c); 
                case L'y': return basic_y(c); 
                case L'z': return basic_z(c); 
                case L'A': return basic_A(c); 
                case L'B': return basic_B(c); 
                case L'C': return basic_C(c); 
                case L'D': return basic_D(c); 
                case L'E': return basic_E(c); 
                case L'F': return basic_F(c); 
                case L'G': return basic_G(c); 
                case L'H': return basic_H(c); 
                case L'I': return basic_I(c); 
                case L'J': return basic_J(c); 
                case L'K': return basic_K(c); 
                case L'L': return basic_L(c); 
                case L'M': return basic_M(c); 
                case L'N': return basic_N(c); 
                case L'O': return basic_O(c); 
                case L'P': return basic_P(c); 
                case L'Q': return basic_Q(c); 
                case L'R': return basic_R(c); 
                case L'S': return basic_S(c); 
                case L'T': return basic_T(c); 
                case L'U': return basic_U(c); 
                case L'V': return basic_V(c); 
                case L'W': return basic_W(c); 
                case L'X': return basic_X(c); 
                case L'Y': return basic_Y(c); 
                case L'Z': return basic_Z(c); 
                case L'0': return basic_0(c); 
                case L'1': return basic_1(c); 
                case L'2': return basic_2(c); 
                case L'3': return basic_3(c); 
                case L'4': return basic_4(c); 
                case L'5': return basic_5(c); 
                case L'6': return basic_6(c); 
                case L'7': return basic_7(c); 
                case L'8': return basic_8(c); 
                case L'9': return basic_9(c); 
                case L' ': return basic_space(); 
                case L'!': return basic_exclamation_mark(c); 
                case L'?': return basic_question_mark(c); 
                case L'.': return basic_period(c); 
                case L',': return basic_comma(c); 
                case L':': return basic_colon(c); 
                case L';': return basic_semicolon(c); 
                case L'\'': return basic_apostrophe(c); 
                case L'\"': return basic_quotation_marks(c); 
                case L'+': return basic_plus_sign(c);  
                case L'-': return basic_minus_sign(c);  
                case L'*': return basic_asterisk(c);  
                case L'/': return basic_slash(c);  
                case L'\\': return basic_backslash(c);  
                case L'|': return basic_pipe(c);  
                case L'(': return basic_left_parenthesis(c);  
                case L')': return basic_right_parenthesis(c);  
                case L'[': return basic_left_square_bracket(c);  
                case L']': return basic_right_square_bracket(c);  
                case L'{': return basic_left_curly_brace(c);  
                case L'}': return basic_right_curly_brace(c);  
                case L'=': return basic_equals_sign(c);  
                case L'&': return basic_ampersand(c);  
                case L'%': return basic_percent_sign(c);  
                case L'£': return basic_pound_sign(c);  
                case L'$': return basic_dollar_sign(c);  
                case L'€': return basic_euro_sign(c);  
                case L'_': return basic_underscore(c);  
                case L'#': return basic_hash(c);  
                case L'@': return basic_at_sign(c);  
                case L'<': return basic_less_sign(c);  
                case L'>': return basic_greater_sign(c);  
                case L'^': return basic_caret(c);  
                default: return basic_question_mark(c); 
            }
    }
    
};

inline void Canvas::drawText(const std::wstring& text, int x, int y, Color c) {
    requireInitialized();

    for (wchar_t ch : text) {
        CImg glyph = Font::basic_char(ch, c);
        drawImg(glyph, x, y);
        x += static_cast<int>(glyph.getWidth());
    }
}

#endif
