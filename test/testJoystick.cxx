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
#include <unistd.h>

#include <fmt/format.h>

#include <cmath>
#include <iomanip>
#include <system_error>

#include "joystick.h"

//-------------------------------------------------------------------------

namespace
{
const char* defaultJoystick = "/dev/input/js0";
}

//-------------------------------------------------------------------------

using namespace fb32;

//-------------------------------------------------------------------------

void
printUsage(
    FILE* file,
    const std::string& name)
{
    fmt::print(file, "\n");
    fmt::print(file, "Usage: {} <options>\n", name);
    fmt::print(file, "\n");
    fmt::print(file, "    --help,-h - print usage and exit\n");
    fmt::print(file, "    --joystick,-j - joystick device\n");
    fmt::print(file, "\n");
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    std::string program = basename(argv[0]);
    std::string joystick = defaultJoystick;

    //---------------------------------------------------------------------

    static const char* sopts = "hj:";
    static option lopts[] =
    {
        { "help", no_argument, nullptr, 'h' },
        { "joystick", required_argument, nullptr, 'j' },
        { nullptr, no_argument, nullptr, 0 }
    };

    int opt{};

    while ((opt = ::getopt_long(argc, argv, sopts, lopts, nullptr)) != -1)
    {
        switch (opt)
        {
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

    try
    {
        Joystick js(joystick, true);

        while (1)
        {
            js.read();

            for (auto button = 0 ; button < js.numberOfButtons() ; ++button)
            {
                fmt::print(
                    "{:02x}:{}{} ",
                    button,
                    (js.buttonPressed(button)) ? 'X' : 'O',
                    (js.buttonDown(button)) ? 'D' : 'U');
            }

            for (auto axes = 0 ; axes < js.numberOfAxes() ; ++axes)
            {
                auto signCharacter = [](int value) -> char
                {
                    if (value < 0)
                    {
                        return '-';
                    }
                    else if (value > 0)
                    {
                        return '+';
                    }

                    return ' ';
                };

                const auto [x, y] = js.getAxes(axes);
                const char xSign = signCharacter(x);
                const char ySign = signCharacter(y);

                fmt::print(
                    "{}{:04x}:{}{:04x} ",
                    xSign,
                    static_cast<uint16_t>(std::abs(x)),
                    ySign,
                    static_cast<uint16_t>(std::abs(y)));
            }

            fmt::print("\n");
        }
    }
    catch (std::exception& error)
    {
        fmt::print(stderr, "Error: {}\n", error.what());
        exit(EXIT_FAILURE);
    }

    return 0;
}

