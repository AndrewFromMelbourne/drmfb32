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

#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <print>
#include <thread>

#include "framebuffer8880.h"
#include "webcam.h"

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
    int signalNumber) noexcept
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
    std::println(stream,"    --FPS,-F - set the desired frames per second");
    std::println(stream,"    --connector,-c - dri connector to use");
    std::println(stream,"    --device,-d - dri device to use");
    std::println(stream,"    --fit,-f - fit image to screen");
    std::println(stream,"    --FPS,-F - request frames per second");
    std::println(stream,"    --help,-h - print usage and exit");
    std::println(stream,"    --videodevice,-v - video device to use");
    std::println(stream, "");
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    uint32_t connector{0};
    std::string device{""};
    const std::string program{basename(argv[0])};
    bool fitToScreen{false};
    int requestedFPS{0};
    std::string videoDevice{"/dev/video0"};

    //---------------------------------------------------------------------

    static const char* sopts = "F:c:d:fhv:";
    static option lopts[] =
    {
        { "FPS", no_argument, NULL, 'F' },
        { "connector", required_argument, nullptr, 'c' },
        { "device", required_argument, nullptr, 'd' },
        { "help", no_argument, nullptr, 'h' },
        { "fit", no_argument, nullptr, 'f' },
        { "videodevice", required_argument, NULL, 'v' },
        { nullptr, no_argument, nullptr, 0 }
    };

    int opt{};

    while ((opt = ::getopt_long(argc, argv, sopts, lopts, nullptr)) != -1)
    {
        switch (opt)
        {
        case 'F':

            requestedFPS = std::stol(optarg);

            break;

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

        case 'v':

            videoDevice = optarg;

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
        FrameBuffer8880 fb(device, connector);
        Webcam wc(videoDevice, fitToScreen, requestedFPS, fb);

        fb.clearBuffers(RGB8880{0, 0, 0});

        //-----------------------------------------------------------------

        const auto [ width, height ] = wc.dimensions();
        std::println("{} [{} x {}]", wc.formatName(), width, height);

        //-----------------------------------------------------------------

        wc.startStream();

        while (run)
        {
            wc.showFrame(fb);
            fb.update();
        }

        wc.stopStream();

        fb.clearBuffers();
    }
    catch (std::exception& error)
    {
        std::println(std::cerr, "Error: {}", error.what());
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    return 0 ;
}

