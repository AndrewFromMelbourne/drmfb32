//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2024 Andrew Duncan
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

#include <string>
#include <vector>

#include "fileDescriptor.h"
#include "image8880.h"
#include "interface8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

class Webcam
{
public:

    struct Dimensions
    {
        int width;
        int height;
    };

    struct VideoBuffer
    {
        int length;
        void* buffer;
    };

    explicit Webcam(
        const std::string& device,
        bool fitToScreen,
        int requestedFPS,
        Interface8880& image);

    ~Webcam();

    Webcam(const Webcam& wc) = delete;
    Webcam& operator=(const Webcam& wc) = delete;

    Webcam(Webcam&& wc) = delete;
    Webcam& operator=(Webcam&& wc) = delete;

    const Dimensions& dimensions() const
    {
        return m_dimensions;
    }

    std::string formatName() const
    {
        return m_formatName;
    }

    bool showFrame(Interface8880& image);
    bool startStream() const;
    bool stopStream() const;

private:

    bool chooseBestFit(Interface8880& image);
    bool chooseFormat();
    bool convertMjpeg(const uint8_t* data, int length);
    bool convertYuyv(const uint8_t* data, int length);
    bool hasVideoCapabilities() const;
    bool initBuffers();
    void initResizedImage(Interface8880& image);
    bool initVideo();
    bool setFPS(int fps) const;

    Dimensions m_dimensions;
    FileDescriptor m_fd;
    bool m_fitToScreen;
    uint32_t m_format;
    std::string m_formatName;
    Image8880 m_image;
    Image8880 m_resizedImage;
    std::vector<VideoBuffer> m_videoBuffers;
};

//-------------------------------------------------------------------------

} // namespace fb32

//-------------------------------------------------------------------------
