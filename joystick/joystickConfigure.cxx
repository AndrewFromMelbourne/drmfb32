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
#include <unistd.h>

#include <fmt/format.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

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

    std::array<std::string, fb32::Joystick::BUTTON_COUNT> descriptions{
        "X",
        "A",
        "B",
        "Y",
        "left shoulder",
        "right shoulder",
        "",
        "",
        "select",
        "start",
        "",
        "",
    };

    std::array<std::string, fb32::Joystick::BUTTON_COUNT> tokens =
    {
        "BUTTON_X",
        "BUTTON_A",
        "BUTTON_B",
        "BUTTON_Y",
        "BUTTON_LEFT_SHOULDER",
        "BUTTON_RIGHT_SHOULDER",
        "",
        "",
        "BUTTON_SELECT",
        "BUTTON_START",
        "",
        "",
    };

    //---------------------------------------------------------------------

    try
    {
        Joystick js(joystick, true);
        std::vector<std::string> configuration;

        for (int i = 0 ; i < descriptions.size() ; ++i)
        {
            if (descriptions[i].length() == 0)
            {
                continue;
            }

            fmt::print("Press and release {} button\n", descriptions[i]);

            bool found{false};

            while (not found)
            {
                js.read();

                for (int b = 0 ; b < js.numberOfButtons() ; ++b)
                {
                    if (js.buttonPressed(b))
                    {
                        const auto button = js.rawButton(b);
                        configuration.emplace_back(tokens[i] +
                                                   " = " +
                                                   std::to_string(button));
                        found = true;
                    }
                }
            }
        }

        //-----------------------------------------------------------------

        fmt::print("Write conifiguration file? [y/N]\n");

        std::string reply;
        std::cin >> reply;

        if (reply != "y")
        {
            return 0;
        }

        std::string configDirectory{std::getenv("HOME") +
                                    std::string{"/.config/drmfb32"}};


        if (std::filesystem::exists(configDirectory) or
            std::filesystem::create_directories(configDirectory))
        {
            std::string configFile{configDirectory + "/joystickButtons"};

            std::ofstream ofs{configFile.c_str()};

            if (ofs)
            {
                for (const auto entry : configuration)
                {
                    ofs << entry << '\n';
                }
            }
        }
    }
    catch (std::exception& error)
    {
        fmt::print(stderr, "Error: {}\n", error.what());
        exit(EXIT_FAILURE);
    }

    return 0;
}

