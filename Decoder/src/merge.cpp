#include <emscripten/emscripten.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>

extern "C" void RGBAlpha2RGBA(const uint32_t width, const uint32_t height, const uint8_t* rgbx, const uint8_t* alpha, uint8_t* rgba)
{
    memcpy(rgba, rgbx, width * height * 4);
    for (int p = 0; p < width * height; p++)
    {
        rgba[(p << 2) + 3] = alpha[p];
    }
}

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 1192 * C(Y)            + 1634 * E(V) + 512) >> 10)
#define YUV2G(Y, U, V) CLIP(( 1192 * C(Y) - 400 * D(U) - 832 * E(V) + 512) >> 10)
#define YUV2B(Y, U, V) CLIP(( 1192 * C(Y) + 2066 * D(U)              + 512) >> 10)

extern "C" void NV12Alpha2RGBA(const uint32_t width, const uint32_t height, const uint8_t* nv12, const uint8_t* alpha, uint8_t* rgba)
{
    const uint8_t* lumaPtr = nv12;
    const uint8_t* chromaUPtr = nv12 + width * height;
    const uint8_t* chromaVPtr = nv12 + width * height + ((width * height) >> 2);
    const uint8_t* alphaPtr = alpha;

    const uint32_t lumaStride = width;
    const uint32_t chromaStride = width >> 1;
    const uint32_t dstStride = width * 4;

    for (int h = 0; h < (height >> 1); h++)
    {
        uint8_t *dstScanline0 = rgba + 2 * h * dstStride;
        uint8_t *dstScanline1 = dstScanline0 + dstStride;

        const uint8_t *lumaScanline0 = lumaPtr + 2 * h * lumaStride;
        const uint8_t *lumaScanline1 = lumaScanline0 + lumaStride;

        const uint8_t *chromaUScanline = chromaUPtr + h * chromaStride;
        const uint8_t *chromaVScanline = chromaVPtr + h * chromaStride;

        const uint8_t *alphaScanline0 = alphaPtr + 2 * h * lumaStride;
        const uint8_t *alphaScanline1 = alphaScanline0 + lumaStride;

        for (int w = 0; w < (width >> 1); w++)
        {
            uint8_t *dstScanline00 = &dstScanline0[w << 3];
            uint8_t *dstScanline10 = &dstScanline1[w << 3];
            auto y00 = lumaScanline0[(w << 1)];
            auto y01 = lumaScanline0[(w << 1) + 1];
            auto y10 = lumaScanline1[(w << 1)];
            auto y11 = lumaScanline1[(w << 1) + 1];
            auto u = chromaUScanline[w];
            auto v = chromaVScanline[w];

            dstScanline00[0] = YUV2R(y00, u, v);
            dstScanline00[1] = YUV2G(y00, u, v);
            dstScanline00[2] = YUV2B(y00, u, v);
            dstScanline00[3] = CLIP(((alphaScanline0[(w << 1)] - 16) * 298) >> 8);

            dstScanline00[4] = YUV2R(y01, u, v);
            dstScanline00[5] = YUV2G(y01, u, v);
            dstScanline00[6] = YUV2B(y01, u, v);
            dstScanline00[7] = CLIP(((alphaScanline0[(w << 1) + 1] - 16) * 298) >> 8);

            dstScanline10[0] = YUV2R(y10, u, v);
            dstScanline10[1] = YUV2G(y10, u, v);
            dstScanline10[2] = YUV2B(y10, u, v);
            dstScanline10[3] = CLIP(((alphaScanline1[(w << 1)] - 16) * 298) >> 8);

            dstScanline10[4] = YUV2R(y11, u, v);
            dstScanline10[5] = YUV2G(y11, u, v);
            dstScanline10[6] = YUV2B(y11, u, v);
            dstScanline10[7] = CLIP(((alphaScanline1[(w << 1) + 1] - 16) * 298) >> 8);
        }
    }
}

extern "C" void NV122RGBA(const uint32_t width, const uint32_t height, const uint8_t* nv12, uint8_t* rgba)
{
    const uint8_t* lumaPtr = nv12;
    const uint8_t* chromaUPtr = nv12 + width * height;
    const uint8_t* chromaVPtr = nv12 + width * height + ((width * height) >> 2);

    const uint32_t lumaStride = width;
    const uint32_t chromaStride = width >> 1;
    const uint32_t dstStride = width * 4;

    for (int h = 0; h < (height >> 1); h++)
    {
        uint8_t *dstScanline0 = rgba + 2 * h * dstStride;
        uint8_t *dstScanline1 = dstScanline0 + dstStride;

        const uint8_t *lumaScanline0 = lumaPtr + 2 * h * lumaStride;
        const uint8_t *lumaScanline1 = lumaScanline0 + lumaStride;

        const uint8_t *chromaUScanline = chromaUPtr + h * chromaStride;
        const uint8_t *chromaVScanline = chromaVPtr + h * chromaStride;

        for (int w = 0; w < (width >> 1); w++)
        {
            uint8_t *dstScanline00 = &dstScanline0[w << 3];
            uint8_t *dstScanline10 = &dstScanline1[w << 3];
            auto y00 = lumaScanline0[(w << 1)];
            auto y01 = lumaScanline0[(w << 1) + 1];
            auto y10 = lumaScanline1[(w << 1)];
            auto y11 = lumaScanline1[(w << 1) + 1];
            auto u = chromaUScanline[w];
            auto v = chromaVScanline[w];

            dstScanline00[0] = YUV2R(y00, u, v);
            dstScanline00[1] = YUV2G(y00, u, v);
            dstScanline00[2] = YUV2B(y00, u, v);
            dstScanline00[3] = 255;

            dstScanline00[4] = YUV2R(y01, u, v);
            dstScanline00[5] = YUV2G(y01, u, v);
            dstScanline00[6] = YUV2B(y01, u, v);
            dstScanline00[7] = 255;

            dstScanline10[0] = YUV2R(y10, u, v);
            dstScanline10[1] = YUV2G(y10, u, v);
            dstScanline10[2] = YUV2B(y10, u, v);
            dstScanline10[3] = 255;

            dstScanline10[4] = YUV2R(y11, u, v);
            dstScanline10[5] = YUV2G(y11, u, v);
            dstScanline10[6] = YUV2B(y11, u, v);
            dstScanline10[7] = 255;
        }
    }
}
