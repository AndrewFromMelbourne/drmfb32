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

#pragma once

//-------------------------------------------------------------------------

#include <memory>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "image8880.h"

//-------------------------------------------------------------------------

void freeAVCodecContext(AVCodecContext* ctx);
void freeAVFrame(AVFrame* frame);
void freeAVPacket(AVPacket* pkt);

//-------------------------------------------------------------------------

using AVCodecContext_ptr = std::unique_ptr<AVCodecContext, decltype(&freeAVCodecContext)>;
using AVFrame_ptr = std::unique_ptr<AVFrame, decltype(&freeAVFrame)>;
using AVPacket_ptr = std::unique_ptr<AVPacket, decltype(&freeAVPacket)>;

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

class DecodeH264
{
public:

    DecodeH264();
    ~DecodeH264() = default;

    DecodeH264(const DecodeH264&) = delete;
    DecodeH264(DecodeH264&&) = delete;
    DecodeH264& operator=(const DecodeH264&) = delete;
    DecodeH264& operator=(DecodeH264&&) = delete;

    bool decode(
        const uint8_t* data,
        int length,
        Image8880& image,
        bool greyscale);

    static std::unique_ptr<DecodeH264> create()
    {
        return std::make_unique<DecodeH264>();
    }

private:

    const AVCodec* m_codec;
    AVCodecContext_ptr m_codecContext;
    AVFrame_ptr m_frame;
    AVPacket_ptr m_pkt;
};

//-------------------------------------------------------------------------

} // namespace fb32

//-------------------------------------------------------------------------
