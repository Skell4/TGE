
#ifndef TGE_HPP

#define TGE_HPP

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

#ifdef _WIN32
    #include <windows.h>
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
        return std::to_string(_r)+','+std::to_string(_g)+','+std::to_string(_b)+','+std::to_string(_a);;
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

const Color Color::white(255,255,255);
const Color Color::black(0,0,0);

const Color Color::red(255,0,0);
const Color Color::green(0,255,0);
const Color Color::blue(0,0,255);

const Color Color::yellow(255,255,0);
const Color Color::cyan(0,255,255);
const Color Color::magenta(255,0,255);

const Color Color::light_grey(200,200,200);
const Color Color::grey(128,128,128);
const Color Color::dark_grey(50,50,50);

const Color Color::orange(255,165,0);
const Color Color::purple(128,0,128);
const Color Color::pink(255,192,203);

const Color Color::brown(139,69,19);
const Color Color::lime(50,205,50);
const Color Color::navy(0,0,128);
const Color Color::teal(0,128,128);
const Color Color::olive(128,128,0);

const Color Color::gold(255,215,0);
const Color Color::silver(192,192,192);
const Color Color::bronze(205,127,50);

const Color Color::sky_blue(135,206,235);
const Color Color::violet(238,130,238);
const Color Color::indigo(75,0,130);
const Color Color::turquoise(64,224,208);

class CImg{
private:
    size_t _width;
    size_t _high;
    std::vector<Color> _pixels;
    bool _isLoaded = false;
public: 
    bool load(std::string & file_name){
        _isLoaded = false;

        std::fstream fin(file_name,std::ios::in | std::ios::binary);
        if(!fin) return false;

        _pixels.clear();

        uint32_t w, h;
        if (!fin.read((char*)&w, sizeof(w))) { fin.close(); return false; }
        if (!fin.read((char*)&h, sizeof(h))) { fin.close(); return false; }
        _width = w;
        _high  = h;

        uint8_t r, g, b, a;
        for(size_t i = 0; i < _width * _high; i++){
            if (!fin.read((char*)&r, sizeof(r))) { fin.close(); return false; }
            if (!fin.read((char*)&g, sizeof(g))) { fin.close(); return false; }
            if (!fin.read((char*)&b, sizeof(b))) { fin.close(); return false; }
            if (!fin.read((char*)&a, sizeof(a))) { fin.close(); return false; }
            _pixels.push_back(Color(r,g,b,a));
        }

        _isLoaded = true;
        return true;
    }

    CImg & operator=(const CImg & other){
        _pixels = other._pixels;
        _high = other._high;
        _width = other._width;
        _isLoaded = other._isLoaded;
        return *this;
    }

    bool operator==(const CImg& other) const {
        return _pixels == other._pixels &&
               _high == other._high &&
               _width == other._width &&
               _isLoaded == other._isLoaded;
    }

    CImg(std::string file) : _width(0), _high(0) {
        _isLoaded = load(file);
    }

    CImg(){}
    
    CImg(const CImg & other){ 
        _pixels = other._pixels;
        _high = other._high;
        _width = other._width;
        _isLoaded = other._isLoaded;
    }

    CImg(size_t width,size_t high,std::vector<Color> pixels){
        if(width == 0 || high == 0 || width*high != pixels.size()){
            throw std::invalid_argument("Error: Unable to initialize CImg");
        }
        _isLoaded = true;
        _width = width;
        _high = high;
        _pixels = pixels;
    }

    inline size_t index(size_t x, size_t y) const {
        return y * _width + x;
    }

    Color getPixelColor(size_t x, size_t y) const {
        if(x >= _width || y >= _high) throw std::out_of_range("Error: Pixel out of range");
        return _pixels[index(x, y)];
    }

    Color getPixelColor(size_t i) const {
        if(i >= _pixels.size()) throw std::out_of_range("Error: Pixel out of range");
        return _pixels[i];
    }

    bool isLoaded() const {
        return _isLoaded;
    }

    size_t getHigh() const { return _high; }
    size_t getWidth() const { return _width; }
    size_t getSize() const { return _pixels.size(); }
};



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



class Canvas {
private:
    size_t _width;
    size_t _high;
    std::vector<Color> _pixels; 
    std::vector<Color> _temp_pixels;
    std::vector<Color> _old_pixels;


    inline size_t index(size_t x, size_t y) const {
        return y * _width + x;
    }

    Color getPixelColor(size_t x, size_t y) const {
        if(x >= _width || y >= _high) throw std::out_of_range("Error: Pixel out of range");
        return _pixels[index(x, y)];
    }

    Color getOldPixelColor(size_t x, size_t y) const {
        if(x >= _width || y >= _high) throw std::out_of_range("Error: Pixel out of range");
        return _old_pixels[index(x, y)];
    }

    void setPixel(int x, int y, Color color) {
        if(x >= 0 && y >= 0 && x < (int)_width && y < (int)_high)
            _temp_pixels[index(x, y)] = color;
    }


public:

    Canvas(size_t width, size_t high)
        : _width(width), _high(high), 
        _pixels(width * high,  CONSOLE_BG_COLOR),
        _temp_pixels(width * high, Color(0,0,0,0)),
        _old_pixels(width * high, Color(1,1,1,1))
    {
        if(width == 0 || high == 0){
            throw std::invalid_argument("Error: screen size must be > 0");
        }
        #ifdef _WIN32
            SetConsoleOutputCP(CP_UTF8);
        #endif
        std::ios::sync_with_stdio(false);
        std::cout << "\033[?25l";
    }

    Canvas(size_t width, size_t high, std::string title): 
        _width(width), _high(high), 
        _pixels(width * high,  CONSOLE_BG_COLOR),
        _temp_pixels(width * high, Color(0,0,0,0)),
        _old_pixels(width * high, Color(1,1,1,1))
    {
        if(width == 0 || high == 0){
            throw std::invalid_argument("Error: screen size must be > 0");
        }
        #ifdef _WIN32
            SetConsoleOutputCP(CP_UTF8);
        #endif
        std::ios::sync_with_stdio(false);
        std::cout << "\033[?25l";
        setTitle(title);
    }

    size_t getHigh() const { return _high; }
    size_t getWidth() const { return _width; }

    #ifdef _WIN32
        void setTitle(const std::string& title){
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, NULL, 0);
            std::wstring wtitle(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wtitle[0], size_needed);
            SetConsoleTitleW(wtitle.c_str());
        }
    #else
        void setTitle(const std::string& title){
            std::cout << "\033]0;" << title << "\007" << std::flush;
        }
    #endif

    void print() {
        std::string buffer;
        bool need_reposition = true;
        buffer.reserve(_width * (_high / 2) * 40);
        
        char tmp[12];
        for (size_t y = 0; y < _high; y += 2) {
            for (size_t x = 0; x < _width; x++) {
                Color top    = getPixelColor(x, y);
                Color bottom = (y + 1 < _high) ? getPixelColor(x, y + 1) : CONSOLE_BG_COLOR;

                Color old_top    = getOldPixelColor(x, y);
                Color old_bottom = (y + 1 < _high) ? getOldPixelColor(x, y + 1) : CONSOLE_BG_COLOR;

                if(top==old_top && bottom==old_bottom){
                    need_reposition = true; 
                    continue;
                } else {
                    if(need_reposition){
                        buffer+= "\033[";
                        buffer+= std::to_string(y/2 + 1);
                        buffer+= ";";
                        buffer+= std::to_string(x+1);
                        buffer+= "H";
                        need_reposition = false;
                    }
                    buffer += "\033[38;2;";
                    buffer.append(tmp, top.toChars(tmp));
                    buffer += "m\033[48;2;";
                    buffer.append(tmp, bottom.toChars(tmp));
                    buffer += "m▀";
                }


            }
            need_reposition = true;
        }
        std::cout.write(buffer.data(), buffer.size());
        std::cout.flush();
        _old_pixels = _pixels;
    }

    void setBG(Color c){
        _pixels.assign(_pixels.size(),c);
    }

    void drawPixel(int x, int y, Color c){
        if(x >= 0 && y >= 0 && x < (int)_width && y < (int)_high)
            _pixels[index(x, y)] = _pixels[index(x, y)].overlapColor(c);
    }

    void drawLine(int x0, int y0, int x1, int y1, int thickness, Color c) {
        if(thickness <= 0) return;
        std::fill(_temp_pixels.begin(), _temp_pixels.end(), Color(0,0,0,0));

        float half = thickness / 2.0f;

        int minX = std::max(0,            std::min(x0, x1) - thickness);
        int maxX = std::min((int)_width  - 1, std::max(x0, x1) + thickness);
        int minY = std::max(0,            std::min(y0, y1) - thickness);
        int maxY = std::min((int)_high   - 1, std::max(y0, y1) + thickness);

        float ldx   = (float)(x1 - x0);
        float ldy   = (float)(y1 - y0);
        float lenSq = ldx * ldx + ldy * ldy;

        for(int y = minY; y <= maxY; y++) {
            for(int x = minX; x <= maxX; x++) {
                float dist;
                if(lenSq == 0.0f) {
                    float ex = x - x0, ey = y - y0;
                    dist = std::sqrt(ex * ex + ey * ey);
                } else {
                    float t = ((x - x0) * ldx + (y - y0) * ldy) / lenSq;
                    t = std::clamp(t, 0.0f, 1.0f);
                    float ex = x - (x0 + t * ldx);
                    float ey = y - (y0 + t * ldy);
                    dist = std::sqrt(ex * ex + ey * ey);
                }
                if(dist <= half) setPixel(x, y, c);
            }
        }

        for(size_t i = 0; i < _pixels.size(); i++)
            _pixels[i] = _pixels[i].overlapColor(_temp_pixels[i]);
    }

    void drawFilledCircle(int cx, int cy, int r, Color c)
    {
        std::fill(_temp_pixels.begin(), _temp_pixels.end(), Color(0,0,0,0));
        int x = 0;
        int y = r;
        int d = 1 - r;


        while (y >= x){
            for (int i = cx - x; i <= cx + x; i++){
                setPixel(i, cy + y, c);
                setPixel(i, cy - y, c);
            }

            if (x != y){
                for (int i = cx - y; i <= cx + y; i++){
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
        for(size_t i = 0; i < _pixels.size(); i++){
            _pixels[i]=_pixels[i].overlapColor(_temp_pixels[i]);
        }
    }
    
    void drawCircle(int cx, int cy, int r, Color c){   
        std::fill(_temp_pixels.begin(), _temp_pixels.end(), Color(0,0,0,0));

        int x = 0;
        int y = r;
        int d = 1 - r;

        while (x <= y){
            setPixel(cx + x, cy + y, c);
            setPixel(cx - x, cy + y, c);
            setPixel(cx + x, cy - y, c);
            setPixel(cx - x, cy - y, c);

            setPixel(cx + y, cy + x, c);
            setPixel(cx - y, cy + x, c);
            setPixel(cx + y, cy - x, c);
            setPixel(cx - y, cy - x, c);

            x++;

            if (d < 0){
                d += 2 * x + 1;
            } else{
                y--;
                d += 2 * (x - y) + 1;
            }
        }
        for(size_t i = 0; i < _pixels.size(); i++){
            _pixels[i]=_pixels[i].overlapColor(_temp_pixels[i]);
        }
    }
    void drawFilledRect(int x, int y, int w, int h, Color c) {
        if(w <= 0 || h <= 0) return;
        std::fill(_temp_pixels.begin(), _temp_pixels.end(), Color(0,0,0,0));

        int x0 = std::max(0, x);
        int y0 = std::max(0, y);
        int x1 = std::min((int)_width,  x + w);
        int y1 = std::min((int)_high,   y + h);

        for(int py = y0; py < y1; py++)
            for(int px = x0; px < x1; px++)
                setPixel(px, py,c);

        for(size_t i = 0; i < _pixels.size(); i++)
            _pixels[i] = _pixels[i].overlapColor(_temp_pixels[i]);
    }

    void drawRect(int x, int y, int w, int h, int thickness, Color c) {
        if(w == 0 || h == 0 || thickness <= 0) return;
        if(w < 0) { x += w; w = -w; }
        if(h < 0) { y += h; h = -h; }
        std::fill(_temp_pixels.begin(), _temp_pixels.end(), Color(0,0,0,0));

        for(int t = 0; t < thickness; t++) {
            for(int i = x; i < x + w; i++) {
                setPixel(i, y + t, c);
                setPixel(i, y + h - 1 - t, c);
            }
            for(int i = y; i < y + h; i++) {
                setPixel(x + t, i, c);     
                setPixel(x + w - 1 - t, i, c); 
            }
        }

        for(size_t i = 0; i < _pixels.size(); i++)
            _pixels[i] = _pixels[i].overlapColor(_temp_pixels[i]);
    }
    void drawText(std::wstring text, int x, int y, Color c){
        CImg cimg;
        for(wchar_t ch : text){
            cimg = Font::basic_char(ch,c);
            drawImg(cimg,x,y);
            x+=cimg.getWidth();
        }
    }

    bool drawImg(const CImg& img, int x, int y){
        if(!img.isLoaded()) return false;
        std::fill(_temp_pixels.begin(), _temp_pixels.end(), Color(0,0,0,0));
        for(size_t rx = 0; rx < img.getWidth(); rx++){
            for(size_t ry = 0; ry < img.getHigh(); ry++){
                setPixel(x+rx,y+ry,img.getPixelColor(rx,ry));
            }
        }
        for(size_t i = 0; i < _pixels.size(); i++)
            _pixels[i] = _pixels[i].overlapColor(_temp_pixels[i]);
        return true;
    }

    bool drawTexture(const CImg& img, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)  
    {
        if(!img.isLoaded()) return false;
        std::fill(_temp_pixels.begin(), _temp_pixels.end(), Color(0,0,0,0));
        auto sample = [&](float u, float v) -> Color {
            u = std::clamp(u, 0.0f, 1.0f);
            v = std::clamp(v, 0.0f, 1.0f);
            size_t tx = (size_t)(u *     (img.getWidth()  - 1));
            size_t ty = (size_t)(v * (img.getHigh() - 1));
            return img.getPixelColor(tx, ty);
        };

        auto rasterTri = [&](
            float ax, float ay, float au, float av,
            float bx, float by, float bu, float bv,
            float cx, float cy, float cu, float cv)
        {
            int minX = std::max(0,            (int)std::floor(std::min({ax, bx, cx})));
            int maxX = std::min((int)_width  - 1, (int)std::ceil (std::max({ax, bx, cx})));
            int minY = std::max(0,            (int)std::floor(std::min({ay, by, cy})));
            int maxY = std::min((int)_high   - 1, (int)std::ceil (std::max({ay, by, cy})));

            float denom = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy);
            if(std::abs(denom) < 1e-6f) return; 

            for(int py = minY; py <= maxY; py++) {
                for(int px = minX; px <= maxX; px++) {

                    float w0 = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / denom;
                    float w1 = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / denom;
                    float w2 = 1.0f - w0 - w1;

                    if(w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                        float u = w0 * au + w1 * bu + w2 * cu;
                        float v = w0 * av + w1 * bv + w2 * cv;
                        setPixel(px, py, sample(u, v));
                    }
                }
            }
        };

        rasterTri(
            (float)x0, (float)y0,  0.0f, 0.0f,
            (float)x1, (float)y1,  1.0f, 0.0f,
            (float)x2, (float)y2,  1.0f, 1.0f
        );

        rasterTri(
            (float)x0, (float)y0,  0.0f, 0.0f,
            (float)x2, (float)y2,  1.0f, 1.0f,
            (float)x3, (float)y3,  0.0f, 1.0f
        );

        for(size_t i = 0; i < _pixels.size(); i++)
            _pixels[i] = _pixels[i].overlapColor(_temp_pixels[i]);
        return true;
    }

    static void sleep(int s){
        std::this_thread::sleep_for(std::chrono::milliseconds(s));
    }



    std::vector<Color> getPixels(){
        return _pixels;
    }

    ~Canvas() {
        std::cout<<"\033[?25h"<<std::flush;
    }



};


#endif