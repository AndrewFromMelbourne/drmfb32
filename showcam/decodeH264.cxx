//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2026 Andrew Duncan
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------

#include "decodeH264.h"

//-------------------------------------------------------------------------

void freeAVCodecContext(AVCodecContext* ctx)
{
    avcodec_free_context(&ctx);
}

//-------------------------------------------------------------------------

void freeAVFrame(AVFrame* frame)
{
    av_frame_free(&frame);
}

//-------------------------------------------------------------------------

void freeAVPacket(AVPacket* pkt)
{
    av_packet_free(&pkt);
}

//-------------------------------------------------------------------------

fb32::DecodeH264::DecodeH264()
:
    m_codec{avcodec_find_decoder(AV_CODEC_ID_H264)},
    m_codecContext{avcodec_alloc_context3(m_codec), freeAVCodecContext},
    m_frame{av_frame_alloc(), freeAVFrame},
    m_pkt{av_packet_alloc(), freeAVPacket}
{
    if (not m_codec)
    {
        throw std::runtime_error{"Failed to find H264 decoder"};
    }

    if (not m_codecContext)
    {
        throw std::runtime_error{"Failed to allocate codec context"};
    }

    if (not m_frame)
    {
        throw std::runtime_error{"Failed to allocate frame"};
    }

    if (not m_pkt)
    {
        throw std::runtime_error{"Failed to allocate packet"};
    }
}

//-------------------------------------------------------------------------

bool
fb32::DecodeH264::decode(
    const uint8_t* data,
    int length,
    [[maybe_unused]] fb32::Image8880& image,
    [[maybe_unused]] bool greyscale)
{
    if (not m_codecContext)
    {
        return false;
    }

    if (avcodec_open2(m_codecContext.get(), m_codec, nullptr) < 0)
    {
        return false;
    }

    av_packet_unref(m_pkt.get());
    m_pkt->data = const_cast<uint8_t*>(data);
    m_pkt->size = length;

    int ret = avcodec_send_packet(m_codecContext.get(), m_pkt.get());
    if (ret < 0)
    {
        return false;
    }

    ret = avcodec_receive_frame(m_codecContext.get(), m_frame.get());
    if (ret < 0)
    {
        return false;
    }

    std::span<const uint8_t> yData{m_frame->data[0], static_cast<std::size_t>(m_frame->linesize[0] * m_frame->height)};

    if (greyscale)
    {
        auto buffer = begin(image.getBuffer());
        for (auto y : yData)
        {
            const RGB8880 rgb{y, y, y};
            *(buffer++) = rgb.get8880();
        }
    }
    else
    {
        std::span<const uint8_t> uData{m_frame->data[1], static_cast<std::size_t>(m_frame->linesize[1] * (m_frame->height / 2))};
        std::span<const uint8_t> vData{m_frame->data[2], static_cast<std::size_t>(m_frame->linesize[2] * (m_frame->height / 2))};
        auto buffer = begin(image.getBuffer());

        for (std::size_t i = 0 ; i < yData.size() ; i += 2)
        {
            const int y1 = yData[i] - 16;
            const int y2 = yData[i + 1] - 16;

            const auto x = static_cast<std::size_t>(i % m_frame->width);
            const auto y = static_cast<std::size_t>(i / m_frame->width);
            const std::size_t uvIndex = (x / 2) + (y / 2) * (m_frame->linesize[1]);
            const int u = uData[uvIndex] - 128;
            const int v = vData[uvIndex] - 128;

            const int y1Part = 1192 * y1;
            const int y2Part = 1192 * y2;

            const int rPart = 1634 * v;
            const int gPart = -832 * v - 400 * u;
            const int bPart = 2066 * u;

            const RGB8880 rgb1{
                static_cast<uint8_t>(std::clamp((y1Part + rPart) / 1024, 0, 255)),
                static_cast<uint8_t>(std::clamp((y1Part + gPart) / 1024, 0, 255)),
                static_cast<uint8_t>(std::clamp((y1Part + bPart) / 1024, 0, 255))};

            const RGB8880 rgb2{
                static_cast<uint8_t>(std::clamp((y2Part + rPart) / 1024, 0, 255)),
                static_cast<uint8_t>(std::clamp((y2Part + gPart) / 1024, 0, 255)),
                static_cast<uint8_t>(std::clamp((y2Part + bPart) / 1024, 0, 255))};

            *(buffer++) = rgb1.get8880();
            *(buffer++) = rgb2.get8880();
        }
    }

    return true;
}
