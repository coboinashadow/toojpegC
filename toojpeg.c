// toojpeg.c - converted from C++ to pure C
// written by Stephan Brumme, 2018-2019 see https://create.stephan-brumme.com/toojpeg/
// Converted to pure C by Copilot

#include "toojpeg.h"
#include <string.h>
#include <math.h>

// quantization tables from JPEG Standard, Annex K
static unsigned char DefaultQuantLuminance[8*8] =
    { 16, 11, 10, 16, 24, 40, 51, 61,
      12, 12, 14, 19, 26, 58, 60, 55,
      14, 13, 16, 24, 40, 57, 69, 56,
      14, 17, 22, 29, 51, 87, 80, 62,
      18, 22, 37, 56, 68,109,103, 77,
      24, 35, 55, 64, 81,104,113, 92,
      49, 64, 78, 87,103,121,120,101,
      72, 92, 95, 98,112,100,103, 99 };

static unsigned char DefaultQuantChrominance[8*8] =
    { 17, 18, 24, 47, 99, 99, 99, 99,
      18, 21, 26, 66, 99, 99, 99, 99,
      24, 26, 56, 99, 99, 99, 99, 99,
      47, 66, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99 };

static unsigned char ZigZagInv[8*8] =
    {  0, 1, 8,16, 9, 2, 3,10,
      17,24,32,25,18,11, 4, 5,
      12,19,26,33,40,48,41,34,
      27,20,13, 6, 7,14,21,28,
      35,42,49,56,57,50,43,36,
      29,22,15,23,30,37,44,51,
      58,59,52,45,38,31,39,46,
      53,60,61,54,47,55,62,63 };

// Huffman tables
static unsigned char DcLuminanceCodesPerBitsize[16]   = { 0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0 };
static unsigned char DcLuminanceValues         [12]   = { 0,1,2,3,4,5,6,7,8,9,10,11 };
static unsigned char AcLuminanceCodesPerBitsize[16]   = { 0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125 };
static unsigned char AcLuminanceValues        [162]   =
    { 0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,
      0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,
      0x29,0x2A,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
      0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
      0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,
      0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,
      0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA };

static unsigned char DcChrominanceCodesPerBitsize[16] = { 0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };
static unsigned char DcChrominanceValues         [12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
static unsigned char AcChrominanceCodesPerBitsize[16] = { 0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119 };
static unsigned char AcChrominanceValues        [162] =
    { 0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,
      0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,
      0x27,0x28,0x29,0x2A,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,
      0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,
      0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,
      0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,
      0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA };

static short CodeWordLimit = 2048;

// Structs
typedef struct {
    unsigned short code;
    unsigned char numBits;
} BitCode;

typedef struct {
    int data;
    unsigned char numBits;
} BitBuffer;

typedef struct {
    BitBuffer buffer;
    WRITE_ONE_BYTE output;
} BitWriter;

// Helper functions
static float minimum(float value, float maximum) {
    return value <= maximum ? value : maximum;
}

static int minimum_int(int value, int maximum) {
    return value <= maximum ? value : maximum;
}

static int clamp(int value, int minValue, int maxValue) {
    if (value <= minValue) return minValue;
    if (value >= maxValue) return maxValue;
    return value;
}

static float rgb2y(float r, float g, float b) {
    return +0.299f * r + 0.587f * g + 0.114f * b;
}

static float rgb2cb(float r, float g, float b) {
    return -0.16874f * r - 0.33126f * g + 0.5f * b;
}

static float rgb2cr(float r, float g, float b) {
    return +0.5f * r - 0.41869f * g - 0.08131f * b;
}

// DCT implementation
static void DCT(float block[8*8], unsigned char stride) {
    float SqrtHalfSqrt = 1.306562965f;
    float InvSqrt = 0.707106781f;
    float HalfSqrtSqrt = 0.382683432f;
    float InvSqrtSqrt = 0.541196100f;

    float *block0 = &block[0];
    float *block1 = &block[1 * stride];
    float *block2 = &block[2 * stride];
    float *block3 = &block[3 * stride];
    float *block4 = &block[4 * stride];
    float *block5 = &block[5 * stride];
    float *block6 = &block[6 * stride];
    float *block7 = &block[7 * stride];

    float add07 = *block0 + *block7; float sub07 = *block0 - *block7;
    float add16 = *block1 + *block6; float sub16 = *block1 - *block6;
    float add25 = *block2 + *block5; float sub25 = *block2 - *block5;
    float add34 = *block3 + *block4; float sub34 = *block3 - *block4;

    float add0347 = add07 + add34; float sub07_34 = add07 - add34;
    float add1256 = add16 + add25; float sub16_25 = add16 - add25;

    *block0 = add0347 + add1256; *block4 = add0347 - add1256;

    float z1 = (sub16_25 + sub07_34) * InvSqrt;
    *block2 = sub07_34 + z1; *block6 = sub07_34 - z1;

    float sub23_45 = sub25 + sub34;
    float sub12_56 = sub16 + sub25;
    float sub01_67 = sub16 + sub07;

    float z5 = (sub23_45 - sub01_67) * HalfSqrtSqrt;
    float z2 = sub23_45 * InvSqrtSqrt + z5;
    float z3 = sub12_56 * InvSqrt;
    float z4 = sub01_67 * SqrtHalfSqrt + z5;
    float z6 = sub07 + z3;
    float z7 = sub07 - z3;
    *block1 = z6 + z4; *block7 = z6 - z4;
    *block5 = z7 + z2; *block3 = z7 - z2;
}

// BitWriter functions
static void bitwriter_write_bits(BitWriter *writer, BitCode *data) {
    writer->buffer.numBits += data->numBits;
    writer->buffer.data <<= data->numBits;
    writer->buffer.data |= data->code;

    while (writer->buffer.numBits >= 8) {
        writer->buffer.numBits -= 8;
        unsigned char oneByte = (unsigned char)(writer->buffer.data >> writer->buffer.numBits);
        writer->output(oneByte);

        if (oneByte == 0xFF)
            writer->output(0);
    }
}

static void bitwriter_write_byte(BitWriter *writer, unsigned char oneByte) {
    writer->output(oneByte);
}

static void bitwriter_write_bytes(BitWriter *writer, unsigned char *bytes, int count) {
    for (int i = 0; i < count; i++)
        writer->output(bytes[i]);
}

static void bitwriter_flush(BitWriter *writer) {
    BitCode flush_code;
    flush_code.code = 0x7F;
    flush_code.numBits = 7;
    bitwriter_write_bits(writer, &flush_code);
}

static void bitwriter_add_marker(BitWriter *writer, unsigned char id, unsigned short length) {
    writer->output(0xFF);
    writer->output(id);
    writer->output((unsigned char)(length >> 8));
    writer->output((unsigned char)(length & 0xFF));
}

// Huffman table generation
static void generateHuffmanTable(unsigned char numCodes[16], unsigned char* values, BitCode result[256]) {
    unsigned int huffmanCode = 0;
    for (int numBits = 1; numBits <= 16; numBits++) {
        for (int i = 0; i < numCodes[numBits - 1]; i++) {
            result[*values] = (BitCode){huffmanCode++, numBits};
            values++;
        }
        huffmanCode <<= 1;
    }
}

// Block encoding
static short encodeBlock(BitWriter *writer, float block[8][8], float scaled[8*8], short lastDC,
                         BitCode huffmanDC[256], BitCode huffmanAC[256], BitCode* codewords) {
    float *block64 = (float*)block;

    // DCT: rows
    for (int offset = 0; offset < 8; offset++)
        DCT(block64 + offset*8, 1);
    // DCT: columns
    for (int offset = 0; offset < 8; offset++)
        DCT(block64 + offset*1, 8);

    // scale
    for (int i = 0; i < 8*8; i++)
        block64[i] *= scaled[i];

    int DC = (int)(block64[0] + (block64[0] >= 0 ? +0.5f : -0.5f));

    int posNonZero = 0;
    short quantized[8*8];
    for (int i = 1; i < 8*8; i++) {
        float value = block64[ZigZagInv[i]];
        quantized[i] = (int)(value + (value >= 0 ? +0.5f : -0.5f));
        if (quantized[i] != 0)
            posNonZero = i;
    }

    short diff = DC - lastDC;
    if (diff == 0) {
        bitwriter_write_bits(writer, &huffmanDC[0x00]);
    } else {
        BitCode bits = codewords[diff];
        bitwriter_write_bits(writer, &huffmanDC[bits.numBits]);
        bitwriter_write_bits(writer, &bits);
    }

    int offset = 0;
    for (int i = 1; i <= posNonZero; i++) {
        while (quantized[i] == 0) {
            offset += 0x10;
            if (offset > 0xF0) {
                bitwriter_write_bits(writer, &huffmanAC[0xF0]);
                offset = 0;
            }
            i++;
        }

        BitCode encoded = codewords[quantized[i]];
        bitwriter_write_bits(writer, &huffmanAC[offset + encoded.numBits]);
        bitwriter_write_bits(writer, &encoded);
        offset = 0;
    }

    if (posNonZero < 8*8 - 1)
        bitwriter_write_bits(writer, &huffmanAC[0x00]);

    return DC;
}

// Main writeJpeg function
char writeJpeg(WRITE_ONE_BYTE output, void* pixels_, unsigned short width, unsigned short height) {
    unsigned char isRGB = 1;
    unsigned char quality_ = 90;
    unsigned char downsample = 0;
    const char* comment = "Created with toojpeg. See https://create.stephan-brumme.com/toojpeg/";

    if (output == NULL || pixels_ == NULL)
        return 0;
    if (width == 0 || height == 0)
        return 0;

    int numComponents = isRGB ? 3 : 1;
    if (!isRGB)
        downsample = 0;

    BitWriter bitWriter;
    bitWriter.output = output;
    bitWriter.buffer.data = 0;
    bitWriter.buffer.numBits = 0;

    // JFIF headers
    unsigned char HeaderJfif[20] = {
        0xFF,0xD8,
        0xFF,0xE0,
        0,16,
        'J','F','I','F',0,
        1,1,
        0,
        0,1,0,1,
        0,0
    };
    bitwriter_write_bytes(&bitWriter, HeaderJfif, 20);

    // Comment
    if (comment != NULL) {
        int length = 0;
        while (comment[length] != 0)
            length++;

        bitwriter_add_marker(&bitWriter, 0xFE, 2 + length);
        for (int i = 0; i < length; i++)
            bitwriter_write_byte(&bitWriter, comment[i]);
    }

    // Quality adjustment
    unsigned short quality = clamp(quality_, 1, 100);
    quality = quality < 50 ? 5000 / quality : 200 - quality * 2;

    unsigned char quantLuminance[8*8];
    unsigned char quantChrominance[8*8];
    for (int i = 0; i < 8*8; i++) {
        int luminance = (DefaultQuantLuminance[ZigZagInv[i]] * quality + 50) / 100;
        int chrominance = (DefaultQuantChrominance[ZigZagInv[i]] * quality + 50) / 100;
        quantLuminance[i] = clamp(luminance, 1, 255);
        quantChrominance[i] = clamp(chrominance, 1, 255);
    }

    // Quantization tables
    bitwriter_add_marker(&bitWriter, 0xDB, 2 + (isRGB ? 2 : 1) * (1 + 8*8));
    bitwriter_write_byte(&bitWriter, 0x00);
    bitwriter_write_bytes(&bitWriter, quantLuminance, 8*8);
    if (isRGB) {
        bitwriter_write_byte(&bitWriter, 0x01);
        bitwriter_write_bytes(&bitWriter, quantChrominance, 8*8);
    }

    // SOF0
    bitwriter_add_marker(&bitWriter, 0xC0, 2 + 6 + 3*numComponents);
    bitwriter_write_byte(&bitWriter, 0x08);
    bitwriter_write_byte(&bitWriter, (unsigned char)(height >> 8));
    bitwriter_write_byte(&bitWriter, (unsigned char)(height & 0xFF));
    bitwriter_write_byte(&bitWriter, (unsigned char)(width >> 8));
    bitwriter_write_byte(&bitWriter, (unsigned char)(width & 0xFF));
    bitwriter_write_byte(&bitWriter, numComponents);
    
    for (int id = 1; id <= numComponents; id++) {
        bitwriter_write_byte(&bitWriter, id);
        bitwriter_write_byte(&bitWriter, (id == 1 && downsample ? 0x22 : 0x11));
        bitwriter_write_byte(&bitWriter, (id == 1 ? 0 : 1));
    }

    // DHT
    bitwriter_add_marker(&bitWriter, 0xC4, isRGB ? (2+208+208) : (2+208));
    bitwriter_write_byte(&bitWriter, 0x00);
    bitwriter_write_bytes(&bitWriter, DcLuminanceCodesPerBitsize, 16);
    bitwriter_write_bytes(&bitWriter, DcLuminanceValues, 12);
    bitwriter_write_byte(&bitWriter, 0x10);
    bitwriter_write_bytes(&bitWriter, AcLuminanceCodesPerBitsize, 16);
    bitwriter_write_bytes(&bitWriter, AcLuminanceValues, 162);

    BitCode huffmanLuminanceDC[256];
    BitCode huffmanLuminanceAC[256];
    generateHuffmanTable(DcLuminanceCodesPerBitsize, DcLuminanceValues, huffmanLuminanceDC);
    generateHuffmanTable(AcLuminanceCodesPerBitsize, AcLuminanceValues, huffmanLuminanceAC);

    BitCode huffmanChrominanceDC[256];
    BitCode huffmanChrominanceAC[256];
    if (isRGB) {
        bitwriter_write_byte(&bitWriter, 0x01);
        bitwriter_write_bytes(&bitWriter, DcChrominanceCodesPerBitsize, 16);
        bitwriter_write_bytes(&bitWriter, DcChrominanceValues, 12);
        bitwriter_write_byte(&bitWriter, 0x11);
        bitwriter_write_bytes(&bitWriter, AcChrominanceCodesPerBitsize, 16);
        bitwriter_write_bytes(&bitWriter, AcChrominanceValues, 162);

        generateHuffmanTable(DcChrominanceCodesPerBitsize, DcChrominanceValues, huffmanChrominanceDC);
        generateHuffmanTable(AcChrominanceCodesPerBitsize, AcChrominanceValues, huffmanChrominanceAC);
    }

    // SOS
    bitwriter_add_marker(&bitWriter, 0xDA, 2 + 1 + 2*numComponents + 3);
    bitwriter_write_byte(&bitWriter, numComponents);
    for (int id = 1; id <= numComponents; id++) {
        bitwriter_write_byte(&bitWriter, id);
        bitwriter_write_byte(&bitWriter, (id == 1 ? 0x00 : 0x11));
    }
    static unsigned char Spectral[3] = { 0, 63, 0 };
    bitwriter_write_bytes(&bitWriter, Spectral, 3);

    // Scaled quantization
    float scaledLuminance[8*8];
    float scaledChrominance[8*8];
    static float AanScaleFactors[8] = { 1, 1.387039845f, 1.306562965f, 1.175875602f, 1, 0.785694958f, 0.541196100f, 0.275899379f };
    
    for (int i = 0; i < 8*8; i++) {
        int row = ZigZagInv[i] / 8;
        int column = ZigZagInv[i] % 8;
        float factor = 1.0f / (AanScaleFactors[row] * AanScaleFactors[column] * 8);
        scaledLuminance[ZigZagInv[i]] = factor / quantLuminance[i];
        scaledChrominance[ZigZagInv[i]] = factor / quantChrominance[i];
    }

    // Codewords
    BitCode codewordsArray[2 * CodeWordLimit];
    BitCode* codewords = &codewordsArray[CodeWordLimit];
    unsigned char numBits = 1;
    int mask = 1;
    for (short value = 1; value < CodeWordLimit; value++) {
        if (value > mask) {
            numBits++;
            mask = (mask << 1) | 1;
        }
        codewords[-value] = (BitCode){mask - value, numBits};
        codewords[+value] = (BitCode){value, numBits};
    }

    unsigned char *pixels = (unsigned char*)pixels_;
    int maxWidth = width - 1;
    int maxHeight = height - 1;

    int sampling = downsample ? 2 : 1;
    int mcuSize = 8 * sampling;

    short lastYDC = 0, lastCbDC = 0, lastCrDC = 0;
    float Y[8][8], Cb[8][8], Cr[8][8];

    for (int mcuY = 0; mcuY < height; mcuY += mcuSize) {
        for (int mcuX = 0; mcuX < width; mcuX += mcuSize) {
            for (int blockY = 0; blockY < mcuSize; blockY += 8) {
                for (int blockX = 0; blockX < mcuSize; blockX += 8) {
                    for (int deltaY = 0; deltaY < 8; deltaY++) {
                        int column = minimum_int(mcuX + blockX, maxWidth);
                        int row = minimum_int(mcuY + blockY + deltaY, maxHeight);
                        for (int deltaX = 0; deltaX < 8; deltaX++) {
                            int pixelPos = row * width + column;
                            if (column < maxWidth)
                                column++;

                            if (!isRGB) {
                                Y[deltaY][deltaX] = pixels[pixelPos] - 128.0f;
                                continue;
                            }

                            unsigned char r = pixels[3 * pixelPos];
                            unsigned char g = pixels[3 * pixelPos + 1];
                            unsigned char b = pixels[3 * pixelPos + 2];

                            Y[deltaY][deltaX] = rgb2y(r, g, b) - 128;
                            if (!downsample) {
                                Cb[deltaY][deltaX] = rgb2cb(r, g, b);
                                Cr[deltaY][deltaX] = rgb2cr(r, g, b);
                            }
                        }
                    }

                    lastYDC = encodeBlock(&bitWriter, Y, scaledLuminance, lastYDC, huffmanLuminanceDC, huffmanLuminanceAC, codewords);
                }
            }

            if (!isRGB)
                continue;

            if (downsample) {
                for (short deltaY = 7; downsample && deltaY >= 0; deltaY--) {
                    int row = minimum_int(mcuY + 2*deltaY, maxHeight);
                    int column = mcuX;
                    int pixelPos = (row * width + column) * 3;

                    int rowStep = (row < maxHeight) ? 3 * width : 0;
                    int columnStep = (column < maxWidth) ? 3 : 0;

                    for (short deltaX = 0; deltaX < 8; deltaX++) {
                        int right = pixelPos + columnStep;
                        int down = pixelPos + rowStep;
                        int downRight = pixelPos + columnStep + rowStep;

                        short r = (short)pixels[pixelPos] + pixels[right] + pixels[down] + pixels[downRight];
                        short g = (short)pixels[pixelPos + 1] + pixels[right + 1] + pixels[down + 1] + pixels[downRight + 1];
                        short b = (short)pixels[pixelPos + 2] + pixels[right + 2] + pixels[down + 2] + pixels[downRight + 2];

                        Cb[deltaY][deltaX] = rgb2cb(r, g, b) / 4;
                        Cr[deltaY][deltaX] = rgb2cr(r, g, b) / 4;

                        pixelPos += 2*3;
                        column += 2;

                        if (column >= maxWidth) {
                            columnStep = 0;
                            pixelPos = ((row + 1) * width - 1) * 3;
                        }
                    }
                }
            }

            lastCbDC = encodeBlock(&bitWriter, Cb, scaledChrominance, lastCbDC, huffmanChrominanceDC, huffmanChrominanceAC, codewords);
            lastCrDC = encodeBlock(&bitWriter, Cr, scaledChrominance, lastCrDC, huffmanChrominanceDC, huffmanChrominanceAC, codewords);
        }
    }

    bitwriter_flush(&bitWriter);

    // EOI
    bitwriter_write_byte(&bitWriter, 0xFF);
    bitwriter_write_byte(&bitWriter, 0xD9);

    return 1;
}