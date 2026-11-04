#ifndef TGE_HPP

#define TGE_HPP

// TGE 3.0

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

// Self-contained static PNG decoder. No system codecs or third-party code.
// Format references: https://www.w3.org/TR/png-3/ and RFC 1950 / RFC 1951.
// Bound each of the input, inflated scanlines and final pixel buffers to 256 MiB.
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
        // Callers request at most 16 bits; at most 7 remain after a read.
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
                return false; // Oversubscribed tree.
        }
        // Incomplete trees are valid only for a single one-bit data symbol.
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
            // Huffman codes arrive most-significant bit first.
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
                // Copy byte by byte: an LZ77 match may overlap its own output.
                for (unsigned i = 0; i < length; ++i)
                    output.push_back(output[output.size() - distance]);
            }
        }
    } while (!finalBlock);

    input.align();
    if (output.size() != expected || input.position() > compressed.size() - 6)
        return false;
    // PNG permits unused bytes after the zlib stream in the final IDAT.
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
        if (name == 0x49484452u) { // IHDR
            if (haveHeader || position != 8 || length != 13 || !readHeader(data, header))
                return false;
            haveHeader = true;
        } else if (name == 0x504c5445u) { // PLTE
            if (havePalette || haveTransparency || haveData || !length || length % 3 ||
                length > 768 || header.type == 0 || header.type == 4)
                return false;
            header.paletteSize = length / 3;
            if (header.type == 3 && header.paletteSize > (size_t(1) << header.depth))
                return false;
            for (size_t i = 0; i < header.paletteSize; ++i)
                header.palette[i] = {data[i * 3], data[i * 3 + 1], data[i * 3 + 2], 255};
            havePalette = true;
        } else if (name == 0x74524e53u) { // tRNS
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
        } else if (name == 0x49444154u) { // IDAT
            if (dataEnded || (header.type == 3 && !havePalette) ||
                length > maxBufferBytes - compressed.size())
                return false;
            compressed.insert(compressed.end(), data, data + length);
            haveData = true;
        } else if (name == 0x49454e44u) { // IEND
            if (length || !haveData || position + 12 != file.size())
                return false;
            haveEnd = true;
            break;
        } else if (!(type[0] & 32u)) {
            return false; // Unknown critical chunk.
        }
        // Ignore ancillary metadata and animation chunks (use the static image).
        position += length + 12;
    }
    if (!haveEnd || !reconstruct(header, compressed, pixels))
        return false;
    width = header.width;
    height = header.height;
    return true;
}

} // namespace png

namespace bitmap {

// Windows/OS2 core BMPs, Windows DIBs and the bitmap payload of ICO files.
// See Microsoft's Bitmap Storage, Bitmap Compression and BITMAPV4HEADER docs.
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
        return !(value & (value + 1u)); // A channel mask must be contiguous.
    }

    unsigned get(uint32_t value) const {
        return bits ? unsigned((uint64_t((value & bits) >> shift) * 255 + maximum / 2) / maximum) : 255;
    }
};

inline bool rle(const uint8_t* data, size_t size, size_t width, size_t height,
                unsigned depth, const std::array<Color, 256>& palette, size_t colors,
                std::vector<Color>& pixels) {
    pixels.assign(width * height, palette[0]);
    // Some encoders include DWORD scanline padding in the RLE stream. Consume
    // those values, but never write them into this or the following image row.
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
    return false; // An RLE bitmap must have an end-of-bitmap marker.
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
        return false; // Embedded JPEG/PNG and OS/2-specific compression are unsupported.
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
    // Many 32-bit icons omit the AND mask when real alpha is present. A partial
    // mask is still invalid. All-zero alpha instead uses the legacy AND mask.
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
                // Palette entries are BGR; their reserved byte is not alpha.
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
                // Screen-inverting monochrome pixels cannot be represented by RGBA.
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

} // namespace bitmap

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
    // Prefer the largest valid image, then higher depth, then PNG. Keep file
    // order for equivalent entries. Unsupported/corrupt payloads may fall back.
    std::stable_sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        const size_t areaA = a.width * a.height, areaB = b.width * b.height;
        if (areaA != areaB) return areaA > areaB;
        if (a.depth != b.depth) return a.depth > b.depth;
        return a.png && !b.png;
    });
    size_t attempted = 0;
    for (const auto& entry : entries) {
        // Bound repeated work even if directory entries share/overlap payloads.
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
                continue; // Reject mismatched dimensions before PNG allocation.
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

} // namespace ico

inline bool decodeImage(const std::vector<uint8_t>& file, size_t& width, size_t& height,
                        std::vector<Color>& pixels) {
    if (file.size() > png::maxBufferBytes)
        return false;
    if (file.size() >= 8 && png::hasSignature(file.data()))
        return png::decode(file, width, height, pixels);
    if (file.size() >= 2 && file[0] == 'B' && file[1] == 'M')
        return bitmap::decode(file, width, height, pixels);
    if (file.size() >= 4 && ico::hasSignature(file.data()))
        return ico::decode(file, width, height, pixels);
    return false;
}

inline bool loadImage(std::ifstream& stream, size_t& width, size_t& height,
                      std::vector<Color>& pixels) {
    stream.seekg(0, std::ios::end);
    const std::streamoff length = stream.tellg();
    if (length < 8 || uint64_t(length) > png::maxBufferBytes)
        return false;
    stream.seekg(0, std::ios::beg);
    std::vector<uint8_t> file(static_cast<size_t>(length));
    if (!stream.read(reinterpret_cast<char*>(file.data()), static_cast<std::streamsize>(file.size())))
        return false;
    return decodeImage(file, width, height, pixels);
}

} // namespace detail



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

        size_t width = 0, height = 0;
        std::vector<Color> loadedPixels;
        if (!detail::loadImage(fin, width, height, loadedPixels))
            return false;

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

} // namespace tge

#endif
