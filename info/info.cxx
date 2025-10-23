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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <syslog.h>
#include <unistd.h>

#include <bsd/libutil.h>


#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <array>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <print>
#include <string_view>
#include <thread>
#include <vector>

#include "image8880FreeType.h"

#include "cpuTrace.h"
#include "dynamicInfo.h"
#include "framebuffer8880.h"
#include "networkTrace.h"
#include "memoryTrace.h"
#include "temperatureTrace.h"

//-------------------------------------------------------------------------

using namespace std::chrono_literals;

//-------------------------------------------------------------------------

namespace
{
volatile static std::sig_atomic_t run{1};
volatile static std::sig_atomic_t display{1};
}

//-------------------------------------------------------------------------

void
messageLog(
    bool isDaemon,
    const std::string& name,
    int priority,
    const std::string& message)
{
    if (isDaemon)
    {
        ::syslog(LOG_MAKEPRI(LOG_USER, priority), "%s", message.c_str());
    }
    else
    {
        std::print(std::cerr, "{}[{}]:", name, getpid());

        switch (priority)
        {
        case LOG_DEBUG:

            std::print(std::cerr, "debug");
            break;

        case LOG_INFO:

            std::print(std::cerr, "info");
            break;

        case LOG_NOTICE:

            std::print(std::cerr, "notice");
            break;

        case LOG_WARNING:

            std::print(std::cerr, "warning");
            break;

        case LOG_ERR:

            std::print(std::cerr, "error");
            break;

        default:

            std::print(std::cerr, "unknown({})", priority);
            break;
        }

        std::print(std::cerr, ":{}\n", message);
    }
}

//-------------------------------------------------------------------------

void
perrorLog(
    bool isDaemon,
    const std::string& name,
    const std::string& s)
{
    messageLog(isDaemon, name, LOG_ERR, s + " - " + ::strerror(errno));
}

//-------------------------------------------------------------------------


void
printUsage(
    std::ostream& stream,
    const std::string& name)
{
    std::println(stream, "");
    std::println(stream, "Usage: {}", name);
    std::println(stream, "");
    std::println(stream, "    --daemon,-D - start in the background as a daemon");
    std::println(stream, "    --connector,-c - dri connector to use");
    std::println(stream, "    --device,-d - dri device to use");
    std::println(stream, "    --font,-f - font file to use");
    std::println(stream, "    --help,-h - println usage and exit");
    std::println(stream, "    --pidfile,-p <pidfile> - create and lock PID file (if being run as a daemon)");
    std::println(stream, "");
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

    case SIGUSR1:

        display = 0;
        break;

    case SIGUSR2:

        display = 1;
        break;
    };
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    uint32_t connector{0};
    std::string device{};
    const std::string program{basename(argv[0])};
    std::string fontFile{};
    char* pidfile{};
    bool isDaemon{false};

    //---------------------------------------------------------------------

    static const char* sopts = "c:d:f:hp:D";
    static option lopts[] =
    {
        { "connector", required_argument, nullptr, 'c' },
        { "device", required_argument, nullptr, 'd' },
        { "font", required_argument, nullptr, 'f' },
        { "help", no_argument, nullptr, 'h' },
        { "pidfile", required_argument, nullptr, 'p' },
        { "daemon", no_argument, nullptr, 'D' },
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

            fontFile = optarg;

            break;

        case 'h':

            printUsage(std::cout, program);
            ::exit(EXIT_SUCCESS);

            break;

        case 'p':

            pidfile = optarg;

            break;

        case 'D':

            isDaemon = true;

            break;

        default:

            printUsage(std::cerr, program);
            ::exit(EXIT_FAILURE);

            break;
        }
    }

    //---------------------------------------------------------------------

    pidfh* pfh{};

    if (isDaemon)
    {
        if (pidfile)
        {
            pid_t otherpid;
            pfh = ::pidfile_open(pidfile, 0600, &otherpid);

            if (not pfh)
            {
                std::println(
                    std::cerr,
                    "{} is already running with pid {}",
                    program,
                    otherpid);
                ::exit(EXIT_FAILURE);
            }
        }

        if (::daemon(0, 0) == -1)
        {
            std::println(std::cerr, "Cannot daemonize");

            if (pfh)
            {
                ::pidfile_remove(pfh);
            }

            ::exit(EXIT_FAILURE);
        }

        if (pfh)
        {
            ::pidfile_write(pfh);
        }

        ::openlog(program.c_str(), LOG_PID, LOG_USER);
    }

    //---------------------------------------------------------------------

    for (auto signal : { SIGINT, SIGTERM, SIGUSR1, SIGUSR2 })
    {
        if (std::signal(signal, signalHandler) == SIG_ERR)
        {
            if (pfh)
            {
                ::pidfile_remove(pfh);
            }

            std::println(
                std::cerr,
                "Error: installing {} signal handler : {}",
                strsignal(signal),
                strerror(errno));

            ::exit(EXIT_FAILURE);
        }
    }

    //---------------------------------------------------------------------

    std::unique_ptr<fb32::Interface8880Font> font{std::make_unique<fb32::Image8880Font8x16>()};

    if (not fontFile.empty())
    {
        try
        {
            font = std::make_unique<fb32::Image8880FreeType>(fontFile, 16);
        }
        catch (std::exception& error)
        {
            std::println(std::cerr, "Warning: {}", error.what());
        }
    }

    //---------------------------------------------------------------------

    try
    {
        fb32::FrameBuffer8880 fb(device, connector);

        fb.clearBuffers(fb32::RGB8880{0, 0, 0});

        //-----------------------------------------------------------------

        using Panels = std::vector<std::unique_ptr<Panel>>;

        auto panelTop = [](const Panels& panels) -> int
        {
            if (panels.empty())
            {
                return 0;
            }
            else
            {
                return panels.back()->getBottom();
            }
        };

        //-----------------------------------------------------------------

        constexpr int traceHeight = 100;
        constexpr int gridHeight = traceHeight / 5;

        Panels panels;

        panels.push_back(
            std::make_unique<DynamicInfo>(fb.getWidth(),
                                          font->getPixelHeight(),
                                          panelTop(panels)));

        panels.push_back(
            std::make_unique<CpuTrace>(fb.getWidth(),
                                       traceHeight,
                                       font->getPixelHeight(),
                                       panelTop(panels),
                                       gridHeight));

        panels.push_back(
            std::make_unique<MemoryTrace>(fb.getWidth(),
                                          traceHeight,
                                          font->getPixelHeight(),
                                          panelTop(panels),
                                          gridHeight));

        if (fb.getHeight() >= 400)
        {
            panels.push_back(
            std::make_unique<NetworkTrace>(fb.getWidth(),
                                           traceHeight,
                                           font->getPixelHeight(),
                                           panelTop(panels),
                                           gridHeight));
        }

        //-----------------------------------------------------------------

        for (auto& panel : panels)
        {
            panel->init(*font);
        }

        //-----------------------------------------------------------------

        std::this_thread::sleep_for(1s);

        while (run)
        {
            const auto now = std::chrono::system_clock::now();
            const auto now_t = std::chrono::system_clock::to_time_t(now);

            for (auto& panel : panels)
            {
                panel->update(now_t, *font);

                if (display)
                {
                    panel->show(fb);
                }
            }

            fb.update();

            const auto nextSecond = std::chrono::round<std::chrono::seconds>(now) + 1s;
            std::this_thread::sleep_until(nextSecond);
        }

        fb.clearBuffers();
    }
    catch (std::exception& error)
    {
        std::println(std::cerr, "Warning: {}", error.what());
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    messageLog(isDaemon, program, LOG_INFO, "exiting");

    if (isDaemon)
    {
        ::closelog();
    }

    if (pfh)
    {
        ::pidfile_remove(pfh);
    }

    //---------------------------------------------------------------------

    return 0 ;
}
