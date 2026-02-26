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

#include <getopt.h>
#include <libgen.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <print>
#include <system_error>
#include <thread>

#include "framebuffer8880.h"
#include "image8880.h"
#include "image8880Font8x16.h"
#include "image8880Graphics.h"
#include "image8880Process.h"
#include "point.h"

//-------------------------------------------------------------------------

using namespace fb32;
using namespace std::chrono_literals;

//-------------------------------------------------------------------------

void
printUsage(
    std::ostream& stream,
    const std::string& name)
{
    std::println(stream, "");
    std::println(stream, "Usage: {} <options>", name);
    std::println(stream, "");
    std::println(stream, "    --connector,-c - dri connector to use");
    std::println(stream, "    --device,-d - dri device to use");
    std::println(stream, "    --help,-h - print usage and exit");
    std::println(stream, "");
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    uint32_t connector{0};
    std::string device{};
    const std::string program = basename(argv[0]);

    //---------------------------------------------------------------------

    static const char* sopts = "c:d:h";
    static option lopts[] =
    {
        { "connector", required_argument, nullptr, 'c' },
        { "device", required_argument, nullptr, 'd' },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, no_argument, nullptr, 0 }
    };

    int opt{};

    while ((opt = ::getopt_long(argc, argv, sopts, lopts, nullptr)) != -1)
    {
        switch (opt)
        {
        case 'c':

            connector = std::stol(optarg);
            break;

        case 'd':

            device = optarg;
            break;

        case 'h':

            printUsage(std::cout, program);
            ::exit(EXIT_SUCCESS);
            break;

        default:

            printUsage(std::cerr, program);
            ::exit(EXIT_FAILURE);
            break;
        }
    }

    //---------------------------------------------------------------------

    try
    {
        FrameBuffer8880 fb{device, connector};

        constexpr RGB8880 darkBlue{0, 0, 63};
        constexpr RGB8880 white{255, 255, 255};

        constexpr fb32::Dimensions8880 d{248, 16};

        Image8880 image(d);
        image.clear(darkBlue);

        //-----------------------------------------------------------------

        Image8880Font8x16 font;

        font.drawString(
            Point8880{4, 0},
            "Lorem ipsum dolor sit amet ...",
            white,
            image);

        //-----------------------------------------------------------------

        constexpr int scale{3};
        constexpr fb32::Dimensions8880 sd{d.width() * scale, d.height() * scale};
        constexpr int imageOffset{200};
        constexpr int yStep{sd.height() + 8};

        const auto imageSu = scaleUp(image, scale);
        const auto imageNn = resizeNearestNeighbour(image, sd);
        const auto imageBi = resizeBilinearInterpolation(image, sd);
        const auto imageLi = resizeLanczos3Interpolation(image, sd);

        Point8880 t{0, 0};
        Point8880 p{ imageOffset, 0 };

        auto show = [&](std::string_view title, const Image8880& image)
        {
            font.drawString(t, title, white, fb);
            fb.putImage(p, image);
            t.translateY(yStep);
            p.translateY(yStep);
        };

        show("Scale up:", imageSu);
        show("Nearest neighbour:", imageNn);
        show("Bilinear interpolation:", imageBi);
        show("Lanczos3 interpolation:", imageLi);

        fb.update();

        //-----------------------------------------------------------------

        std::this_thread::sleep_for(10s);
    }
    catch (std::exception& error)
    {
        std::println(std::cerr, "Error: {}", error.what());
        exit(EXIT_FAILURE);
    }
}

