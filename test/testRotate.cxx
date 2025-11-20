//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2025 Andrew Duncan
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

#include <fmt/format.h>

#include <chrono>
#include <csignal>
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

//-------------------------------------------------------------------------

namespace
{
volatile static std::sig_atomic_t run = 1;
}

//-------------------------------------------------------------------------

static void
signalHandler(
    int signalNumber)
{
    switch (signalNumber)
    {
    case SIGINT:
    case SIGTERM:

        run = 0;
        break;
    };
}

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

    for (auto signal : { SIGINT, SIGTERM })
    {
        if (std::signal(signal, signalHandler) == SIG_ERR)
        {
            std::println(
                std::cerr,
                "Error: installing {} signal handler : {}",
                strsignal(signal),
                strerror(errno));

            ::exit(EXIT_FAILURE);
        }
    }

    //---------------------------------------------------------------------

    try
    {
        constexpr RGB8880 darkBlue{0, 0, 63};
        constexpr RGB8880 darkGrey{63, 63, 63};
        constexpr RGB8880 white{255, 255, 255};

        FrameBuffer8880 fb{device, connector};
        fb.clearBuffers(darkGrey);

        constexpr int width{72};
        constexpr int height{16};

        Image8880 image(width, height);
        image.clear(darkBlue);

        //-----------------------------------------------------------------

        Image8880Font8x16 font;

        font.drawString(
            Interface8880Point{4, 0},
            "rotating",
            white,
            image);

        //-----------------------------------------------------------------

        image = scaleUp(image, 3);

        //-----------------------------------------------------------------

        auto degreeChar = font.getCharacterCode(Interface8880Font::CharacterCode::DEGREE_SYMBOL).value_or(' ');

        for (int angle = 0; (angle < 3600) and run; ++angle)
        {
            fb.clear(darkGrey);

            font.drawString(
                Interface8880Point{4, 0},
                std::format("Angle: {:3d}{}{:02d}'", angle / 10, degreeChar, (angle % 10) * 6),
                white,
                fb);

            const auto rotated = rotate(image, darkGrey, angle / 10.0);
            const Interface8880Point p
            {
                (fb.getWidth() - rotated.getWidth()) / 2,
                (fb.getHeight() - rotated.getHeight()) / 2
            };

            fb.putImage(p, rotated);
            fb.update();
        }
    }
    catch (std::exception& error)
    {
        std::println(std::cerr, "Error: {}", error.what());
        exit(EXIT_FAILURE);
    }

    return 0;
}

