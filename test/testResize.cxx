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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <system_error>
#include <thread>

#include <getopt.h>
#include <libgen.h>
#include <unistd.h>

#include "framebuffer8880.h"
#include "image8880.h"
#include "image8880Font8x16.h"
#include "image8880Graphics.h"
#include "point.h"

//-------------------------------------------------------------------------

using namespace fb32;
using namespace std::chrono_literals;

//-------------------------------------------------------------------------

void
printUsage(
    std::ostream& os,
    const std::string& name)
{
    os << '\n';
    os << "Usage: " << name << " <options>\n";
    os << '\n';
    os << "    --connector,-c - dri connector to use\n";
    os << "    --device,-d - dri device to use\n";
    os << "    --help,-h - print usage and exit\n";
    os << '\n';
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    uint32_t connector{0};
    std::string device{};
    std::string program = basename(argv[0]);

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
        fb.clear();

        const RGB8880 darkBlue{0, 0, 63};
        const RGB8880 white{255, 255, 255};

        constexpr int width{248};
        constexpr int height{16};

        Image8880 image(width, height);
        image.clear(darkBlue);

        //-----------------------------------------------------------------

        Image8880Font8x16 font;

        font.drawString(
            Interface8880Point{4, 0},
            "Lorem ipsum dolor sit amet ...",
            white,
            image);

        //-----------------------------------------------------------------

        constexpr int scale{3};
        constexpr int swidth{scale * width};
        constexpr int sheight{scale * height};
        constexpr int imageOffset{200};
        constexpr int yStep{sheight + 8};

        auto imageSu = image.scaleUp(scale);
        auto imageNn =  image.resizeNearestNeighbour(swidth, sheight);
        auto imageBi = image.resizeBilinearInterpolation(swidth, sheight);
        auto imageLi = image.resizeLanczos3Interpolation(swidth, sheight);

        Interface8880Point t{0, 0};
        Interface8880Point p{ imageOffset, 0 };

        auto show = [&](std::string_view title, Image8880& image)
        {
            font.drawString(t, title, white, fb);
            fb.putImage(p, image);
            t.incrY(yStep);
            p.incrY(yStep);
        };

        show("Scale up:", imageSu);
        show("Nearest neighbour:", imageNn);
        show("Bilinear interpolation:", imageBi);
        show("Lanczos3 interpolation:", imageLi);

        fb.update();

        //-----------------------------------------------------------------

        std::this_thread::sleep_for(10s);
        fb.clear();
    }
    catch (std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        exit(EXIT_FAILURE);
    }

    return 0;
}

