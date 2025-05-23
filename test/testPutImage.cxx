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

#include <fmt/format.h>

#include <chrono>
#include <system_error>
#include <thread>

#include "framebuffer8880.h"
#include "image8880.h"
#include "point.h"

//-------------------------------------------------------------------------

using namespace fb32;
using namespace std::chrono_literals;

//-------------------------------------------------------------------------

void
printUsage(
    FILE* file,
    const std::string& name)
{
    fmt::print(file, "\n");
    fmt::print(file, "Usage: {} <options>\n", name);
    fmt::print(file, "\n");
    fmt::print(file, "    --connector,-c - dri connector to use\n");
    fmt::print(file, "    --device,-d - dri device to use\n");
    fmt::print(file, "    --help,-h - print usage and exit\n");
    fmt::print(file, "\n");
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    uint32_t connector{0};
    std::string device{};
    std::string program{basename(argv[0])};

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

            printUsage(stdout, program);
            ::exit(EXIT_SUCCESS);

            break;

        default:

            printUsage(stderr, program);
            ::exit(EXIT_FAILURE);

            break;
        }
    }

    //---------------------------------------------------------------------

    try
    {
        FrameBuffer8880 fb{device};
        fb.clearBuffers();

        //-----------------------------------------------------------------

        RGB8880 red{255, 0, 0};

        //-----------------------------------------------------------------

        constexpr int iwidth{30};
        constexpr int ihwidth{iwidth/2};
        const auto fwidth{fb.getWidth()};
        const auto fheight{fb.getHeight()};

        Image8880 image
        {
            iwidth,
            iwidth,
            {
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00000000,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000,
                0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00008000,
                0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000,
                0x00008000, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000,
                0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00000000,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00000000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00000000,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000, 0x00008000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000, 0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00,
                0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00ffff00, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
            }
        };

        for (auto y : { -ihwidth, (fheight / 2) - ihwidth, fheight - ihwidth })
        {
            for (auto x : { -ihwidth, (fwidth / 2) - ihwidth, fwidth - ihwidth })
            {
                fb.putImage(Interface8880Point{x, y}, image);
            }
        }

        //-----------------------------------------------------------------

        fb.update();

        //-----------------------------------------------------------------

        std::this_thread::sleep_for(10s);

        fb.clearBuffers();
    }
    catch (std::exception& error)
    {
        fmt::print(stderr, "Error: {}\n", error.what());
        exit(EXIT_FAILURE);
    }

    return 0;
}
