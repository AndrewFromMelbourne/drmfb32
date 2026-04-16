//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2026 Andrew Duncan
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

#include <sys/wait.h>

#include <array>
#include <atomic>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <system_error>

#include "joystick.h"
#include "tokenize.h"

//-------------------------------------------------------------------------

using namespace fb32;

//-------------------------------------------------------------------------

namespace
{
pid_t childPid{};
const char* defaultJoystick = "/dev/input/js0";
}

//-------------------------------------------------------------------------

void waitForChild(pid_t pid)
{
    while (waitpid(pid, nullptr, 0) == -1)
    {
    }
}

//-------------------------------------------------------------------------

static void
signalHandler(
    int signalNumber) noexcept
{
    if (childPid == 0)
    {
        exit(EXIT_SUCCESS);
    }

    switch (signalNumber)
    {
    case SIGCHLD:

        waitForChild(childPid);
        exit(EXIT_SUCCESS);
        break;

    case SIGINT:
    case SIGTERM:

        kill(childPid, SIGTERM);
        waitForChild(childPid);
        exit(EXIT_SUCCESS);
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
    std::println(stream, "    --command,-c - command to run");
    std::println(stream, "    --help,-h - print usage and exit");
    std::println(stream, "    --joystick,-j - joystick device");
    std::println(stream, "");
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    std::string command;
    const std::string program = basename(argv[0]);
    std::string joystick = defaultJoystick;

    //---------------------------------------------------------------------

    static const char* sopts = "c:hj:";
    static option lopts[] =
    {
        { "command", required_argument, nullptr, 'c' },
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

            command = optarg;
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

    if (command.empty())
    {
        std::println(std::cerr, "No command specified");
        ::exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    try
    {
        auto pid = fork();

        if (pid == -1)
        {
            throw std::system_error(errno, std::generic_category(), "fork");
        }
        else if (pid == 0)
        {
            auto argsv = split(command);
            std::vector<std::string> args{argsv.begin(), argsv.end()};

            std::vector<char*> argcs;
            std::transform(
                   cbegin(args),
                   cend(args),
                   std::back_inserter(argcs),
                   [](const std::string& s){ return const_cast<char*>(s.c_str()); });

            argcs.push_back(nullptr);

            execvp(argcs[0], argcs.data());

            throw std::system_error(errno, std::generic_category(), "execvp");
        }

        //-----------------------------------------------------------------

        for (auto signal : { SIGCHLD, SIGINT, SIGTERM })
        {
            struct sigaction sa{};

            sa.sa_handler = signalHandler;
            sa.sa_flags = 0;

            if (sigaction(signal, &sa, nullptr) == -1)
            {
                std::println(
                    std::cerr,
                    "Error: installing {} signal handler : {}",
                    strsignal(signal),
                    strerror(errno));

                ::exit(EXIT_FAILURE);
            }
        }

        //-----------------------------------------------------------------

        Joystick js(joystick, Joystick::ReadType::BLOCKING);

        while (true)
        {
            js.read();
            if (js.buttonPressed(Joystick::BUTTON_START))
            {
                break;
            }
        }

        kill(childPid, SIGTERM);
        waitForChild(childPid);
    }
    catch (std::exception& error)
    {
        std::println(std::cerr, "Error: {}", error.what());
        exit(EXIT_FAILURE);
    }
}

