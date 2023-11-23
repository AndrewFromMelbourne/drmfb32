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

#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
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
    std::ostream& os,
    const std::string& name)
{
    os << "\n";
    os << "Usage: " << name << " <options>\n";
    os << "\n";
    os << "    --device,-d - dri device to use\n";
    os << "    --help,-h - print usage and exit\n";
    os << "\n";
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    std::string device = "";
    std::string program = basename(argv[0]);

    //---------------------------------------------------------------------

    static const char* sopts = "d:h";
    static struct option lopts[] =
    {
        { "device", required_argument, nullptr, 'd' },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, no_argument, nullptr, 0 }
    };

    int opt = 0;

    while ((opt = ::getopt_long(argc, argv, sopts, lopts, nullptr)) != -1)
    {
        switch (opt)
        {
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
            std::string message {"installing "};
            message += strsignal(signal);
            message += " signal handler";

            perror(message.c_str());
            ::exit(EXIT_FAILURE);
        }
    }
    //---------------------------------------------------------------------

    try
    {
        FrameBuffer8880 fb(device);
        fb.clear(RGB8880{0, 0, 0});

        std::cout
            << "width = "
            << fb.getWidth()
            << " height = "
            << fb.getHeight()
            << "\n";

        Sphere sphere(fb.getHeight() - 10);
        sphere.init();
        sphere.update();
        sphere.draw(fb);

        //-----------------------------------------------------------------

        while (run)
        {
            std::this_thread::sleep_for(1s);
        }

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
