//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2022 Andrew Duncan
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
#include <csignal>
#include <cstring>
#include <thread>

#include "framebuffer8880.h"
#include "sphere.h"

//-------------------------------------------------------------------------

using namespace fb32;
using namespace std::chrono_literals;

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
    std::string device = "";
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

    for (auto signal : { SIGINT, SIGTERM })
    {
        if (std::signal(signal, signalHandler) == SIG_ERR)
        {
            fmt::print(
                stderr,
                "Error: installing {} signal handler : {}\n",
                strsignal(signal),
                strerror(errno));

            ::exit(EXIT_FAILURE);
        }
    }
    //---------------------------------------------------------------------

    try
    {
        FrameBuffer8880 fb(device, connector);
        fb.clear(RGB8880{0, 0, 0});

        fmt::print("width = {} height = {}\n", fb.getWidth() ,fb.getHeight());

        Sphere sphere(fb.getHeight() - 10);
        sphere.setAmbient(0.1);

        //-----------------------------------------------------------------

        int bearing = 0;

        while (run)
        {
            sphere.init();
            sphere.setLight(45.0, bearing);
            sphere.update();
            sphere.draw(fb);
            fb.update();

            bearing = (bearing + 1) % 360;
        }

        fb.clear();
    }
    catch (std::exception& error)
    {
        fmt::print(stderr, "Error: {}\n", error.what());
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    return 0 ;
}

