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

#include <csignal>
#include <cstring>

#include "framebuffer8880.h"
#include "image8880Jpeg.h"
#include "joystick.h"
#include "viewer.h"

//-------------------------------------------------------------------------

using namespace fb32;

//-------------------------------------------------------------------------

namespace
{
volatile static std::sig_atomic_t run{1};
const char* defaultJoystick = "/dev/input/js0";
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
    FILE* file,
    const std::string& name)
{
    fmt::print(file, "\n");
    fmt::print(file, "Usage: {} <options>\n", name);
    fmt::print(file, "\n");
    fmt::print(file, "    --connector,-c - dri connector to use\n");
    fmt::print(file, "    --device,-d - dri device to use\n");
    fmt::print(file, "    --folder,-f - folder containing images\n");
    fmt::print(file, "    --help,-h - print usage and exit\n");
    fmt::print(file, "    --joystick,-j - joystick device\n");
    fmt::print(file, "\n");;
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
    std::string folder{};
    std::string joystick{defaultJoystick};

    //---------------------------------------------------------------------

    static const char* sopts = "c:d:f:hj:";
    static option lopts[] =
    {
        { "connector", required_argument, nullptr, 'c' },
        { "device", required_argument, nullptr, 'd' },
        { "folder", required_argument, nullptr, 'f' },
        { "help", no_argument, nullptr, 'h' },
        { "joystick", required_argument, nullptr, 'j' },
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

            folder = optarg;

            break;

        case 'h':

            printUsage(stdout, program);
            ::exit(EXIT_SUCCESS);

            break;

        case 'j':

            joystick = optarg;

            break;

        default:

            printUsage(stderr, program);
            ::exit(EXIT_FAILURE);

            break;
        }
    }

    //---------------------------------------------------------------------

    if (folder.empty())
    {
        printUsage(stderr, program);
        ::exit(EXIT_FAILURE);
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
        Joystick js{joystick};
        Viewer viewer{fb, folder};

        viewer.draw(fb);
        fb.update();

        while (run)
        {
            js.read();

            if (js.buttonPressed(Joystick::BUTTON_START))
            {
                run = 0;
            }
            else if (viewer.update(js))
            {
                viewer.draw(fb);
                fb.update();
            }
        }

    }
    catch (std::exception& error)
    {
        fmt::print(stderr, "Error: {}\n", error.what());
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    return 0 ;
}

