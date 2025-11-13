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

#include <csignal>
#include <iostream>
#include <print>
#include <thread>

#include "framebuffer8880.h"
#include "image8880Font8x16.h"
#include "joystick.h"
#include "boxworld.h"

//-------------------------------------------------------------------------

using namespace std::chrono_literals;
using namespace fb32;

//-------------------------------------------------------------------------

namespace
{
volatile static std::sig_atomic_t run = 1;
const char* defaultJoystick = "/dev/input/js0";
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
    std::println(stream, "    --fitToScreen,-f - fit boxworld to screen");
    std::println(stream, "    --help,-h - println usage and exit");
    std::println(stream, "    --joystick,-j - joystick device");
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
    bool fitToScreen{false};
    const std::string program{basename(argv[0])};
    std::string joystick{defaultJoystick};

    //---------------------------------------------------------------------

    static const char* sopts = "c:d:fhj:";
    static option lopts[] =
    {
        { "connector", required_argument, nullptr, 'c' },
        { "device", required_argument, nullptr, 'd' },
        { "fitToScreen", no_argument, nullptr, 'f' },
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

            fitToScreen = true;

            break;

        case 'h':

            printUsage(std::cout, program);
            ::exit(EXIT_SUCCESS);

            break;

        case 'j':

            joystick = optarg;

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
        Image8880Font8x16 font;
        Joystick js{joystick};
        FrameBuffer8880 fb{device, connector};

        if (fb.getHeight() < 480)
        {
            std::println(std::cerr, "Display too small, must be at least 480 pixels high");
            exit(EXIT_FAILURE);
        }

        Boxworld boxworld{fitToScreen};
        boxworld.init();
        boxworld.draw(fb, font);

        //-----------------------------------------------------------------

        while (run)
        {
            js.read();

            if (js.buttonPressed(Joystick::BUTTON_START))
            {
                run = 0;
            }
            else
            {
                boxworld.update(js);
                boxworld.draw(fb, font);
                fb.update();
            }

            std::this_thread::sleep_for(250ms);
        }
    }
    catch (std::exception& error)
    {
        std::println(std::cerr, "Error: {} \n",error.what());
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    return 0 ;
}

