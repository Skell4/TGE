#ifndef TGE_HPP

#define TGE_HPP

// TGE 4.2

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
#include <array>
#include <string_view>
#include <map>
#include <limits>

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
    #define CONSOLE_BG_COLOR ::tge::Color(12,12,12)
#endif

namespace tge {

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

namespace detail {
namespace png {

inline constexpr size_t maxBufferBytes = size_t(256) * 1024 * 1024;

inline bool hasSignature(const uint8_t* bytes) {
    static constexpr uint8_t signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    return std::equal(signature, signature + 8, bytes);
}

inline uint32_t read32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

inline unsigned read16(const uint8_t* p) {
    return (unsigned(p[0]) << 8) | unsigned(p[1]);
}

inline uint32_t crc32(const uint8_t* bytes, size_t size) {
    static const std::array<uint32_t, 256> table = [] {
        std::array<uint32_t, 256> values{};
        for (unsigned i = 0; i < 256; ++i) {
            uint32_t value = i;
            for (unsigned bit = 0; bit < 8; ++bit)
                value = (value >> 1) ^ ((value & 1) ? 0xedb88320u : 0u);
            values[i] = value;
        }
        return values;
    }();
    uint32_t crc = 0xffffffffu;
    for (size_t i = 0; i < size; ++i)
        crc = table[(crc ^ bytes[i]) & 255u] ^ (crc >> 8);
    return crc ^ 0xffffffffu;
}

inline uint32_t adler32(const std::vector<uint8_t>& bytes) {
    uint32_t a = 1, b = 0;
    size_t pos = 0;
    while (pos < bytes.size()) {
        const size_t end = pos + (std::min)(size_t(5552), bytes.size() - pos);
        while (pos < end) {
            a += bytes[pos++];
            b += a;
        }
        a %= 65521;
        b %= 65521;
    }
    return (b << 16) | a;
}

class BitReader {
    const uint8_t* _data;
    size_t _size;
    size_t _position = 0;
    uint32_t _buffer = 0;
    unsigned _bits = 0;
public:
    BitReader(const uint8_t* data, size_t size) : _data(data), _size(size) { }

    bool read(unsigned count, unsigned& value) {
        while (_bits < count) {
            if (_position == _size)
                return false;
            _buffer |= uint32_t(_data[_position++]) << _bits;
            _bits += 8;
        }
        value = _buffer & ((1u << count) - 1u);
        _buffer >>= count;
        _bits -= count;
        return true;
    }

    void align() { _buffer = 0; _bits = 0; }
    size_t position() const { return _position; }
};

class Huffman {
    std::array<unsigned, 16> _counts{};
    std::array<unsigned, 288> _symbols{};
    unsigned _maxBits = 0;
public:
    bool build(const uint8_t* lengths, unsigned size,
               bool codeLengthTree = false, bool allowEmpty = false) {
        _counts.fill(0);
        _maxBits = 0;
        if (size > _symbols.size())
            return false;
        unsigned used = 0;
        for (unsigned i = 0; i < size; ++i) {
            const unsigned bits = lengths[i];
            if (bits > 15)
                return false;
            if (bits) {
                ++_counts[bits];
                ++used;
                _maxBits = (std::max)(_maxBits, bits);
            }
        }
        if (!used)
            return allowEmpty;

        int available = 1;
        for (unsigned bits = 1; bits <= 15; ++bits) {
            available = available * 2 - int(_counts[bits]);
            if (available < 0)
                return false;
        }
        if (available && (codeLengthTree || _maxBits != 1))
            return false;

        std::array<unsigned, 16> offsets{};
        for (unsigned bits = 1; bits < 15; ++bits)
            offsets[bits + 1] = offsets[bits] + _counts[bits];
        for (unsigned i = 0; i < size; ++i)
            if (lengths[i])
                _symbols[offsets[lengths[i]]++] = i;
        return true;
    }

    bool decode(BitReader& input, unsigned& symbol) const {
        unsigned code = 0, first = 0, index = 0;
        for (unsigned bits = 1; bits <= _maxBits; ++bits) {
            unsigned bit;
            if (!input.read(1, bit))
                return false;
            code = (code << 1) | bit;
            if (code >= first && code - first < _counts[bits]) {
                symbol = _symbols[index + code - first];
                return true;
            }
            index += _counts[bits];
            first = (first + _counts[bits]) << 1;
        }
        return false;
    }
};

inline bool inflate(const std::vector<uint8_t>& compressed, size_t expected,
                    std::vector<uint8_t>& output) {
    if (compressed.size() < 6 || expected > maxBufferBytes)
        return false;
    const unsigned cmf = compressed[0], flg = compressed[1];
    if ((cmf & 15u) != 8 || (cmf >> 4) > 7 ||
        ((cmf << 8) | flg) % 31 != 0 || (flg & 32u))
        return false;
    const unsigned window = 1u << ((cmf >> 4) + 8);
    BitReader input(compressed.data() + 2, compressed.size() - 2);
    output.clear();
    output.reserve(expected);

    static constexpr unsigned lengthBase[29] = {
        3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,
        35,43,51,59,67,83,99,115,131,163,195,227,258
    };
    static constexpr unsigned lengthBits[29] = {
        0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
    };
    static constexpr unsigned distanceBase[30] = {
        1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
        257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577
    };
    static constexpr unsigned distanceBits[30] = {
        0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
    };

    unsigned finalBlock = 0;
    do {
        unsigned type;
        if (!input.read(1, finalBlock) || !input.read(2, type) || type == 3)
            return false;
        if (type == 0) {
            input.align();
            unsigned length, inverse;
            if (!input.read(16, length) || !input.read(16, inverse) ||
                (length ^ inverse) != 65535u || length > expected - output.size())
                return false;
            for (unsigned i = 0; i < length; ++i) {
                unsigned byte;
                if (!input.read(8, byte))
                    return false;
                output.push_back(uint8_t(byte));
            }
            continue;
        }

        Huffman literals, distances;
        if (type == 1) {
            std::array<uint8_t, 288> lengths{};
            for (unsigned i = 0; i < lengths.size(); ++i)
                lengths[i] = uint8_t(i < 144 ? 8 : i < 256 ? 9 : i < 280 ? 7 : 8);
            std::array<uint8_t, 32> distLengths{};
            distLengths.fill(5);
            if (!literals.build(lengths.data(), unsigned(lengths.size())) ||
                !distances.build(distLengths.data(), unsigned(distLengths.size())))
                return false;
        } else {
            unsigned literalCount, distanceCount, codeCount;
            if (!input.read(5, literalCount) || !input.read(5, distanceCount) ||
                !input.read(4, codeCount))
                return false;
            literalCount += 257;
            distanceCount += 1;
            codeCount += 4;
            if (literalCount > 286)
                return false;
            static constexpr unsigned order[19] = {
                16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
            };
            std::array<uint8_t, 19> codeLengths{};
            for (unsigned i = 0; i < codeCount; ++i) {
                unsigned bits;
                if (!input.read(3, bits))
                    return false;
                codeLengths[order[i]] = uint8_t(bits);
            }
            Huffman codes;
            if (!codes.build(codeLengths.data(), unsigned(codeLengths.size()), true))
                return false;
            std::array<uint8_t, 318> lengths{};
            const unsigned total = literalCount + distanceCount;
            unsigned count = 0;
            while (count < total) {
                unsigned symbol;
                if (!codes.decode(input, symbol))
                    return false;
                if (symbol < 16) {
                    lengths[count++] = uint8_t(symbol);
                } else {
                    if (symbol == 16 && count == 0)
                        return false;
                    unsigned extra;
                    if (!input.read(symbol == 16 ? 2 : symbol == 17 ? 3 : 7, extra))
                        return false;
                    const unsigned repeat = extra + (symbol == 18 ? 11 : 3);
                    const uint8_t value = symbol == 16 ? lengths[count - 1] : uint8_t(0);
                    if (repeat > total - count)
                        return false;
                    for (unsigned i = 0; i < repeat; ++i)
                        lengths[count++] = value;
                }
            }
            if (!lengths[256] || !literals.build(lengths.data(), literalCount) ||
                !distances.build(lengths.data() + literalCount, distanceCount, false, true))
                return false;
        }

        for (;;) {
            unsigned symbol;
            if (!literals.decode(input, symbol))
                return false;
            if (symbol < 256) {
                if (output.size() == expected)
                    return false;
                output.push_back(uint8_t(symbol));
            } else if (symbol == 256) {
                break;
            } else {
                if (symbol > 285)
                    return false;
                unsigned extra, distanceSymbol;
                if (!input.read(lengthBits[symbol - 257], extra))
                    return false;
                const unsigned length = lengthBase[symbol - 257] + extra;
                if (!distances.decode(input, distanceSymbol) || distanceSymbol > 29 ||
                    !input.read(distanceBits[distanceSymbol], extra))
                    return false;
                const unsigned distance = distanceBase[distanceSymbol] + extra;
                if (distance > output.size() || distance > window ||
                    length > expected - output.size())
                    return false;
                for (unsigned i = 0; i < length; ++i)
                    output.push_back(output[output.size() - distance]);
            }
        }
    } while (!finalBlock);

    input.align();
    if (output.size() != expected || input.position() > compressed.size() - 6)
        return false;
    return read32(compressed.data() + 2 + input.position()) == adler32(output);
}

struct Header {
    size_t width = 0, height = 0;
    unsigned depth = 0, type = 0, channels = 0, interlace = 0;
    std::array<std::array<uint8_t, 4>, 256> palette{};
    size_t paletteSize = 0;
    bool transparent = false;
    std::array<unsigned, 3> key{};
};

inline bool readHeader(const uint8_t* data, Header& header) {
    header.width = read32(data);
    header.height = read32(data + 4);
    header.depth = data[8];
    header.type = data[9];
    header.interlace = data[12];
    if (!header.width || !header.height || header.width > 0x7fffffffu ||
        header.height > 0x7fffffffu || data[10] || data[11] || header.interlace > 1)
        return false;
    const unsigned depth = header.depth;
    if (depth != 1 && depth != 2 && depth != 4 && depth != 8 && depth != 16)
        return false;
    switch (header.type) {
        case 0: header.channels = 1; break;
        case 2: header.channels = 3; if (depth < 8) return false; break;
        case 3: header.channels = 1; if (depth == 16) return false; break;
        case 4: header.channels = 2; if (depth < 8) return false; break;
        case 6: header.channels = 4; if (depth < 8) return false; break;
        default: return false;
    }
    return header.width <= (maxBufferBytes / sizeof(Color)) / header.height &&
           header.width <= std::vector<Color>().max_size() / header.height;
}

inline unsigned sample(const std::vector<uint8_t>& row, size_t index, unsigned depth) {
    if (depth == 8)
        return row[index];
    if (depth == 16)
        return read16(row.data() + index * 2);
    const size_t bit = index * depth;
    return (row[bit / 8] >> (8 - depth - unsigned(bit % 8))) & ((1u << depth) - 1u);
}

inline unsigned toByte(unsigned value, unsigned depth) {
    const unsigned maximum = (1u << depth) - 1u;
    return (value * 255u + maximum / 2u) / maximum;
}

inline bool pixel(const Header& header, const std::vector<uint8_t>& row,
                  size_t x, Color& output) {
    const unsigned depth = header.depth;
    const size_t base = x * header.channels;
    const unsigned first = sample(row, base, depth);
    if (header.type == 3) {
        if (first >= header.paletteSize)
            return false;
        const auto& entry = header.palette[first];
        output = Color(entry[0], entry[1], entry[2], entry[3]);
    } else if (header.type == 0 || header.type == 4) {
        const unsigned gray = toByte(first, depth);
        const unsigned alpha = header.type == 4 ? toByte(sample(row, base + 1, depth), depth) :
            (header.transparent && first == header.key[0] ? 0u : 255u);
        output = Color(gray, gray, gray, alpha);
    } else {
        const unsigned green = sample(row, base + 1, depth);
        const unsigned blue = sample(row, base + 2, depth);
        const unsigned alpha = header.type == 6 ? toByte(sample(row, base + 3, depth), depth) :
            (header.transparent && first == header.key[0] && green == header.key[1] &&
             blue == header.key[2] ? 0u : 255u);
        output = Color(toByte(first, depth), toByte(green, depth), toByte(blue, depth), alpha);
    }
    return true;
}

inline unsigned paeth(unsigned a, unsigned b, unsigned c) {
    const int p = int(a) + int(b) - int(c);
    const int da = std::abs(p - int(a)), db = std::abs(p - int(b)), dc = std::abs(p - int(c));
    return da <= db && da <= dc ? a : db <= dc ? b : c;
}

inline bool reconstruct(const Header& header, const std::vector<uint8_t>& compressed,
                        std::vector<Color>& pixels) {
    struct Pass { size_t x, y, dx, dy, width, height, rowBytes; };
    std::array<Pass, 7> passes{};
    static constexpr unsigned startX[7] = {0,4,0,2,0,1,0};
    static constexpr unsigned startY[7] = {0,0,4,0,2,0,1};
    static constexpr unsigned stepX[7] = {8,8,4,4,2,2,1};
    static constexpr unsigned stepY[7] = {8,8,8,4,4,2,2};
    const unsigned passCount = header.interlace ? 7 : 1;
    size_t expected = 0;
    for (unsigned i = 0; i < passCount; ++i) {
        auto& pass = passes[i];
        pass.x = header.interlace ? startX[i] : 0;
        pass.y = header.interlace ? startY[i] : 0;
        pass.dx = header.interlace ? stepX[i] : 1;
        pass.dy = header.interlace ? stepY[i] : 1;
        pass.width = header.width <= pass.x ? 0 : (header.width - pass.x - 1) / pass.dx + 1;
        pass.height = header.height <= pass.y ? 0 : (header.height - pass.y - 1) / pass.dy + 1;
        if (!pass.width || !pass.height)
            continue;
        const uint64_t rowBytes = (uint64_t(pass.width) * header.channels * header.depth + 7) / 8;
        if (rowBytes >= maxBufferBytes || pass.height > (maxBufferBytes - expected) / (rowBytes + 1))
            return false;
        pass.rowBytes = size_t(rowBytes);
        expected += (pass.rowBytes + 1) * pass.height;
    }

    std::vector<uint8_t> raw;
    if (!inflate(compressed, expected, raw))
        return false;
    pixels.resize(header.width * header.height);
    const size_t bytesPerPixel = (std::max)(size_t(1), size_t(header.channels * header.depth / 8));
    size_t position = 0;
    for (unsigned i = 0; i < passCount; ++i) {
        const auto& pass = passes[i];
        if (!pass.width || !pass.height)
            continue;
        std::vector<uint8_t> previous(pass.rowBytes, 0), row(pass.rowBytes);
        for (size_t y = 0; y < pass.height; ++y) {
            const unsigned filter = raw[position++];
            if (filter > 4)
                return false;
            for (size_t b = 0; b < pass.rowBytes; ++b) {
                const unsigned left = b < bytesPerPixel ? 0 : row[b - bytesPerPixel];
                const unsigned up = previous[b];
                const unsigned corner = b < bytesPerPixel ? 0 : previous[b - bytesPerPixel];
                unsigned prediction = 0;
                switch (filter) {
                    case 1: prediction = left; break;
                    case 2: prediction = up; break;
                    case 3: prediction = (left + up) / 2; break;
                    case 4: prediction = paeth(left, up, corner); break;
                    default: break;
                }
                row[b] = uint8_t(raw[position + b] + prediction);
            }
            position += pass.rowBytes;
            const size_t destination = (pass.y + y * pass.dy) * header.width + pass.x;
            for (size_t x = 0; x < pass.width; ++x)
                if (!pixel(header, row, x, pixels[destination + x * pass.dx]))
                    return false;
            previous.swap(row);
        }
    }
    return position == raw.size();
}

inline bool decode(const std::vector<uint8_t>& file, size_t& width, size_t& height,
                   std::vector<Color>& pixels) {
    if (file.size() < 8 || file.size() > maxBufferBytes || !hasSignature(file.data()))
        return false;
    Header header;
    bool haveHeader = false, havePalette = false, haveTransparency = false;
    bool haveData = false, dataEnded = false, haveEnd = false;
    std::vector<uint8_t> compressed;
    size_t position = 8;
    while (file.size() - position >= 12) {
        const size_t length = read32(file.data() + position);
        if (length > 0x7fffffffu || length > file.size() - position - 12)
            return false;
        const uint8_t* type = file.data() + position + 4;
        const uint8_t* data = type + 4;
        for (unsigned i = 0; i < 4; ++i)
            if (!((type[i] >= 'A' && type[i] <= 'Z') || (type[i] >= 'a' && type[i] <= 'z')))
                return false;
        if (crc32(type, length + 4) != read32(data + length))
            return false;
        const uint32_t name = read32(type);
        if (!haveHeader && name != 0x49484452u)
            return false;
        if (haveData && name != 0x49444154u)
            dataEnded = true;
        if (name == 0x49484452u) {
            if (haveHeader || position != 8 || length != 13 || !readHeader(data, header))
                return false;
            haveHeader = true;
        } else if (name == 0x504c5445u) {
            if (havePalette || haveTransparency || haveData || !length || length % 3 ||
                length > 768 || header.type == 0 || header.type == 4)
                return false;
            header.paletteSize = length / 3;
            if (header.type == 3 && header.paletteSize > (size_t(1) << header.depth))
                return false;
            for (size_t i = 0; i < header.paletteSize; ++i)
                header.palette[i] = {data[i * 3], data[i * 3 + 1], data[i * 3 + 2], 255};
            havePalette = true;
        } else if (name == 0x74524e53u) {
            if (haveTransparency || haveData)
                return false;
            const unsigned mask = (1u << header.depth) - 1u;
            if (header.type == 3) {
                if (!havePalette || length > header.paletteSize)
                    return false;
                for (size_t i = 0; i < length; ++i)
                    header.palette[i][3] = data[i];
            } else if (header.type == 0 || header.type == 2) {
                const unsigned count = header.type == 0 ? 1 : 3;
                if (length != count * 2)
                    return false;
                for (unsigned i = 0; i < count; ++i)
                    header.key[i] = read16(data + i * 2) & mask;
                header.transparent = true;
            } else {
                return false;
            }
            haveTransparency = true;
        } else if (name == 0x49444154u) {
            if (dataEnded || (header.type == 3 && !havePalette) ||
                length > maxBufferBytes - compressed.size())
                return false;
            compressed.insert(compressed.end(), data, data + length);
            haveData = true;
        } else if (name == 0x49454e44u) {
            if (length || !haveData || position + 12 != file.size())
                return false;
            haveEnd = true;
            break;
        } else if (!(type[0] & 32u)) {
            return false;
        }
        position += length + 12;
    }
    if (!haveEnd || !reconstruct(header, compressed, pixels))
        return false;
    width = header.width;
    height = header.height;
    return true;
}

}

namespace bitmap {

inline unsigned read16(const uint8_t* p) {
    return unsigned(p[0]) | (unsigned(p[1]) << 8);
}

inline uint32_t read32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
           (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

inline int64_t signed32(const uint8_t* p) {
    const uint32_t value = read32(p);
    return (value & 0x80000000u) ? int64_t(value) - 0x100000000LL : int64_t(value);
}

struct Mask {
    uint32_t bits = 0, maximum = 0;
    unsigned shift = 0;

    bool set(uint32_t value, unsigned depth) {
        bits = value;
        shift = 0;
        maximum = 0;
        if (!value)
            return true;
        if (depth == 16 && (value & 0xffff0000u))
            return false;
        while (!(value & 1u)) {
            value >>= 1;
            ++shift;
        }
        maximum = value;
        return !(value & (value + 1u));
    }

    unsigned get(uint32_t value) const {
        return bits ? unsigned((uint64_t((value & bits) >> shift) * 255 + maximum / 2) / maximum) : 255;
    }
};

inline bool rle(const uint8_t* data, size_t size, size_t width, size_t height,
                unsigned depth, const std::array<Color, 256>& palette, size_t colors,
                std::vector<Color>& pixels) {
    pixels.assign(width * height, palette[0]);
    const size_t rowPixels = size_t(((uint64_t(width) * depth + 31) / 32) * 32 / depth);
    size_t position = 0, x = 0, y = 0;
    const auto write = [&](unsigned index) {
        if (y >= height || x >= rowPixels)
            return false;
        if (x < width) {
            if (index >= colors)
                return false;
            pixels[(height - 1 - y) * width + x] = palette[index];
        }
        ++x;
        return true;
    };
    while (size - position >= 2) {
        const unsigned count = data[position++], value = data[position++];
        if (count) {
            if (y >= height || count > rowPixels - x)
                return false;
            for (unsigned i = 0; i < count; ++i) {
                const unsigned index = depth == 8 ? value : (i & 1u) ? value & 15u : value >> 4;
                if (!write(index))
                    return false;
            }
        } else if (value == 0) {
            if (y >= height)
                return false;
            x = 0;
            ++y;
        } else if (value == 1) {
            return true;
        } else if (value == 2) {
            if (size - position < 2 || y >= height)
                return false;
            const unsigned dx = data[position++], dy = data[position++];
            if (dx > rowPixels - x || dy >= height - y)
                return false;
            x += dx;
            y += dy;
        } else {
            const size_t bytes = depth == 8 ? value : (value + 1u) / 2;
            const size_t padded = (bytes + 1) & ~size_t(1);
            if (y >= height || value > rowPixels - x || padded > size - position)
                return false;
            for (unsigned i = 0; i < value; ++i) {
                const unsigned index = depth == 8 ? data[position + i] :
                    (data[position + i / 2] >> ((i & 1u) ? 0 : 4)) & 15u;
                if (!write(index))
                    return false;
            }
            position += padded;
        }
    }
    return false;
}

inline bool dib(const uint8_t* data, size_t size, size_t pixelOffset,
                size_t& width, size_t& height, std::vector<Color>& pixels,
                size_t iconWidth = 0, size_t iconHeight = 0) {
    if (size < 12)
        return false;
    const size_t header = read32(data);
    const bool core = header == 12, icon = iconWidth != 0;
    if ((header != 12 && header != 40 && header != 52 && header != 56 &&
         header != 108 && header != 124) || header > size)
        return false;
    const int64_t w = core ? int64_t(read16(data + 4)) : signed32(data + 4);
    const int64_t h = core ? int64_t(read16(data + 6)) : signed32(data + 8);
    const unsigned planes = read16(data + (core ? 8 : 12));
    const unsigned depth = read16(data + (core ? 10 : 14));
    const unsigned compression = core ? 0 : read32(data + 16);
    const size_t imageBytes = core ? 0 : read32(data + 20);
    size_t colors = core ? 0 : read32(data + 32);
    if (w <= 0 || !h || h == -0x80000000LL || planes != 1 ||
        (depth != 1 && depth != 4 && depth != 8 && depth != 16 && depth != 24 && depth != 32) ||
        (core && (depth == 16 || depth == 32)))
        return false;
    const bool topDown = h < 0;
    const size_t localWidth = size_t(w), storedHeight = size_t(topDown ? -h : h);
    if (icon && (topDown || storedHeight % 2 || localWidth != iconWidth || storedHeight / 2 != iconHeight))
        return false;
    const size_t localHeight = icon ? storedHeight / 2 : storedHeight;
    if (!localHeight || localWidth > (png::maxBufferBytes / sizeof(Color)) / localHeight ||
        localWidth > std::vector<Color>().max_size() / localHeight)
        return false;
    const bool compressed = compression == 1 || compression == 2;
    if (compression != 0 && compression != 1 && compression != 2 && compression != 3 && compression != 6)
        return false;
    if ((compression == 1 && depth != 8) || (compression == 2 && depth != 4) ||
        (compressed && (topDown || icon)) ||
        ((compression == 3 || compression == 6) && depth != 16 && depth != 32))
        return false;

    size_t prefix = header;
    std::array<uint32_t, 4> values{};
    if (compression == 3 || compression == 6) {
        if (header == 40) {
            const size_t bytes = compression == 6 ? 16 : 12;
            if (bytes > size - prefix)
                return false;
            for (size_t i = 0; i < bytes / 4; ++i)
                values[i] = read32(data + prefix + i * 4);
            prefix += bytes;
        } else {
            if (header < 52 || (compression == 6 && header < 56))
                return false;
            for (unsigned i = 0; i < 3; ++i)
                values[i] = read32(data + 40 + i * 4);
            if (header >= 56)
                values[3] = read32(data + 52);
        }
        if (!values[0] || !values[1] || !values[2] || (compression == 6 && !values[3]))
            return false;
    } else if (depth == 16) {
        values = {0x7c00u, 0x03e0u, 0x001fu, 0};
    } else if (depth == 32) {
        values = {0xff0000u, 0x00ff00u, 0x0000ffu, 0};
    }
    if (icon && depth == 32 && !values[3] && (values[0] | values[1] | values[2]) == 0x00ffffffu)
        values[3] = 0xff000000u;
    std::array<Mask, 4> masks{};
    uint32_t used = 0;
    for (unsigned i = 0; i < 4; ++i) {
        if ((used & values[i]) || !masks[i].set(values[i], depth))
            return false;
        used |= values[i];
    }

    if (depth <= 8) {
        const size_t maximum = size_t(1) << depth;
        if (!colors)
            colors = maximum;
        if (colors > maximum)
            return false;
    }
    const size_t entryBytes = core ? 3 : 4;
    if (colors > (size - prefix) / entryBytes)
        return false;
    const size_t paletteOffset = prefix;
    std::array<Color, 256> palette{};
    if (depth <= 8)
        for (size_t i = 0; i < colors; ++i) {
            const uint8_t* entry = data + prefix + i * entryBytes;
            palette[i] = Color(entry[2], entry[1], entry[0]);
        }
    prefix += colors * entryBytes;
    if (icon)
        pixelOffset = prefix;
    if (pixelOffset < prefix || pixelOffset > size || (imageBytes && imageBytes > size - pixelOffset))
        return false;
    if (compressed) {
        const size_t bytes = imageBytes ? imageBytes : size - pixelOffset;
        if (bytes < 2 || !rle(data + pixelOffset, bytes, localWidth, localHeight, depth, palette, colors, pixels))
            return false;
        width = localWidth;
        height = localHeight;
        return true;
    }

    const uint64_t rowBytes = ((uint64_t(localWidth) * depth + 31) / 32) * 4;
    const uint64_t rasterBytes = rowBytes * localHeight;
    if (rasterBytes > size - pixelOffset || (imageBytes && imageBytes < rasterBytes))
        return false;
    const size_t stride = size_t(rowBytes), maskOffset = pixelOffset + size_t(rasterBytes);
    const size_t maskStride = size_t(((uint64_t(localWidth) + 31) / 32) * 4);
    const size_t maskBytes = maskStride * localHeight;
    bool iconAlpha = false;
    if (icon && masks[3].bits)
        for (size_t y = 0; y < localHeight && !iconAlpha; ++y) {
            const uint8_t* row = data + pixelOffset + y * stride;
            for (size_t x = 0; x < localWidth; ++x) {
                const uint32_t value = depth == 16 ? read16(row + x * 2) : read32(row + x * 4);
                if (value & masks[3].bits) {
                    iconAlpha = true;
                    break;
                }
            }
        }
    if (icon && maskBytes > size - maskOffset && !(iconAlpha && maskOffset == size))
        return false;
    pixels.resize(localWidth * localHeight);
    for (size_t y = 0; y < localHeight; ++y) {
        const size_t sourceY = topDown ? y : localHeight - 1 - y;
        const uint8_t* row = data + pixelOffset + sourceY * stride;
        for (size_t x = 0; x < localWidth; ++x) {
            unsigned r, g, b, a = 255;
            if (depth <= 8) {
                const unsigned index = depth == 8 ? row[x] :
                    depth == 4 ? (row[x / 2] >> ((x & 1u) ? 0 : 4)) & 15u :
                    (row[x / 8] >> (7 - unsigned(x % 8))) & 1u;
                if (index >= colors)
                    return false;
                const uint8_t* entry = data + paletteOffset + index * entryBytes;
                r = entry[2]; g = entry[1]; b = entry[0];
            } else if (depth == 24) {
                b = row[x * 3]; g = row[x * 3 + 1]; r = row[x * 3 + 2];
            } else {
                const uint32_t value = depth == 16 ? read16(row + x * 2) : read32(row + x * 4);
                r = masks[0].get(value); g = masks[1].get(value); b = masks[2].get(value);
                if (!icon || iconAlpha)
                    a = masks[3].get(value);
            }
            if (icon && !iconAlpha && (data[maskOffset + sourceY * maskStride + x / 8] & (0x80u >> (x % 8)))) {
                if (depth == 1 && (r || g || b))
                    return false;
                a = 0;
            }
            pixels[y * localWidth + x] = Color(r, g, b, a);
        }
    }
    width = localWidth;
    height = localHeight;
    return true;
}

inline bool decode(const std::vector<uint8_t>& file, size_t& width, size_t& height,
                   std::vector<Color>& pixels) {
    if (file.size() < 26 || file.size() > png::maxBufferBytes || file[0] != 'B' || file[1] != 'M')
        return false;
    const size_t declared = read32(file.data() + 2), offset = read32(file.data() + 10);
    if (read32(file.data() + 6) || declared < 26 || declared > file.size() || offset < 14 || offset > declared)
        return false;
    return dib(file.data() + 14, declared - 14, offset - 14, width, height, pixels);
}

}

namespace ico {

inline bool hasSignature(const uint8_t* bytes) {
    return bytes[0] == 0 && bytes[1] == 0 && bytes[2] == 1 && bytes[3] == 0;
}

inline bool decode(const std::vector<uint8_t>& file, size_t& width, size_t& height,
                   std::vector<Color>& pixels) {
    if (file.size() < 6 || file.size() > png::maxBufferBytes || !hasSignature(file.data()))
        return false;
    const unsigned count = bitmap::read16(file.data() + 4);
    if (!count || count > (file.size() - 6) / 16)
        return false;
    const size_t directoryEnd = 6 + size_t(count) * 16;
    struct Entry { size_t offset, size, width, height; unsigned depth; bool png; };
    std::vector<Entry> entries;
    entries.reserve(count);
    for (unsigned i = 0; i < count; ++i) {
        const uint8_t* entry = file.data() + 6 + size_t(i) * 16;
        const size_t size = bitmap::read32(entry + 8), offset = bitmap::read32(entry + 12);
        if (entry[3] || bitmap::read16(entry + 4) > 1 || size < 8 ||
            offset < directoryEnd || offset > file.size() || size > file.size() - offset)
            return false;
        const uint8_t* image = file.data() + offset;
        const bool isPng = png::hasSignature(image);
        unsigned depth = 0;
        if (isPng && size >= 33) {
            const unsigned type = image[25];
            const unsigned channels = type == 2 ? 3 : type == 4 ? 2 : type == 6 ? 4 : 1;
            depth = image[24] * channels;
        } else if (!isPng && size >= 16) {
            depth = bitmap::read16(image + (bitmap::read32(image) == 12 ? 10 : 14));
        }
        entries.push_back({offset, size, entry[0] ? size_t(entry[0]) : 256,
                           entry[1] ? size_t(entry[1]) : 256, depth, isPng});
    }
    std::stable_sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        const size_t areaA = a.width * a.height, areaB = b.width * b.height;
        if (areaA != areaB) return areaA > areaB;
        if (a.depth != b.depth) return a.depth > b.depth;
        return a.png && !b.png;
    });
    size_t attempted = 0;
    for (const auto& entry : entries) {
        if (entry.size > png::maxBufferBytes - attempted)
            return false;
        attempted += entry.size;
        size_t w = 0, h = 0;
        std::vector<Color> decoded;
        bool success;
        if (entry.png) {
            const uint8_t* image = file.data() + entry.offset;
            if (entry.size < 33 || png::read32(image + 16) != entry.width ||
                png::read32(image + 20) != entry.height)
                continue;
            std::vector<uint8_t> payload(file.data() + entry.offset, file.data() + entry.offset + entry.size);
            success = png::decode(payload, w, h, decoded);
        } else {
            success = bitmap::dib(file.data() + entry.offset, entry.size, 0, w, h, decoded, entry.width, entry.height);
        }
        if (success && w == entry.width && h == entry.height) {
            width = w; height = h;
            pixels.swap(decoded);
            return true;
        }
    }
    return false;
}

}

// Static SVG subset, implemented in TGE. No scripts, network access or external resources.
namespace svg {

inline constexpr double pi = 3.14159265358979323846;
inline constexpr size_t maxNodes = 10000, maxPoints = 200000, maxDepth = 64;
inline constexpr size_t maxWork = 200000000;
struct Invalid {};
inline void require(bool value) { if (!value) throw Invalid{}; }
inline bool space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
inline bool digit(char c) { return c >= '0' && c <= '9'; }
inline std::string_view trim(std::string_view s) {
    while (!s.empty() && space(s.front())) s.remove_prefix(1);
    while (!s.empty() && space(s.back())) s.remove_suffix(1);
    return s;
}
inline double clamp(double x, double lo = 0, double hi = 1) {
    return (std::max)(lo, (std::min)(hi, x));
}

// Locale-independent SVG numbers, including exponents and adjacent signed values.
struct Numbers {
    std::string_view s;
    size_t p = 0;
    explicit Numbers(std::string_view text) : s(text) {}
    void ws() { while (p < s.size() && space(s[p])) ++p; }
    void separators() { ws(); if (p < s.size() && s[p] == ',') { ++p; ws(); } }
    bool more() { ws(); return p < s.size(); }
    double number() {
        separators();
        bool negative = false;
        if (p < s.size() && (s[p] == '+' || s[p] == '-')) negative = s[p++] == '-';
        double v = 0, fraction = .1;
        bool found = false;
        while (p < s.size() && digit(s[p])) { found = true; v = v * 10 + s[p++] - '0'; require(v <= 1e100); }
        if (p < s.size() && s[p] == '.') {
            ++p;
            while (p < s.size() && digit(s[p])) { found = true; v += (s[p++] - '0') * fraction; fraction *= .1; }
        }
        require(found);
        if (p < s.size() && (s[p] == 'e' || s[p] == 'E')) {
            // 'em' and 'ex' are units rather than numeric exponents.
            size_t saved = p++;
            bool minus = false;
            if (p < s.size() && (s[p] == '+' || s[p] == '-')) minus = s[p++] == '-';
            if (p == s.size() || !digit(s[p])) p = saved;
            else {
                int exponent = 0;
                while (p < s.size() && digit(s[p])) { exponent = exponent * 10 + s[p++] - '0'; require(exponent <= 300); }
                v *= std::pow(10.0, minus ? -exponent : exponent);
            }
        }
        require(std::isfinite(v) && v <= 1e12);
        return negative ? -v : v;
    }
    bool flag() {
        separators(); require(p < s.size() && (s[p] == '0' || s[p] == '1'));
        return s[p++] == '1';
    }
};
inline double scalar(std::string_view s) {
    Numbers n(trim(s)); double v = n.number(); require(!n.more()); return v;
}
inline double length(std::string_view s, double basis) {
    Numbers n(trim(s)); double v = n.number(); n.ws(); auto unit = trim(n.s.substr(n.p));
    if (unit == "%") return v * basis / 100;
    if (unit.empty() || unit == "px") return v;
    if (unit == "in") return v * 96;
    if (unit == "cm") return v * 96 / 2.54;
    if (unit == "mm") return v * 96 / 25.4;
    if (unit == "pt") return v * 96 / 72;
    if (unit == "pc") return v * 16;
    if (unit == "q" || unit == "Q") return v * 96 / 101.6;
    throw Invalid{};
}
inline double opacity(std::string_view s) {
    s = trim(s);
    if (!s.empty() && s.back() == '%') return clamp(scalar(s.substr(0, s.size() - 1)) / 100);
    return clamp(scalar(s));
}

struct Point {
    double x = 0, y = 0;
    Point operator+(Point q) const { return {x + q.x, y + q.y}; }
    Point operator-(Point q) const { return {x - q.x, y - q.y}; }
    Point operator*(double t) const { return {x * t, y * t}; }
};
inline double dot(Point p, Point q) { return p.x * q.x + p.y * q.y; }
inline double cross(Point p, Point q) { return p.x * q.y - p.y * q.x; }
inline double norm(Point p) { return std::hypot(p.x, p.y); }
struct Matrix {
    double a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
    Point apply(Point p) const { return {a * p.x + c * p.y + e, b * p.x + d * p.y + f}; }
    Matrix operator*(const Matrix& m) const {
        return {a*m.a+c*m.b, b*m.a+d*m.b, a*m.c+c*m.d, b*m.c+d*m.d,
                a*m.e+c*m.f+e, b*m.e+d*m.f+f};
    }
    bool inverse(Matrix& m) const {
        double det = a*d-b*c;
        if (std::abs(det) < 1e-24 || !std::isfinite(det)) return false;
        m = {d/det, -b/det, -c/det, a/det, (c*f-d*e)/det, (b*e-a*f)/det};
        return true;
    }
    double scale() const { return (std::max)(std::hypot(a,b), std::hypot(c,d)); }
    void validate() const {
        for (double v : {a,b,c,d,e,f}) require(std::isfinite(v) && std::abs(v) <= 1e12);
    }
};
inline Matrix transform(std::string_view s) {
    Numbers n(s); Matrix result;
    while (n.more()) {
        if (n.s[n.p] == ',') { ++n.p; n.ws(); }
        size_t start = n.p;
        while (n.p < s.size() && ((s[n.p] >= 'a' && s[n.p] <= 'z') || (s[n.p] >= 'A' && s[n.p] <= 'Z'))) ++n.p;
        auto name = s.substr(start, n.p - start);
        n.ws(); require(n.p < s.size() && s[n.p++] == '(');
        std::vector<double> v;
        n.ws();
        while (n.p < s.size() && s[n.p] != ')') {
            require(v.size() < 6); v.push_back(n.number()); n.ws();
        }
        require(n.p < s.size() && s[n.p++] == ')'); Matrix m;
        if (name == "matrix") { require(v.size() == 6); m = {v[0],v[1],v[2],v[3],v[4],v[5]}; }
        else if (name == "translate") { require(v.size() == 1 || v.size() == 2); m.e = v[0]; m.f = v.size() == 2 ? v[1] : 0; }
        else if (name == "scale") { require(v.size() == 1 || v.size() == 2); m.a = v[0]; m.d = v.size() == 2 ? v[1] : v[0]; }
        else if (name == "rotate") {
            require(v.size() == 1 || v.size() == 3); double angle = v[0] * pi / 180;
            m.a = m.d = std::cos(angle); m.b = std::sin(angle); m.c = -m.b;
            if (v.size() == 3) { m.e = v[1] - m.a*v[1] - m.c*v[2]; m.f = v[2] - m.b*v[1] - m.d*v[2]; }
        } else if (name == "skewX" || name == "skewY") {
            require(v.size() == 1); double t = std::tan(v[0] * pi / 180);
            if (name == "skewX") m.c = t; else m.b = t;
        } else throw Invalid{};
        result = result * m; result.validate();
    }
    return result;
}

using Attributes = std::vector<std::pair<std::string, std::string>>;
inline std::string_view attr(const Attributes& a, std::string_view key, std::string_view fallback = {}) {
    for (const auto& p : a) if (p.first == key) return p.second;
    return fallback;
}
inline bool has(const Attributes& a, std::string_view key) {
    for (const auto& p : a) if (p.first == key) return true;
    return false;
}
inline void set(Attributes& a, std::string key, std::string value) {
    for (auto& p : a) if (p.first == key) { p.second = std::move(value); return; }
    a.emplace_back(std::move(key), std::move(value));
}
inline Attributes properties(const Attributes& attributes) {
    Attributes result = attributes;
    auto style = attr(attributes, "style");
    while (!style.empty()) {
        size_t end = style.find(';'); auto declaration = trim(style.substr(0, end));
        if (!declaration.empty()) {
            size_t colon = declaration.find(':'); require(colon != std::string_view::npos);
            auto key = trim(declaration.substr(0, colon)), value = trim(declaration.substr(colon + 1));
            require(!key.empty() && !value.empty());
            set(result, std::string(key), std::string(value));
        }
        if (end == std::string_view::npos) break;
        style.remove_prefix(end + 1);
    }
    return result;
}
struct Node { std::string name; Attributes attributes; std::vector<size_t> children; };
struct XML {
    std::string_view s; size_t p = 0; std::vector<Node> nodes;
    explicit XML(std::string_view text) : s(text) {}
    bool at(std::string_view value) const { return s.substr(p, value.size()) == value; }
    void ws() { while (p < s.size() && space(s[p])) ++p; }
    static bool nameChar(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || digit(c) || c == ':' || c == '_' || c == '-' || c == '.';
    }
    std::string name() {
        size_t start = p;
        require(p < s.size() && !digit(s[p]) && nameChar(s[p]));
        while (p < s.size() && nameChar(s[p])) ++p;
        require(p - start <= 256); return std::string(s.substr(start, p - start));
    }
    std::string value() {
        require(p < s.size() && (s[p] == '\'' || s[p] == '"'));
        char quote = s[p++]; std::string out;
        while (p < s.size() && s[p] != quote) {
            require(s[p] != '<');
            if (s[p] != '&') { out += s[p++]; continue; }
            size_t end = s.find(';', ++p); require(end != std::string_view::npos && end - p < 16);
            auto entity = s.substr(p, end - p); p = end + 1;
            if (entity == "amp") out += '&';
            else if (entity == "lt") out += '<';
            else if (entity == "gt") out += '>';
            else if (entity == "quot") out += '"';
            else if (entity == "apos") out += '\'';
            else if (!entity.empty() && entity[0] == '#') {
                entity.remove_prefix(1); unsigned base = 10, code = 0;
                if (!entity.empty() && entity[0] == 'x') { base = 16; entity.remove_prefix(1); }
                require(!entity.empty());
                for (char c : entity) {
                    unsigned v = digit(c) ? unsigned(c - '0') : c >= 'a' && c <= 'f' ? unsigned(c - 'a' + 10) : c >= 'A' && c <= 'F' ? unsigned(c - 'A' + 10) : 99;
                    require(v < base); code = code * base + v; require(code <= 0x10ffff);
                }
                require(code != 0 && (code < 0xd800 || code > 0xdfff));
                if (code < 0x80) out += char(code);
                else if (code < 0x800) { out += char(0xc0 | (code >> 6)); out += char(0x80 | (code & 63)); }
                else if (code < 0x10000) { out += char(0xe0 | (code >> 12)); out += char(0x80 | ((code >> 6) & 63)); out += char(0x80 | (code & 63)); }
                else { out += char(0xf0 | (code >> 18)); out += char(0x80 | ((code >> 12) & 63)); out += char(0x80 | ((code >> 6) & 63)); out += char(0x80 | (code & 63)); }
            } else throw Invalid{};
        }
        require(p < s.size()); ++p; return out;
    }
    bool misc() {
        if (at("<!--")) { size_t end = s.find("-->", p + 4); require(end != std::string_view::npos); p = end + 3; return true; }
        if (at("<?")) { size_t end = s.find("?>", p + 2); require(end != std::string_view::npos); p = end + 2; return true; }
        return false;
    }
    size_t element(size_t depth) {
        require(depth < maxDepth && nodes.size() < maxNodes && p < s.size() && s[p++] == '<');
        std::string fullName = name(); size_t id = nodes.size(); nodes.push_back({fullName, {}, {}});
        for (;;) {
            size_t before = p; ws(); require(p < s.size());
            if (at("/>")) { p += 2; return id; }
            if (s[p] == '>') { ++p; break; }
            require(p > before && nodes[id].attributes.size() < 128);
            std::string key = name(); ws(); require(p < s.size() && s[p++] == '='); ws();
            require(!has(nodes[id].attributes, key));
            nodes[id].attributes.emplace_back(std::move(key), value());
        }
        for (;;) {
            require(p < s.size());
            if (at("</")) { p += 2; require(name() == fullName); ws(); require(p < s.size() && s[p++] == '>'); break; }
            if (misc()) continue;
            if (at("<![CDATA[")) { size_t end = s.find("]]>", p + 9); require(end != std::string_view::npos); p = end + 3; continue; }
            if (s[p] == '<') { size_t child = element(depth + 1); nodes[id].children.push_back(child); }
            else { while (p < s.size() && s[p] != '<') ++p; }
        }
        return id;
    }
    void parse() {
        if (s.substr(0, 3) == "\xef\xbb\xbf") p = 3;
        for (;;) {
            ws(); if (misc()) continue;
            if (at("<!DOCTYPE")) {
                // Accept an external doctype declaration without resolving it. Internal entities are unsupported.
                p += 9; char quote = 0;
                while (p < s.size()) {
                    char c = s[p++];
                    if (quote) { if (c == quote) quote = 0; }
                    else if (c == '\'' || c == '"') quote = c;
                    else { require(c != '['); if (c == '>') break; }
                }
                require(p <= s.size() && p && s[p - 1] == '>'); continue;
            }
            break;
        }
        element(0);
        for (;;) { ws(); if (!misc()) break; }
        require(p == s.size());
        for (auto& node : nodes) {
            size_t colon = node.name.find(':');
            if (colon != std::string::npos) node.name.erase(0, colon + 1);
        }
        require(!nodes.empty() && nodes[0].name == "svg");
    }
};

struct RGBA { double r = 0, g = 0, b = 0, a = 1; };
inline int hex(char c) { return digit(c) ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1; }
inline RGBA color(std::string_view s) {
    s = trim(s); require(!s.empty());
    if (s[0] == '#') {
        s.remove_prefix(1); require(s.size() == 3 || s.size() == 4 || s.size() == 6 || s.size() == 8);
        unsigned c[4] = {0, 0, 0, 255}; size_t channels = s.size() == 3 || s.size() == 6 ? 3 : 4;
        bool shortForm = s.size() <= 4;
        for (size_t i = 0; i < channels; ++i) {
            int a = hex(s[i * (shortForm ? 1 : 2)]); require(a >= 0);
            int b = shortForm ? a : hex(s[i*2+1]); require(b >= 0); c[i] = unsigned(a * 16 + b);
        }
        return {double(c[0]), double(c[1]), double(c[2]), c[3] / 255.0};
    }
    if (s.substr(0,4) == "rgb(" || s.substr(0,5) == "rgba(") {
        bool alpha = s[3] == 'a'; require(s.back() == ')');
        Numbers n(s.substr(alpha ? 5 : 4, s.size() - (alpha ? 6 : 5)));
        double c[4] = {0,0,0,1};
        for (unsigned i = 0; i < (alpha ? 4u : 3u); ++i) {
            c[i] = n.number();
            if (n.p < n.s.size() && n.s[n.p] == '%') { ++n.p; c[i] *= i == 3 ? .01 : 2.55; }
            c[i] = clamp(c[i], 0, i == 3 ? 1 : 255);
        }
        require(!n.more()); return {c[0],c[1],c[2],c[3]};
    }
    if (s == "transparent") return {0,0,0,0};
    // SVG/CSS named colors, including grey/gray aliases.
    static const std::pair<const char*, uint32_t> names[] = {
        {"aliceblue",0xf0f8ff},{"antiquewhite",0xfaebd7},{"aqua",0x00ffff},{"aquamarine",0x7fffd4},{"azure",0xf0ffff},
        {"beige",0xf5f5dc},{"bisque",0xffe4c4},{"black",0},{"blanchedalmond",0xffebcd},{"blue",0x0000ff},{"blueviolet",0x8a2be2},{"brown",0xa52a2a},{"burlywood",0xdeb887},
        {"cadetblue",0x5f9ea0},{"chartreuse",0x7fff00},{"chocolate",0xd2691e},{"coral",0xff7f50},{"cornflowerblue",0x6495ed},{"cornsilk",0xfff8dc},{"crimson",0xdc143c},{"cyan",0x00ffff},
        {"darkblue",0x00008b},{"darkcyan",0x008b8b},{"darkgoldenrod",0xb8860b},{"darkgray",0xa9a9a9},{"darkgreen",0x006400},{"darkgrey",0xa9a9a9},{"darkkhaki",0xbdb76b},{"darkmagenta",0x8b008b},{"darkolivegreen",0x556b2f},{"darkorange",0xff8c00},{"darkorchid",0x9932cc},{"darkred",0x8b0000},{"darksalmon",0xe9967a},{"darkseagreen",0x8fbc8f},{"darkslateblue",0x483d8b},{"darkslategray",0x2f4f4f},{"darkslategrey",0x2f4f4f},{"darkturquoise",0x00ced1},{"darkviolet",0x9400d3},
        {"deeppink",0xff1493},{"deepskyblue",0x00bfff},{"dimgray",0x696969},{"dimgrey",0x696969},{"dodgerblue",0x1e90ff},{"firebrick",0xb22222},{"floralwhite",0xfffaf0},{"forestgreen",0x228b22},{"fuchsia",0xff00ff},
        {"gainsboro",0xdcdcdc},{"ghostwhite",0xf8f8ff},{"gold",0xffd700},{"goldenrod",0xdaa520},{"gray",0x808080},{"green",0x008000},{"greenyellow",0xadff2f},{"grey",0x808080},
        {"honeydew",0xf0fff0},{"hotpink",0xff69b4},{"indianred",0xcd5c5c},{"indigo",0x4b0082},{"ivory",0xfffff0},{"khaki",0xf0e68c},
        {"lavender",0xe6e6fa},{"lavenderblush",0xfff0f5},{"lawngreen",0x7cfc00},{"lemonchiffon",0xfffacd},{"lightblue",0xadd8e6},{"lightcoral",0xf08080},{"lightcyan",0xe0ffff},{"lightgoldenrodyellow",0xfafad2},{"lightgray",0xd3d3d3},{"lightgreen",0x90ee90},{"lightgrey",0xd3d3d3},{"lightpink",0xffb6c1},{"lightsalmon",0xffa07a},{"lightseagreen",0x20b2aa},{"lightskyblue",0x87cefa},{"lightslategray",0x778899},{"lightslategrey",0x778899},{"lightsteelblue",0xb0c4de},{"lightyellow",0xffffe0},
        {"lime",0x00ff00},{"limegreen",0x32cd32},{"linen",0xfaf0e6},{"magenta",0xff00ff},{"maroon",0x800000},{"mediumaquamarine",0x66cdaa},{"mediumblue",0x0000cd},{"mediumorchid",0xba55d3},{"mediumpurple",0x9370db},{"mediumseagreen",0x3cb371},{"mediumslateblue",0x7b68ee},{"mediumspringgreen",0x00fa9a},{"mediumturquoise",0x48d1cc},{"mediumvioletred",0xc71585},{"midnightblue",0x191970},{"mintcream",0xf5fffa},{"mistyrose",0xffe4e1},{"moccasin",0xffe4b5},
        {"navajowhite",0xffdead},{"navy",0x000080},{"oldlace",0xfdf5e6},{"olive",0x808000},{"olivedrab",0x6b8e23},{"orange",0xffa500},{"orangered",0xff4500},{"orchid",0xda70d6},
        {"palegoldenrod",0xeee8aa},{"palegreen",0x98fb98},{"paleturquoise",0xafeeee},{"palevioletred",0xdb7093},{"papayawhip",0xffefd5},{"peachpuff",0xffdab9},{"peru",0xcd853f},{"pink",0xffc0cb},{"plum",0xdda0dd},{"powderblue",0xb0e0e6},{"purple",0x800080},{"rebeccapurple",0x663399},{"red",0xff0000},{"rosybrown",0xbc8f8f},{"royalblue",0x4169e1},
        {"saddlebrown",0x8b4513},{"salmon",0xfa8072},{"sandybrown",0xf4a460},{"seagreen",0x2e8b57},{"seashell",0xfff5ee},{"sienna",0xa0522d},{"silver",0xc0c0c0},{"skyblue",0x87ceeb},{"slateblue",0x6a5acd},{"slategray",0x708090},{"slategrey",0x708090},{"snow",0xfffafa},{"springgreen",0x00ff7f},{"steelblue",0x4682b4},
        {"tan",0xd2b48c},{"teal",0x008080},{"thistle",0xd8bfd8},{"tomato",0xff6347},{"turquoise",0x40e0d0},{"violet",0xee82ee},{"wheat",0xf5deb3},{"white",0xffffff},{"whitesmoke",0xf5f5f5},{"yellow",0xffff00},{"yellowgreen",0x9acd32}
    };
    std::string lower(s);
    for (char& c : lower) if (c >= 'A' && c <= 'Z') c = char(c + 'a' - 'A');
    for (const auto& p : names) if (lower == p.first)
        return {double(p.second >> 16), double((p.second >> 8) & 255), double(p.second & 255), 1};
    throw Invalid{};
}

struct Contour { std::vector<Point> points; bool closed = false; };
using Paths = std::vector<Contour>;
struct Builder {
    Paths paths; size_t& count; double tolerance;
    Builder(size_t& pointCount, double tol) : count(pointCount), tolerance(tol) {}
    void begin(Point p) { paths.push_back({{}, false}); add(p); }
    void add(Point p) {
        require(std::isfinite(p.x) && std::isfinite(p.y) && std::abs(p.x) <= 1e12 && std::abs(p.y) <= 1e12 && ++count <= maxPoints);
        require(!paths.empty()); paths.back().points.push_back(p);
    }
    void cubic(Point a, Point b, Point c, Point d, unsigned depth = 0) {
        Point chord = d-a; double n = dot(chord,chord);
        auto distance = [&](Point p) { Point q = a + chord * (n ? clamp(dot(p-a,chord)/n) : 0); return norm(p-q); };
        if (depth >= 18 || (distance(b) <= tolerance && distance(c) <= tolerance)) { add(d); return; }
        Point ab=(a+b)*.5, bc=(b+c)*.5, cd=(c+d)*.5, abc=(ab+bc)*.5, bcd=(bc+cd)*.5, mid=(abc+bcd)*.5;
        cubic(a,ab,abc,mid,depth+1); cubic(mid,bcd,cd,d,depth+1);
    }
    void arc(Point from, double rx, double ry, double rotation, bool large, bool sweep, Point to) {
        rx = std::abs(rx); ry = std::abs(ry);
        if (norm(to-from) == 0) return;
        if (rx == 0 || ry == 0) { add(to); return; }
        double angle = rotation*pi/180, cs=std::cos(angle), sn=std::sin(angle);
        Point mid=(from-to)*.5; double x=cs*mid.x+sn*mid.y, y=-sn*mid.x+cs*mid.y;
        double ratio = x*x/(rx*rx) + y*y/(ry*ry);
        if (ratio > 1) { double scale = std::sqrt(ratio); rx *= scale; ry *= scale; }
        double divisor = rx*rx*y*y+ry*ry*x*x;
        double k = divisor == 0 ? 0 : std::sqrt((std::max)(0.0, (rx*rx*ry*ry-divisor)/divisor));
        if (large == sweep) k = -k;
        double cx=k*rx*y/ry, cy=-k*ry*x/rx;
        Point center={cs*cx-sn*cy+(from.x+to.x)*.5, sn*cx+cs*cy+(from.y+to.y)*.5};
        Point u={(x-cx)/rx,(y-cy)/ry}, v={(-x-cx)/rx,(-y-cy)/ry};
        double start=std::atan2(u.y,u.x), delta=std::atan2(cross(u,v),dot(u,v));
        if (!sweep && delta > 0) delta -= 2*pi;
        if (sweep && delta < 0) delta += 2*pi;
        double step = (std::min)(pi/8, 2*std::acos(clamp(1-tolerance/(std::max)(rx,ry), -1, 1)));
        require(step > 0 && std::isfinite(step));
        double needed = std::ceil(std::abs(delta)/step); require(needed <= maxPoints);
        size_t segments = (std::max)(size_t(1), static_cast<size_t>(needed));
        for (size_t i=1; i<=segments; ++i) {
            double t=start+delta*double(i)/segments;
            add(i == segments ? to : Point{center.x+cs*rx*std::cos(t)-sn*ry*std::sin(t),center.y+sn*rx*std::cos(t)+cs*ry*std::sin(t)});
        }
    }
};
inline void pathData(std::string_view data, Builder& b) {
    Numbers n(data); Point current{}, start{}, control{}; char command=0, previous=0;
    while (n.more()) {
        char c=n.s[n.p];
        if ((c>='a'&&c<='z') || (c>='A'&&c<='Z')) { command=c; ++n.p; }
        else require(command != 0 && command != 'z' && command != 'Z');
        bool relative=command>='a'&&command<='z'; char upper=relative ? char(command-'a'+'A') : command;
        auto point = [&]() { double x=n.number(), y=n.number(); Point p{x,y}; return relative ? current+p : p; };
        if (upper=='M') {
            current=point(); start=current; b.begin(current); command=relative?'l':'L';
        } else {
            require(!b.paths.empty());
            if (upper!='Z' && b.paths.back().closed) b.begin(current);
            if (upper=='Z') { b.paths.back().closed=true; current=start; }
            else if (upper=='L') { current=point(); b.add(current); }
            else if (upper=='H') { double x=n.number(); current.x=relative?current.x+x:x; b.add(current); }
            else if (upper=='V') { double y=n.number(); current.y=relative?current.y+y:y; b.add(current); }
            else if (upper=='C') { Point a=point(), c2=point(), end=point(); b.cubic(current,a,c2,end); control=c2; current=end; }
            else if (upper=='S') { Point a=(previous=='C'||previous=='S') ? current*2-control : current; Point c2=point(), end=point(); b.cubic(current,a,c2,end); control=c2; current=end; }
            else if (upper=='Q' || upper=='T') {
                Point q=upper=='Q' ? point() : (previous=='Q'||previous=='T') ? current*2-control : current;
                Point end=point(); b.cubic(current,current+(q-current)*(2.0/3),end+(q-end)*(2.0/3),end); control=q; current=end;
            } else if (upper=='A') {
                double rx=n.number(), ry=n.number(), angle=n.number(); bool large=n.flag(), sweep=n.flag(); Point end=point();
                b.arc(current,rx,ry,angle,large,sweep,end); current=end;
            } else throw Invalid{};
        }
        previous=upper;
    }
}

struct Paint { std::string value = "black"; };
struct Style {
    Paint fill, stroke{"none"}; RGBA current;
    double fillOpacity=1, strokeOpacity=1, strokeWidth=1, miterLimit=4, dashOffset=0, ownOpacity=1;
    std::vector<double> dashes;
    std::string cap="butt", join="miter";
    bool evenOdd=false, visible=true;
};
inline void styleFrom(const Attributes& p, Style& s, double diagonal) {
    for (auto key : {"clip-path","mask","filter","marker-start","marker-mid","marker-end"})
        require(!has(p,key) || attr(p,key)=="none");
    require(!has(p,"vector-effect") || attr(p,"vector-effect")=="none");
    auto use = [&](const char* key) { return has(p,key) && attr(p,key)!="inherit"; };
    if (use("fill")) s.fill.value=std::string(trim(attr(p,"fill")));
    if (use("stroke")) s.stroke.value=std::string(trim(attr(p,"stroke")));
    if (use("color") && attr(p,"color")!="currentColor") s.current=color(attr(p,"color"));
    if (use("fill-opacity")) s.fillOpacity=opacity(attr(p,"fill-opacity"));
    if (use("stroke-opacity")) s.strokeOpacity=opacity(attr(p,"stroke-opacity"));
    if (use("stroke-width")) { s.strokeWidth=length(attr(p,"stroke-width"),diagonal); require(s.strokeWidth>=0); }
    if (use("stroke-miterlimit")) { s.miterLimit=scalar(attr(p,"stroke-miterlimit")); require(s.miterLimit>=1); }
    if (use("stroke-linecap")) { s.cap=std::string(attr(p,"stroke-linecap")); require(s.cap=="butt"||s.cap=="round"||s.cap=="square"); }
    if (use("stroke-linejoin")) { s.join=std::string(attr(p,"stroke-linejoin")); require(s.join=="miter"||s.join=="round"||s.join=="bevel"); }
    if (use("fill-rule")) { auto rule=attr(p,"fill-rule"); require(rule=="evenodd"||rule=="nonzero"); s.evenOdd=rule=="evenodd"; }
    if (use("visibility")) { auto v=attr(p,"visibility"); require(v=="hidden"||v=="collapse"||v=="visible"); s.visible=v=="visible"; }
    if (use("stroke-dashoffset")) s.dashOffset=length(attr(p,"stroke-dashoffset"),diagonal);
    if (use("stroke-dasharray")) {
        s.dashes.clear(); auto value=trim(attr(p,"stroke-dasharray"));
        if (value!="none") {
            Numbers n(value);
            while (n.more()) {
                size_t start=n.p; n.number();
                while (n.p<n.s.size() && (n.s[n.p]=='%' || (n.s[n.p]>='a'&&n.s[n.p]<='z'))) ++n.p;
                double d=length(n.s.substr(start,n.p-start),diagonal); require(d>=0 && s.dashes.size()<128); s.dashes.push_back(d);
                n.ws(); if (n.p<n.s.size() && n.s[n.p]==',') ++n.p;
            }
            double total=0; for (double d:s.dashes) total+=d;
            if (total==0) s.dashes.clear();
            else if (s.dashes.size()%2) { auto copy=s.dashes; s.dashes.insert(s.dashes.end(),copy.begin(),copy.end()); }
        }
    }
}

inline Paths geometry(const Node& node, double vw, double vh, double tolerance, size_t& points) {
    const auto& a=node.attributes; Builder b(points,tolerance);
    auto get=[&](const char* key, double basis, double fallback=0) { return has(a,key)?length(attr(a,key),basis):fallback; };
    auto ellipse=[&](double cx, double cy, double rx, double ry) {
        require(rx>=0 && ry>=0); if (rx==0 || ry==0) return;
        b.begin({cx+rx,cy});
        b.arc({cx+rx,cy},rx,ry,0,false,true,{cx-rx,cy});
        b.arc({cx-rx,cy},rx,ry,0,false,true,{cx+rx,cy});
        b.paths.back().closed=true;
    };
    if (node.name=="path") pathData(attr(a,"d"),b);
    else if (node.name=="circle") ellipse(get("cx",vw),get("cy",vh),get("r",std::hypot(vw,vh)/std::sqrt(2.0)),get("r",std::hypot(vw,vh)/std::sqrt(2.0)));
    else if (node.name=="ellipse") ellipse(get("cx",vw),get("cy",vh),get("rx",vw),get("ry",vh));
    else if (node.name=="rect") {
        double x=get("x",vw),y=get("y",vh),w=get("width",vw),h=get("height",vh);
        require(w>=0 && h>=0); if (w==0 || h==0) return {};
        double rx=get("rx",vw,has(a,"ry")?get("ry",vh):0),ry=get("ry",vh,has(a,"rx")?get("rx",vw):0);
        require(rx>=0 && ry>=0); rx=(std::min)(rx,w*.5); ry=(std::min)(ry,h*.5);
        if (rx==0 || ry==0) { b.begin({x,y}); b.add({x+w,y}); b.add({x+w,y+h}); b.add({x,y+h}); }
        else {
            b.begin({x+rx,y}); b.add({x+w-rx,y}); b.arc({x+w-rx,y},rx,ry,0,false,true,{x+w,y+ry});
            b.add({x+w,y+h-ry}); b.arc({x+w,y+h-ry},rx,ry,0,false,true,{x+w-rx,y+h});
            b.add({x+rx,y+h}); b.arc({x+rx,y+h},rx,ry,0,false,true,{x,y+h-ry});
            b.add({x,y+ry}); b.arc({x,y+ry},rx,ry,0,false,true,{x+rx,y});
        }
        b.paths.back().closed=true;
    } else if (node.name=="line") { b.begin({get("x1",vw),get("y1",vh)}); b.add({get("x2",vw),get("y2",vh)}); }
    else if (node.name=="polygon" || node.name=="polyline") {
        Numbers n(attr(a,"points"));
        while (n.more()) { double x=n.number(),y=n.number(); if (b.paths.empty()) b.begin({x,y}); else b.add({x,y}); }
        if (!b.paths.empty()) b.paths.back().closed=node.name=="polygon";
    } else throw Invalid{};
    return std::move(b.paths);
}
inline void polygon(Builder& b, std::vector<Point> p) {
    if (p.size()<3) return;
    double area=0; for (size_t i=0;i<p.size();++i) area+=cross(p[i],p[(i+1)%p.size()]);
    if (area<0) std::reverse(p.begin(),p.end());
    b.begin(p[0]); for (size_t i=1;i<p.size();++i) b.add(p[i]); b.paths.back().closed=true;
}
inline void disk(Builder& b, Point p, double r) {
    double step=(std::min)(pi/8,2*std::acos(clamp(1-b.tolerance/r,-1,1)));
    require(step>0); double count=std::ceil(2*pi/step); require(count<=maxPoints);
    size_t n=(std::max)(size_t(8),static_cast<size_t>(count));
    b.begin({p.x+r,p.y});
    for (size_t i=1;i<n;++i) { double t=2*pi*double(i)/n; b.add({p.x+r*std::cos(t),p.y+r*std::sin(t)}); }
    b.paths.back().closed=true;
}
inline Paths dashed(const Paths& paths, const Style& s, size_t& work) {
    if (s.dashes.empty()) return paths;
    Paths output; double total=0; for (double d:s.dashes) total+=d;
    for (const auto& path:paths) {
        if (path.points.size()<2) continue;
        double offset=std::fmod(s.dashOffset,total); if (offset<0) offset+=total;
        size_t index=0;
        while (s.dashes[index]<=offset) { offset-=s.dashes[index]; index=(index+1)%s.dashes.size(); }
        double remaining=s.dashes[index]-offset;
        size_t first=output.size(); bool active=false;
        size_t segments=path.points.size()-1+(path.closed?1:0);
        for (size_t i=0;i<segments;++i) {
            Point a=path.points[i],b=path.points[(i+1)%path.points.size()]; double distance=norm(b-a);
            if (distance==0) continue;
            double pos=0;
            while (pos<distance) {
                require(++work<=maxWork && output.size()<maxPoints);
                if (remaining<=0) { index=(index+1)%s.dashes.size(); remaining=s.dashes[index]; active=false; continue; }
                double step=(std::min)(remaining,distance-pos); require(step>0 && pos+step>pos);
                Point from=a+(b-a)*(pos/distance),to=a+(b-a)*((pos+step)/distance);
                if (index%2==0) {
                    if (!active) { output.push_back({{from},false}); active=true; }
                    output.back().points.push_back(to);
                }
                pos+=step; remaining-=step;
                if (remaining<=1e-12*total) { index=(index+1)%s.dashes.size(); remaining=s.dashes[index]; active=false; }
            }
        }
        // Join a dash crossing the closing vertex instead of placing two end caps there.
        if (path.closed && output.size()>first && norm(output[first].points.front()-path.points.front())<1e-9 && norm(output.back().points.back()-path.points.front())<1e-9) {
            if (output.size()==first+1) output.back().closed=true;
            else {
                auto tail=std::move(output.back().points); output.pop_back();
                tail.insert(tail.end(),output[first].points.begin()+1,output[first].points.end()); output[first].points=std::move(tail);
            }
        }
    }
    return output;
}
inline Paths stroke(const Paths& paths, const Style& s, double tolerance, size_t& points, size_t& work) {
    Builder b(points,tolerance); double r=s.strokeWidth*.5;
    if (r==0) return {};
    auto lines=dashed(paths,s,work);
    for (const auto& path:lines) {
        std::vector<Point> p;
        for (Point v:path.points) if (p.empty() || norm(v-p.back())>1e-12) p.push_back(v);
        if (path.closed && p.size()>1 && norm(p.front()-p.back())<1e-12) p.pop_back();
        if (p.size()<2) {
            if (p.size()==1 && !path.closed && s.cap=="round") disk(b,p[0],r);
            if (p.size()==1 && !path.closed && s.cap=="square") polygon(b,{{p[0].x-r,p[0].y-r},{p[0].x+r,p[0].y-r},{p[0].x+r,p[0].y+r},{p[0].x-r,p[0].y+r}});
            continue;
        }
        size_t segments=p.size()-1+(path.closed?1:0);
        for (size_t i=0;i<segments;++i) {
            Point a=p[i],end=p[(i+1)%p.size()],d=(end-a)*(1/norm(end-a)),n={-d.y*r,d.x*r};
            if (!path.closed && s.cap=="square") { if (i==0) a=a-d*r; if (i+1==segments) end=end+d*r; }
            polygon(b,{a+n,end+n,end-n,a-n});
        }
        if (!path.closed && s.cap=="round") { disk(b,p.front(),r); disk(b,p.back(),r); }
        size_t start=path.closed?0:1,limit=path.closed?p.size():p.size()-1;
        for (size_t i=start;i<limit;++i) {
            Point center=p[i],u=center-p[(i+p.size()-1)%p.size()],v=p[(i+1)%p.size()]-center;
            u=u*(1/norm(u)); v=v*(1/norm(v)); double turn=cross(u,v);
            if (std::abs(turn)<1e-12) { if (s.join=="round" && dot(u,v)<0) disk(b,center,r); continue; }
            if (s.join=="round") { disk(b,center,r); continue; }
            double side=turn>0?-1:1; Point n1={-u.y*r*side,u.x*r*side},n2={-v.y*r*side,v.x*r*side};
            Point a=center+n1,c=center+n2;
            double t=cross(c-a,v)/turn; Point m=a+u*t;
            if (s.join=="miter" && norm(m-center)<=s.miterLimit*r) polygon(b,{center,a,m,c});
            else polygon(b,{center,a,c});
        }
    }
    return std::move(b.paths);
}

struct Stop { double offset; RGBA color; };
struct Gradient { Attributes params; std::vector<Stop> stops; bool radial=false; };
struct Pixel { uint8_t r=0,g=0,b=0,a=0; };
inline void composite(Pixel& dst, RGBA src, double coverage=1) {
    double sa=clamp(src.a*coverage), da=dst.a/255.0, out=sa+da*(1-sa);
    if (sa==0) return;
    auto byte=[](double v) { return static_cast<uint8_t>(std::lround(clamp(v,0,255))); };
    dst.r=byte((src.r*sa+dst.r*da*(1-sa))/out);
    dst.g=byte((src.g*sa+dst.g*da*(1-sa))/out);
    dst.b=byte((src.b*sa+dst.b*da*(1-sa))/out);
    dst.a=byte(out*255);
}
struct Sampler {
    bool none=false, gradient=false, radial=false; RGBA solid;
    Matrix inverse; std::vector<Stop> stops; std::string spread="pad";
    Point from{},to{1,0},focus{}; double radius=.5;
    RGBA sample(Point p) const {
        if (!gradient) return solid;
        if (stops.empty()) return {0,0,0,0};
        p=inverse.apply(p); double t=1;
        if (!radial) { Point v=to-from; double d=dot(v,v); t=d==0?1:dot(p-from,v)/d; }
        else if (radius>0) {
            Point d=p-focus,f=focus-from; double a=dot(d,d),b=dot(f,d),c=dot(f,f)-radius*radius;
            if (a==0) t=0;
            else { double u=(-b+std::sqrt((std::max)(0.0,b*b-a*c)))/a; t=u>0?1/u:1; }
        }
        if (spread=="repeat") t-=std::floor(t);
        else if (spread=="reflect") { t=std::fmod(t,2); if (t<0) t+=2; if (t>1) t=2-t; }
        else t=clamp(t);
        auto upper=std::upper_bound(stops.begin(),stops.end(),t,[](double value,const Stop& stop) { return value<stop.offset; });
        if (upper==stops.begin()) return stops.front().color;
        size_t i=static_cast<size_t>(upper-stops.begin()-1);
        if (i+1==stops.size()) return stops.back().color;
        double denominator=stops[i+1].offset-stops[i].offset;
        double k=denominator>0?clamp((t-stops[i].offset)/denominator):1; RGBA a=stops[i].color,b=stops[i+1].color;
        return {a.r+(b.r-a.r)*k,a.g+(b.g-a.g)*k,a.b+(b.b-a.b)*k,a.a+(b.a-a.a)*k};
    }
};
struct Bounds { double x0=1e300,y0=1e300,x1=-1e300,y1=-1e300; };
inline Bounds bounds(const Paths& paths) {
    Bounds b;
    for (const auto& path:paths) for (Point p:path.points) {
        b.x0=(std::min)(b.x0,p.x); b.y0=(std::min)(b.y0,p.y); b.x1=(std::max)(b.x1,p.x); b.y1=(std::max)(b.y1,p.y);
    }
    return b;
}
struct Renderer {
    const std::vector<Node>& nodes; std::map<std::string,size_t> ids;
    size_t width,height,points=0,work=0,surfaceBytes=0;
    double vw=0,vh=0;
    Renderer(const std::vector<Node>& tree,size_t w,size_t h) : nodes(tree),width(w),height(h) {
        for (size_t i=0;i<nodes.size();++i) {
            auto id=attr(nodes[i].attributes,"id");
            if (!id.empty()) require(ids.emplace(std::string(id),i).second);
        }
    }
    void spend(size_t amount) { require(amount<=maxWork-work); work+=amount; }
    Gradient gradient(size_t id, const Style& style, size_t depth=0) {
        require(depth<32); const auto& node=nodes[id];
        require(node.name=="linearGradient"||node.name=="radialGradient");
        Gradient g; auto a=properties(node.attributes); auto href=attr(a,"href",attr(a,"xlink:href"));
        if (!href.empty()) {
            require(href[0]=='#'); auto found=ids.find(std::string(href.substr(1))); require(found!=ids.end());
            g=gradient(found->second,style,depth+1);
        }
        g.radial=node.name=="radialGradient";
        for (const auto& p:a) set(g.params,p.first,p.second);
        std::vector<Stop> local;
        RGBA current=has(a,"color")&&attr(a,"color")!="currentColor"?color(attr(a,"color")):style.current;
        for (size_t child:node.children) {
            const auto& stop=nodes[child]; require(stop.name=="stop"||stop.name=="title"||stop.name=="desc");
            if (stop.name!="stop") continue;
            auto p=properties(stop.attributes); double offset=opacity(attr(p,"offset","0"));
            if (!local.empty()) offset=(std::max)(offset,local.back().offset);
            RGBA stopCurrent=has(p,"color")&&attr(p,"color")!="currentColor"?color(attr(p,"color")):current;
            auto value=attr(p,"stop-color","black"); RGBA c=value=="currentColor"?stopCurrent:color(value);
            c.a*=opacity(attr(p,"stop-opacity","1")); local.push_back({offset,c});
        }
        if (!local.empty()) g.stops=std::move(local);
        return g;
    }
    Sampler sampler(const Paint& paint,const Style& style,const Paths& paths,const Matrix& matrix) {
        Sampler s; auto value=trim(paint.value);
        if (value=="none") { s.none=true; return s; }
        if (value.substr(0,4)!="url(") { s.solid=value=="currentColor"?style.current:color(value); return s; }
        require(value.back()==')'); auto ref=trim(value.substr(4,value.size()-5));
        if (ref.size()>=2 && ((ref.front()=='\''&&ref.back()=='\'')||(ref.front()=='"'&&ref.back()=='"'))) ref=ref.substr(1,ref.size()-2);
        require(!ref.empty() && ref[0]=='#'); auto found=ids.find(std::string(ref.substr(1))); require(found!=ids.end());
        auto g=gradient(found->second,style); const auto& a=g.params;
        auto units=attr(a,"gradientUnits","objectBoundingBox"); require(units=="objectBoundingBox"||units=="userSpaceOnUse");
        bool user=units=="userSpaceOnUse"; Matrix map=matrix;
        if (!user) {
            Bounds b=bounds(paths); if (!(b.x1>b.x0 && b.y1>b.y0)) { s.none=true; return s; }
            map=map*Matrix{b.x1-b.x0,0,0,b.y1-b.y0,b.x0,b.y0};
        }
        if (has(a,"gradientTransform")) map=map*transform(attr(a,"gradientTransform"));
        if (!map.inverse(s.inverse)) { s.none=true; return s; }
        double xbasis=user?vw:1,ybasis=user?vh:1,dbasis=user?std::hypot(vw,vh)/std::sqrt(2.0):1;
        auto coord=[&](const char* key,double basis,std::string_view fallback) { return length(attr(a,key,fallback),basis); };
        s.gradient=true; s.radial=g.radial; s.stops=std::move(g.stops); s.spread=std::string(attr(a,"spreadMethod","pad"));
        require(s.spread=="pad"||s.spread=="repeat"||s.spread=="reflect");
        if (!s.radial) { s.from={coord("x1",xbasis,"0%"),coord("y1",ybasis,"0%")}; s.to={coord("x2",xbasis,"100%"),coord("y2",ybasis,"0%")}; }
        else {
            s.from={coord("cx",xbasis,"50%"),coord("cy",ybasis,"50%")}; s.radius=coord("r",dbasis,"50%"); require(s.radius>=0);
            s.focus={coord("fx",xbasis,attr(a,"cx","50%")),coord("fy",ybasis,attr(a,"cy","50%"))};
            Point d=s.focus-s.from; double distance=norm(d);
            if (s.radius>0 && distance>=s.radius) s.focus=s.from+d*(s.radius*(1-1e-7)/distance);
        }
        return s;
    }
    void paint(std::vector<Pixel>& target,Paths paths,const Matrix& matrix,const Sampler& sample,bool evenOdd,double alpha) {
        if (sample.none || alpha==0 || paths.empty()) return;
        size_t edgeCount=0;
        for (auto& path:paths) {
            for (auto& p:path.points) { p=matrix.apply(p); require(std::isfinite(p.x)&&std::isfinite(p.y)&&std::abs(p.x)<=1e15&&std::abs(p.y)<=1e15); }
            if (path.points.size()>1) edgeCount+=path.points.size();
        }
        Bounds b=bounds(paths);
        if (b.x1<=0 || b.y1<=0 || b.x0>=double(width) || b.y0>=double(height) || !edgeCount) return;
        int x0=static_cast<int>(std::floor(clamp(b.x0,0,double(width)))),y0=static_cast<int>(std::floor(clamp(b.y0,0,double(height))));
        int x1=static_cast<int>(std::ceil(clamp(b.x1,0,double(width)))),y1=static_cast<int>(std::ceil(clamp(b.y1,0,double(height))));
        if (x0>=x1 || y0>=y1) return;
        constexpr int aa=4; size_t bw=size_t(x1-x0),bh=size_t(y1-y0);
        require(edgeCount<=maxWork/(bh*aa)); spend(edgeCount*bh*aa); spend(bw*bh*aa*aa);
        std::vector<uint8_t> coverage(bw*bh,0);
        struct Hit { double x; int winding; }; std::vector<Hit> hits; hits.reserve(edgeCount);
        for (int sy=y0*aa;sy<y1*aa;++sy) {
            double y=(sy+.5)/aa; hits.clear();
            for (const auto& path:paths) {
                if (path.points.size()<2) continue;
                for (size_t i=0;i<path.points.size();++i) {
                    Point a=path.points[i],end=path.points[(i+1)%path.points.size()];
                    if ((a.y<=y&&y<end.y)||(end.y<=y&&y<a.y)) hits.push_back({a.x+(y-a.y)*(end.x-a.x)/(end.y-a.y),end.y>a.y?1:-1});
                }
            }
            std::sort(hits.begin(),hits.end(),[](const Hit& a,const Hit& c) { return a.x<c.x; });
            int winding=0; double left=0;
            for (const auto& hit:hits) {
                bool before=evenOdd?(winding%2!=0):(winding!=0); winding+=hit.winding;
                bool after=evenOdd?(winding%2!=0):(winding!=0);
                if (!before && after) left=hit.x;
                if (before && !after) {
                    double lo=clamp(left*aa-.5,double(x0*aa),double(x1*aa)),hi=clamp(hit.x*aa-.5,double(x0*aa),double(x1*aa));
                    int first=static_cast<int>(std::ceil(lo)),last=static_cast<int>(std::ceil(hi));
                    size_t row=size_t(sy/aa-y0)*bw;
                    for (int sx=first;sx<last;++sx) ++coverage[row+size_t(sx/aa-x0)];
                }
            }
        }
        for (int y=y0;y<y1;++y) for (int x=x0;x<x1;++x) {
            uint8_t c=coverage[size_t(y-y0)*bw+size_t(x-x0)];
            if (c) composite(target[size_t(y)*width+size_t(x)],sample.sample({x+.5,y+.5}),alpha*c/(aa*aa));
        }
    }
    void render(size_t id,Style inherited,Matrix parent,std::vector<Pixel>& target,size_t depth=0) {
        require(depth<maxDepth); spend(1); const auto& node=nodes[id]; const auto& name=node.name;
        if (name=="defs"||name=="linearGradient"||name=="radialGradient"||name=="title"||name=="desc"||name=="metadata") return;
        auto p=properties(node.attributes);
        if (attr(p,"display")=="none") return;
        styleFrom(p,inherited,std::hypot(vw,vh)/std::sqrt(2.0));
        Matrix matrix=parent;
        if (has(p,"transform")) matrix=matrix*transform(attr(p,"transform"));
        matrix.validate();
        Matrix inverse; if (!matrix.inverse(inverse)) return;
        auto opacityValue=attr(p,"opacity","1");
        double alpha=opacityValue=="inherit"?inherited.ownOpacity:opacity(opacityValue);
        inherited.ownOpacity=alpha;
        if (alpha==0) return;
        std::vector<Pixel> layer; std::vector<Pixel>* destination=&target;
        size_t bytes=width*height*sizeof(Pixel);
        if (alpha<1) { require(bytes<=png::maxBufferBytes-surfaceBytes); surfaceBytes+=bytes; layer.resize(width*height); destination=&layer; }
        if (name=="g"||name=="a"||(name=="svg"&&id==0)) {
            for (size_t child:node.children) render(child,inherited,matrix,*destination,depth+1);
        } else if (name=="use") {
            auto href=attr(p,"href",attr(p,"xlink:href")); require(href.size()>1&&href[0]=='#');
            auto found=ids.find(std::string(href.substr(1))); require(found!=ids.end());
            matrix=matrix*Matrix{1,0,0,1,length(attr(p,"x","0"),vw),length(attr(p,"y","0"),vh)};
            render(found->second,inherited,matrix,*destination,depth+1);
        } else if (inherited.visible) {
            double tolerance=.15/(std::max)(1e-12,matrix.scale());
            Paths paths=geometry(node,vw,vh,tolerance,points);
            auto fill=sampler(inherited.fill,inherited,paths,matrix);
            paint(*destination,paths,matrix,fill,inherited.evenOdd,inherited.fillOpacity);
            if (inherited.strokeWidth>0 && inherited.stroke.value!="none") {
                auto outline=sampler(inherited.stroke,inherited,paths,matrix);
                auto strokes=stroke(paths,inherited,tolerance,points,work);
                paint(*destination,std::move(strokes),matrix,outline,false,inherited.strokeOpacity);
            }
        }
        if (alpha<1) {
            spend(width*height);
            for (size_t i=0;i<target.size();++i) { Pixel c=layer[i]; if (c.a) composite(target[i],{double(c.r),double(c.g),double(c.b),c.a/255.0},alpha); }
            surfaceBytes-=bytes;
        }
    }
};

inline bool looksLike(const std::vector<uint8_t>& file) {
    size_t p=0;
    if (file.size()>=3&&file[0]==0xef&&file[1]==0xbb&&file[2]==0xbf) p=3;
    while (p<file.size()&&space(char(file[p]))) ++p;
    return p<file.size()&&file[p]=='<';
}
inline bool decode(const std::vector<uint8_t>& file,size_t& width,size_t& height,std::vector<Color>& pixels,size_t requestedWidth=0,size_t requestedHeight=0) {
    try {
        require(!file.empty()&&file.size()<=size_t(16)*1024*1024);
        std::string_view source(reinterpret_cast<const char*>(file.data()),file.size());
        require(source.find('\0')==std::string_view::npos); XML xml(source); xml.parse();
        const auto& a=xml.nodes[0].attributes;
        double bx=0,by=0,bw=0,bh=0;
        bool viewBox=has(a,"viewBox");
        if (viewBox) { Numbers n(attr(a,"viewBox")); bx=n.number(); by=n.number(); bw=n.number(); bh=n.number(); require(!n.more()&&bw>0&&bh>0); }
        double intrinsicWidth=has(a,"width")?length(attr(a,"width"),viewBox?bw:300):(viewBox?bw:300);
        double intrinsicHeight=has(a,"height")?length(attr(a,"height"),viewBox?bh:150):(viewBox?bh:150);
        require(intrinsicWidth>0&&intrinsicHeight>0&&std::isfinite(intrinsicWidth)&&std::isfinite(intrinsicHeight));
        double w=requestedWidth?double(requestedWidth):std::ceil(intrinsicWidth),h=requestedHeight?double(requestedHeight):std::ceil(intrinsicHeight);
        require(w>0&&h>0&&w<=double(png::maxBufferBytes/4)&&h<=double(png::maxBufferBytes/4));
        size_t iw=static_cast<size_t>(w),ih=static_cast<size_t>(h);
        require(iw<=png::maxBufferBytes/4/ih&&iw<=size_t((std::numeric_limits<int>::max)()/4)&&ih<=size_t((std::numeric_limits<int>::max)()/4));
        if (!viewBox) { bw=intrinsicWidth; bh=intrinsicHeight; }
        double sx=w/bw,sy=h/bh,tx=-bx*sx,ty=-by*sy;
        auto preserve=trim(attr(a,"preserveAspectRatio","xMidYMid meet"));
        if (preserve.substr(0,6)=="defer ") preserve=trim(preserve.substr(6));
        if (preserve!="none") {
            size_t split=preserve.find_first_of(" \t\r\n"); auto align=preserve.substr(0,split);
            auto mode=split==std::string_view::npos?std::string_view("meet"):trim(preserve.substr(split+1));
            require(mode=="meet"||mode=="slice"); require(align.size()==8&&align.substr(0,1)=="x"&&align.substr(4,1)=="Y");
            auto ax=align.substr(1,3),ay=align.substr(5,3);
            require((ax=="Min"||ax=="Mid"||ax=="Max")&&(ay=="Min"||ay=="Mid"||ay=="Max"));
            sx=sy=mode=="slice"?(std::max)(sx,sy):(std::min)(sx,sy);
            tx=-bx*sx+(w-bw*sx)*(ax=="Min"?0:ax=="Mid"?.5:1);
            ty=-by*sy+(h-bh*sy)*(ay=="Min"?0:ay=="Mid"?.5:1);
        }
        Renderer renderer(xml.nodes,iw,ih); renderer.vw=bw; renderer.vh=bh;
        renderer.surfaceBytes=iw*ih*sizeof(Pixel);
        std::vector<Pixel> target(iw*ih); renderer.render(0,Style{},Matrix{sx,0,0,sy,tx,ty},target);
        std::vector<Color> result; result.reserve(target.size());
        for (const auto& c:target) result.emplace_back(c.r,c.g,c.b,c.a);
        width=iw; height=ih; pixels.swap(result); return true;
    } catch (const Invalid&) { return false; }
}

} // namespace svg


inline bool decodeImage(const std::vector<uint8_t>& file, size_t& width, size_t& height,
                        std::vector<Color>& pixels, size_t renderWidth = 0, size_t renderHeight = 0) {
    if (file.size() > png::maxBufferBytes || (renderWidth == 0) != (renderHeight == 0))
        return false;
    if (renderWidth && (renderHeight > png::maxBufferBytes / 4 ||
                       renderWidth > png::maxBufferBytes / 4 / renderHeight))
        return false;
    if (svg::looksLike(file))
        return svg::decode(file, width, height, pixels, renderWidth, renderHeight);
    size_t w = 0, h = 0;
    std::vector<Color> decoded;
    bool success = false;
    if (file.size() >= 8 && png::hasSignature(file.data()))
        success = png::decode(file, w, h, decoded);
    else if (file.size() >= 2 && file[0] == 'B' && file[1] == 'M')
        success = bitmap::decode(file, w, h, decoded);
    else if (file.size() >= 4 && ico::hasSignature(file.data()))
        success = ico::decode(file, w, h, decoded);
    if (!success)
        return false;
    if (renderWidth && (renderWidth != w || renderHeight != h)) {
        std::vector<Color> resized(renderWidth * renderHeight);
        const size_t dx = renderWidth > 1 ? renderWidth - 1 : 1;
        const size_t dy = renderHeight > 1 ? renderHeight - 1 : 1;
        for (size_t y = 0; y < renderHeight; ++y)
            for (size_t x = 0; x < renderWidth; ++x)
                resized[y * renderWidth + x] = decoded[(y * (h - 1) / dy) * w + x * (w - 1) / dx];
        decoded.swap(resized);
        w = renderWidth; h = renderHeight;
    }
    width = w; height = h; pixels.swap(decoded);
    return true;
}

inline bool loadImage(std::ifstream& stream, size_t& width, size_t& height,
                      std::vector<Color>& pixels, size_t renderWidth = 0, size_t renderHeight = 0) {
    stream.seekg(0, std::ios::end);
    const std::streamoff length = stream.tellg();
    if (length < 8 || uint64_t(length) > png::maxBufferBytes)
        return false;
    stream.seekg(0, std::ios::beg);
    std::vector<uint8_t> file(static_cast<size_t>(length));
    if (!stream.read(reinterpret_cast<char*>(file.data()), static_cast<std::streamsize>(file.size())))
        return false;
    return decodeImage(file, width, height, pixels, renderWidth, renderHeight);
}

}



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

    static size_t positiveDimension(int dimension) {
        if (dimension <= 0)
            throw std::invalid_argument("Error: screen size must be > 0");
        return static_cast<size_t>(dimension);
    }

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

        x0 = (std::max)(0, x0);
        y0 = (std::max)(0, y0);
        x1 = (std::min)((int)_width, x1);
        y1 = (std::min)((int)_height, y1);

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

    Canvas(std::pair<int, int> size)
        : Canvas(positiveDimension(size.first), positiveDimension(size.second)) { }

    Canvas(std::pair<int, int> size, std::string title)
        : Canvas(positiveDimension(size.first), positiveDimension(size.second), std::move(title)) { }

    Canvas(std::pair<int, int> size, const std::vector<Color>& pixels)
        : Canvas(positiveDimension(size.first), positiveDimension(size.second), pixels) { }

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
    
    // SVG is rasterized at this viewport size; raster formats use nearest-neighbor scaling.
    Canvas(const std::string& file_name, int width, int height) : Canvas() {
        if (!load(file_name, width, height))
            throw std::runtime_error("Error: Unable to load Canvas image: " + file_name);
    }

    bool load(const std::string& file_name) {
        return loadImageFile(file_name, 0, 0);
    }

    bool load(const std::string& file_name, int width, int height) {
        const size_t w = positiveDimension(width), h = positiveDimension(height);
        return loadImageFile(file_name, w, h);
    }

private:
    bool loadImageFile(const std::string& file_name, size_t renderWidth, size_t renderHeight) {
        std::ifstream fin(file_name, std::ios::binary);
        if (!fin)
            return false;
        size_t width = 0, height = 0;
        std::vector<Color> loadedPixels;
        if (!detail::loadImage(fin, width, height, loadedPixels, renderWidth, renderHeight))
            return false;
        _width = width;
        _height = height;
        _pixels.swap(loadedPixels);
        _initialized = true;
        setImageMode(true);
        return true;
    }

public:

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
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            throw std::runtime_error("Error: Unable to query terminal size");

        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        struct winsize w{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != 0)
            throw std::runtime_error("Error: Unable to query terminal size");

        width = w.ws_col;
        height = w.ws_row;
#endif

        if (width <= 0 || height <= 0)
            throw std::runtime_error("Error: Terminal size must be > 0");

        return { width, height*2 };
    }

    bool resize(int width, int height) {
        if (width <= 0 || height <= 0)
            throw std::invalid_argument("Error: screen size must be > 0");

        const size_t w = static_cast<size_t>(width);
        const size_t h = static_cast<size_t>(height);

        if (_initialized && _width == w && _height == h)
            return false;

        _width = w;
        _height = h;
        _pixels.assign(w * h, CONSOLE_BG_COLOR);
        _initialized = true;

        if (_temp_pixels.empty()) {
            std::vector<Color>().swap(_temp_pixels);
        } else {
            _temp_pixels.assign(w * h, Color(0, 0, 0, 0));
        }

        std::vector<Color>().swap(_old_pixels);

        if (terminalActive()) {
            const std::string clear = "\033[3J\033[1;1H";
            writeAll(clear.data(), clear.size());
        }

        return true;
    }

    bool resize(std::pair<int, int> size) {
        auto [width, height] = size;
        return resize(width, height);
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
        int minX = (std::max)(0, (std::min)(x0, x1) - thickness);
        int maxX = (std::min)((int)_width - 1, (std::max)(x0, x1) + thickness);
        int minY = (std::max)(0, (std::min)(y0, y1) - thickness);
        int maxY = (std::min)((int)_height - 1, (std::max)(y0, y1) + thickness);

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

        int x0 = (std::max)(0, x);
        int y0 = (std::max)(0, y);
        int x1 = (std::min)((int)_width, x + w);
        int y1 = (std::min)((int)_height, y + h);

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
            int minX = (std::max)(0, (int)std::floor((std::min)({ ax, bx, cx })));
            int maxX = (std::min)((int)_width - 1, (int)std::ceil((std::max)({ ax, bx, cx })));
            int minY = (std::max)(0, (int)std::floor((std::min)({ ay, by, cy })));
            int maxY = (std::min)((int)_height - 1, (int)std::ceil((std::max)({ ay, by, cy })));

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

        int bx0 = (std::min)((std::min)(x0, x1), (std::min)(x2, x3));
        int bx1 = (std::max)((std::max)(x0, x1), (std::max)(x2, x3));
        int by0 = (std::min)((std::min)(y0, y1), (std::min)(y2, y3));
        int by1 = (std::max)((std::max)(y0, y1), (std::max)(y2, y3));

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

using Img = Canvas;

class Font{
    inline static Color t = Color(0,0,0,0);

    static Img basic_a(Color c){
        return Img(6,8,{
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

    static Img basic_a_grave(Color c){
        return Img(6,8,{
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

    static Img basic_b(Color c){
        return Img(6,8,{
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

    static Img basic_c(Color c){
        return Img(6,8,{
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

    static Img basic_d(Color c){
        return Img(6,8,{
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

    static Img basic_e(Color c){
        return Img(6,8,{
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

    static Img basic_e_grave(Color c){
        return Img(6,8,{
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

    static Img basic_e_acute(Color c){
        return Img(6,8,{
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

    static Img basic_f(Color c){
        return Img(5,8,{
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

    static Img basic_g(Color c){
        return Img(6,9,{
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

    static Img basic_h(Color c){
        return Img(6,8,{
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

    static Img basic_i(Color c){
        return Img(2,8,{
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

    static Img basic_i_grave(Color c){
        return Img(3,8,{
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

    static Img basic_j(Color c){
        return Img(5,9,{
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

    static Img basic_k(Color c){
        return Img(5,8,{
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

    static Img basic_l(Color c){
        return Img(2,8,{
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

    static Img basic_m(Color c){
        return Img(8,8,{
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

    static Img basic_n(Color c){
        return Img(6,8,{
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

    static Img basic_o(Color c){
        return Img(6,8,{
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

    static Img basic_o_grave(Color c){
        return Img(6,8,{
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


    static Img basic_p(Color c){
        return Img(6,10,{
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

    static Img basic_q(Color c){
        return Img(6,10,{
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

    static Img basic_r(Color c){
        return Img(5,8,{
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

    static Img basic_s(Color c){
        return Img(6,8,{
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

    static Img basic_t(Color c){
        return Img(4,8,{
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

    static Img basic_u(Color c){
        return Img(6,8,{
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

    static Img basic_u_grave(Color c){
        return Img(6,8,{
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

    static Img basic_v(Color c){
        return Img(6,8,{
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

    static Img basic_w(Color c){
        return Img(8,8,{
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

    static Img basic_x(Color c){
        return Img(6,8,{
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

    static Img basic_y(Color c){
        return Img(5,10,{
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

    static Img basic_z(Color c){
        return Img(5,8,{
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


    static Img basic_A(Color c){
        return Img(6,8,{
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

    static Img basic_B(Color c){
        return Img(6,8,{
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

    static Img basic_C(Color c){
        return Img(6,8,{
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

    static Img basic_D(Color c){
        return Img(6,8,{
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

    static Img basic_E(Color c){
        return Img(6,8,{
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

    static Img basic_F(Color c){
        return Img(6,8,{
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

    static Img basic_G(Color c){
        return Img(6,8,{
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

    static Img basic_H(Color c){
        return Img(6,8,{
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

    static Img basic_I(Color c){
        return Img(4,8,{
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

    static Img basic_J(Color c){
        return Img(6,8,{
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

    static Img basic_K(Color c){
        return Img(5,8,{
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

    static Img basic_L(Color c){
        return Img(4,8,{
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

    static Img basic_M(Color c){
        return Img(8,8,{
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

    static Img basic_N(Color c){
        return Img(7,8,{
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

    static Img basic_O(Color c){
        return Img(6,8,{
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

    static Img basic_P(Color c){
        return Img(6,8,{
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

    static Img basic_Q(Color c){
        return Img(7,8,{
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

    static Img basic_R(Color c){
        return Img(6,8,{
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

    static Img basic_S(Color c){
        return Img(6,8,{
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

    static Img basic_T(Color c){
        return Img(6,8,{
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

    static Img basic_U(Color c){
        return Img(6,8,{
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

    static Img basic_V(Color c){
        return Img(6,8,{
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

    static Img basic_W(Color c){
        return Img(10,8,{
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

    static Img basic_X(Color c){
        return Img(6,8,{
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

    static Img basic_Y(Color c){
        return Img(6,8,{
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

    static Img basic_Z(Color c){
        return Img(7,8,{
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



    static Img basic_0(Color c){
        return Img(6,8,{
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

    static Img basic_1(Color c){
        return Img(6,8,{
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

    static Img basic_2(Color c){
        return Img(6,8,{
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

    static Img basic_3(Color c){
        return Img(6,8,{
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

    static Img basic_4(Color c){
        return Img(6,8,{
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

    static Img basic_5(Color c){
        return Img(6,8,{
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

    static Img basic_6(Color c){
        return Img(6,8,{
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

    static Img basic_7(Color c){
        return Img(6,8,{
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

    static Img basic_8(Color c){
        return Img(6,8,{
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

    static Img basic_9(Color c){
        return Img(6,8,{
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


    static Img basic_space(){
        return Img(4,1,{
            t,  t,  t,  t
        });
    }

    static Img basic_period(Color c){
        return Img(2,9,{
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

    static Img basic_comma(Color c){
        return Img(2,9,{
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

    static Img basic_exclamation_mark(Color c){
        return Img(2,9,{
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

    static Img basic_question_mark(Color c){
        return Img(6,9,{
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

    static Img basic_colon(Color c){
        return Img(2,8,{
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

    static Img basic_semicolon(Color c){
        return Img(2,9,{
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

    static Img basic_apostrophe(Color c){
        return Img(3,3,{
            c, t, t,
            c, t, t,
            t, c, t
        });
    }

    static Img basic_quotation_marks(Color c){
        return Img(4,2,{
            c, t, c, t,
            c, t, c, t,
        });
    }

    static Img basic_minus_sign(Color c){
        return Img(6,5,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            c, c, c, c, c, t

        });
    }

    static Img basic_plus_sign(Color c){
        return Img(6,7,{
            t, t, t, t, t, t,
            t, t, t, t, t, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t,
            c, c, c, c, c, t,
            t, t, c, t, t, t,
            t, t, c, t, t, t

        });
    }

    static Img basic_asterisk(Color c){
        return Img(4,3,{
            c, t, c, t,
            t, c, t, t,
            c, t, c, t
        });
    }

    static Img basic_slash(Color c){
        return Img(5,8,{
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

    static Img basic_backslash(Color c){
        return Img(5,8,{
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

    static Img basic_pipe(Color c){
        return Img(2,8,{
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

    static Img basic_left_parenthesis(Color c){
        return Img(4,8,{
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

    static Img basic_right_parenthesis(Color c){
        return Img(4,8,{
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

    static Img basic_left_square_bracket(Color c){
        return Img(4,8,{
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

    static Img basic_right_square_bracket(Color c){
        return Img(4,8,{
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

    static Img basic_left_curly_brace(Color c){
        return Img(3,8,{
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

    static Img basic_right_curly_brace(Color c){
        return Img(3,8,{
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

    static Img basic_equals_sign(Color c){
        return Img(5,6,{
            t, t, t, t, t, 
            t, t, t, t, t, 
            c, c, c, c, t, 
            t, t, t, t, t, 
            t, t, t, t, t, 
            c, c, c, c, t
        });
    }

    static Img basic_ampersand(Color c){
        return Img(5,8,{
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

    static Img basic_percent_sign(Color c){
        return Img(7,8,{
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

    static Img basic_pound_sign(Color c){
        return Img(6,9,{
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

    static Img basic_dollar_sign(Color c){
        return Img(6,9,{
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

    static Img basic_euro_sign(Color c){
        return Img(6,9,{
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

    static Img basic_underscore(Color c){
        return Img(6,9,{
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

    static Img basic_hash(Color c){
        return Img(6,8,{
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

    static Img basic_at_sign(Color c){
        return Img(7,8,{
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


    static Img basic_greater_sign(Color c){
        return Img(5,8,{
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

    static Img basic_less_sign(Color c){
        return Img(5,8,{
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

    static Img basic_caret(Color c){
        return Img(6,3,{
            t, t, c, t, t, t,
            t, c, t, c, t, t,
            c, t, t, t, c, t
        });
    }

public:

    static Img basic_char(wchar_t ch, Color c){
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
        Img glyph = Font::basic_char(ch, c);
        drawImg(glyph, x, y);
        x += static_cast<int>(glyph.getWidth());
    }
}

}

#endif
