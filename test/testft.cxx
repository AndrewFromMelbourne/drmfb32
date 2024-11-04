//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2023 Andrew Duncan
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

#include <chrono>
#include <iostream>
#include <system_error>
#include <thread>

#include "framebuffer8880.h"
#include "image8880.h"
#include "image8880FreeType.h"
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
    os << "\n";
    os << "Usage: " << name << " <options>\n";
    os << "\n";
    os << "    --connector,-c - dri connector to use\n";
    os << "    --device,-d - dri device to use\n";
    os << "    --font,-f - font file to use\n";
    os << "    --help,-h - print usage and exit\n";
    os << "\n";
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
    std::string font{};

    //---------------------------------------------------------------------

    static const char* sopts = "c:d:f:h";
    static option lopts[] =
    {
        { "connector", required_argument, nullptr, 'c' },
        { "device", required_argument, nullptr, 'd' },
        { "font", required_argument, nullptr, 'f' },
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

        case 'f':

            font = optarg;

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

    if (font.empty())
    {
        std::cerr << "Error: Font file must be specfied\n";
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    try
    {
        const RGB8880 black{0, 0, 0};
        const RGB8880 white{255, 255, 255};
        FrameBuffer8880 fb{device, connector};

        fb.clear(black);

        Image8880 image{fb.getWidth(), fb.getHeight()};
        image.clear(black);

        //-----------------------------------------------------------------

        Image8880FreeType ft{font, 32};

        Interface8880Point p{0, 0};

        p = ft.drawString(p, "abcdefghijklmnopqrstuvwxyz ", white, image);
        p = ft.drawString(p, "0123456789", white, image);

        p.setX(0);
        p.incrY(ft.getPixelHeight());

        p = ft.drawString(p, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", white, image);

        p.setX(0);
        p.incrY(ft.getPixelHeight());

        p = ft.drawChar(p, '@', white, image);

        p.setX(0);
        p.incrY(ft.getPixelHeight());

        for (int j = 0 ; j < 16 ; ++j)
        {
            for (int i = 0 ; i < 16 ; ++i)
            {
                const uint8_t c = static_cast<uint8_t>(i + (j * 16));
                p.setX(i * ft.getPixelWidth());
                ft.drawChar(p, c, white, image);
            }

            p.incrY(ft.getPixelHeight());
        }

        //-----------------------------------------------------------------

        fb.putImage(Interface8880Point{0, 0}, image);
        fb.update();

        //-----------------------------------------------------------------

        std::this_thread::sleep_for(10s);

        fb.clear();
    }
    catch (std::exception& error)
    {
        std::cerr << "Error: " << error.what() << "\n";
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    return 0 ;
}

