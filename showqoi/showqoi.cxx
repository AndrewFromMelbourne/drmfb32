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
#include <csignal>
#include <cstring>
#include <iostream>
#include <print>
#include <thread>

#include "framebuffer8880.h"
#include "image8880Process.h"
#include "image8880Qoi.h"

//-------------------------------------------------------------------------

using namespace fb32;
using namespace std::chrono_literals;

//-------------------------------------------------------------------------

namespace
{
volatile static std::sig_atomic_t run{1};
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
    std::println(stream , "Usage: {} <options>", name);
    std::println(stream, "");
    std::println(stream, "    --background,-b - background colour");
    std::println(stream, "    --connector,-c - dri connector to use");
    std::println(stream, "    --device,-d - dri device to use");
    std::println(stream, "    --fit,-f - fit image to screen");
    std::println(stream, "    --help,-h - print usage and exit");
    std::println(stream, "    --qoi,-q - qoi file to display");
    std::println(stream, "");
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    fb32::RGB8880 background{0, 0, 0};
    uint32_t connector{0};
    std::string device{""};
    const std::string program{basename(argv[0])};
    std::string qoi{};
    bool fitToScreen{false};

    //---------------------------------------------------------------------

    static const char* sopts = "b:c:d:fhq:";
    static option lopts[] =
    {
        { "background", required_argument, nullptr, 'b' },
        { "connector", required_argument, nullptr, 'c' },
        { "device", required_argument, nullptr, 'd' },
        { "fit", no_argument, nullptr, 'f' },
        { "help", no_argument, nullptr, 'h' },
        { "qoi", required_argument, nullptr, 'q' },
        { nullptr, no_argument, nullptr, 0 }
    };

    int opt{};

    while ((opt = ::getopt_long(argc, argv, sopts, lopts, nullptr)) != -1)
    {
        switch (opt)
        {
        case 'b':
        {
            auto bg = parseRGB8880(optarg);

            if (bg.has_value())
            {
                background = bg.value();
            }
            else
            {
                std::println(
                    std::cerr,
                    "Error: invalid background colour \"{}\"",
                    optarg);

                ::exit(EXIT_FAILURE);
            }

            break;
        }
        case 'c':

            connector = std::stol(optarg);

            break;

        case 'd':

            device = optarg;

            break;

        case 'f':

            fitToScreen = true;

            break;

        case 'h':

            printUsage(std::cout, program);
            ::exit(EXIT_SUCCESS);

            break;

        case 'q':

            qoi = optarg;

            break;

        default:

            printUsage(std::cerr, program);
            ::exit(EXIT_FAILURE);

            break;
        }
    }

    //---------------------------------------------------------------------

    if (qoi.empty())
    {
        printUsage(std::cerr, program);
        ::exit(EXIT_FAILURE);
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
        FrameBuffer8880 fb(device, connector);
        fb.clearBuffers(background);

        auto image = readQoi(qoi, background);

        if (fitToScreen)
        {
            int width = (fb.getHeight() * image.getWidth()) / image.getHeight();
            int height = fb.getHeight();

            if (width > fb.getWidth())
            {
                width = fb.getWidth();
                height = (fb.getWidth() * image.getHeight()) / image.getWidth();
            }

            image = resizeBilinearInterpolation(image, width, height);
        }

        fb.putImage(center(fb, image), image);
        fb.update();

        //-----------------------------------------------------------------

        while (run)
        {
            std::this_thread::sleep_for(1s);
        }
    }
    catch (std::exception& error)
    {
        std::println(std::cerr, "Error: {}", error.what());
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    return 0 ;
}

