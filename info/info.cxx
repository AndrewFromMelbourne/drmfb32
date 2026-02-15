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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <print>
#include <string_view>
#include <thread>
#include <vector>

#include "image8880FreeType.h"

#include "cpuTrace.h"
#include "dynamicInfo.h"
#include "framebuffer8880.h"
#include "info.h"
#include "networkTrace.h"
#include "memoryTrace.h"
#include "temperatureTrace.h"

//-------------------------------------------------------------------------

using namespace std::chrono_literals;

//-------------------------------------------------------------------------

Info::Info(
    std::atomic<bool>* display,
    std::atomic<bool>* run)
:
    m_connector(0),
    m_device{},
    m_display(display),
    m_fb(nullptr),
    m_font(nullptr),
    m_fontConfig(),
    m_hostname(),
    m_isDaemon(false),
    m_panels(),
    m_pidFile{},
    m_programName{},
    m_run(run)
{
}

//-------------------------------------------------------------------------

pidFile_ptr
Info::daemonize()
{
    pidFile_ptr pfh{nullptr, &pidfile_remove};

    if (not m_pidFile.empty())
    {
        pid_t otherpid;
        pfh.reset(::pidfile_open(m_pidFile.c_str(), 0600, &otherpid));

        if (not pfh)
        {
            messageLog(
                LOG_ERR,
                std::format(
                    "{} is already running with pid {}",
                    m_programName,
                    otherpid));
            ::exit(EXIT_FAILURE);
        }
    }

    if (::daemon(0, 0) == -1)
    {
        messageLog(LOG_ERR, "Cannot daemonize");
        ::exit(EXIT_FAILURE);
    }

    if (pfh)
    {
        ::pidfile_write(pfh.get());
    }

    return pfh;
}

//-------------------------------------------------------------------------

std::string
Info::getHostname()
{
    char hostname[256];
    if (::gethostname(hostname, sizeof(hostname)) == 0)
    {
        return hostname;
    }
    else
    {
        perrorLog("Error getting hostname");
        return "localhost";
    }
}

//-------------------------------------------------------------------------

void
Info::init()
{
    setFontConfig();

    m_fb = std::make_unique<fb32::FrameBuffer8880>(m_device, m_connector);
    m_fb->clearBuffers();

    //-----------------------------------------------------------------

    constexpr int traceHeight = 100;
    constexpr int gridHeight = traceHeight / 5;

    m_panels.push_back(
        std::make_unique<DynamicInfo>(m_fb->getWidth(),
                                      m_font->getPixelHeight(),
                                      panelTop()));

    m_panels.push_back(
        std::make_unique<CpuTrace>(m_fb->getWidth(),
                                   traceHeight,
                                   m_font->getPixelHeight(),
                                   panelTop(),
                                   gridHeight));

    m_panels.push_back(
        std::make_unique<MemoryTrace>(m_fb->getWidth(),
                                      traceHeight,
                                      m_font->getPixelHeight(),
                                      panelTop(),
                                      gridHeight));

    if (m_fb->getHeight() >= 400)
    {
        m_panels.push_back(
            std::make_unique<NetworkTrace>(m_fb->getWidth(),
                                           traceHeight,
                                           m_font->getPixelHeight(),
                                           panelTop(),
                                           gridHeight));
    }

    //-----------------------------------------------------------------

    for (auto& panel : m_panels)
    {
        panel->init(*m_font);
    }
}

//-------------------------------------------------------------------------

void
Info::messageLog(
    int priority,
    std::string_view message)
{
    m_hostname = getHostname();

    if (m_isDaemon)
    {
        std::string messageString(message);
        ::syslog(LOG_MAKEPRI(LOG_USER, priority), "%s", messageString.c_str());
    }
    else
    {
        const auto now = floor<std::chrono::seconds>(std::chrono::system_clock::now());
        const auto localTime = std::chrono::current_zone()->to_local(now);

        std::print(std::cerr, "{:%b %e %T} ", localTime);
        std::print(std::cerr, "{} ", m_hostname);
        std::print(std::cerr, "{}[{}]:", m_programName, getpid());

        const static std::map<int, std::string> priorityMap
        {
            { LOG_EMERG, "emergency" },
            { LOG_ALERT, "alert" },
            { LOG_CRIT, "critical" },
            { LOG_ERR, "error" },
            { LOG_WARNING, "warning" },
            { LOG_NOTICE, "notice" },
            { LOG_INFO, "info" },
            { LOG_DEBUG, "debug" }
        };

        if (const auto it = priorityMap.find(priority); it != priorityMap.end())
        {
            std::print(std::cerr, "{}", it->second);
        }
        else
        {
            std::print(std::cerr, "unknown({})", priority);
        }

        std::println(std::cerr, ":{}", message);
    }
}

//-------------------------------------------------------------------------

int
Info::panelTop() const
{
    if (m_panels.empty())
    {
        return 0;
    }
    else
    {
        return m_panels.back()->getBottom();
    }
}

//-------------------------------------------------------------------------

std::optional<int>
Info::parseCommandLine(
    int argc,
    char* argv[])
{
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

            m_connector = std::stol(optarg);
            break;

        case 'd':

            m_device = optarg;
            break;

        case 'f':

            m_fontConfig = fb32::parseFontConfig(optarg, 16);
            break;

        case 'h':

            printUsage(std::cout);
            return EXIT_SUCCESS;

        case 'p':

            m_pidFile = optarg;
            break;

        case 'D':

            m_isDaemon = true;
            break;

        default:

            printUsage(std::cerr);
            return EXIT_FAILURE;
        }
    }

    return std::nullopt;
}

//-------------------------------------------------------------------------

void
Info::perrorLog(
    std::string_view s)
{
    messageLog(LOG_ERR, std::string(s) + " - " + ::strerror(errno));
}

//-------------------------------------------------------------------------

void
Info::printUsage(
    std::ostream& stream)
{
    std::println(stream, "");
    std::println(stream, "Usage: {}", m_programName);
    std::println(stream, "");
    std::println(stream, "    --daemon,-D - start in the background as a daemon");
    std::println(stream, "    --connector,-c - dri connector to use");
    std::println(stream, "    --device,-d - dri device to use");
    std::println(stream, "    --font,-f - font file to use[:pixel height]");
    std::println(stream, "    --help,-h - println usage and exit");
    std::println(stream, "    --pidfile,-p <pidfile> - create and lock PID file (if being run as a daemon)");
    std::println(stream, "");
}

//-------------------------------------------------------------------------

void
Info::run()
{
    init();
    std::this_thread::sleep_for(1s);
    messageLog(LOG_INFO, "starting");

    while (*m_run)
    {
        const auto now = std::chrono::system_clock::now();
        const auto now_t = std::chrono::system_clock::to_time_t(now);

        if (*m_display)
        {
            if (not m_fb->isMaster())
            {
                m_fb->masterSet();
                messageLog(LOG_INFO, "display enabled");
            }

            for (auto& panel : m_panels)
            {
                panel->update(now_t, *m_font);
                panel->show(*m_fb);
            }

            m_fb->update();

        }
        else
        {
            if (m_fb->isMaster())
            {
                m_fb->masterDrop();
                messageLog(LOG_INFO, "display disabled");
            }

            for (auto& panel : m_panels)
            {
                panel->update(now_t, *m_font);
            }
        }

        const auto nextSecond = std::chrono::round<std::chrono::seconds>(now) + 1s;
        std::this_thread::sleep_until(nextSecond);
    }

    messageLog(LOG_INFO, "exiting");
}

//-------------------------------------------------------------------------


void
Info::setFontConfig() noexcept
{
    if (not m_fontConfig.m_fontFile.empty())
    {
        try
        {
            m_font = std::make_unique<fb32::Image8880FreeType>(m_fontConfig);
        }
        catch (std::exception& error)
        {
            messageLog(
                LOG_WARNING,
                std::format(
                    "Error: loading font {} : {}",
                    m_fontConfig.m_fontFile,
                    error.what()));
        }
    }

    if (not m_font)
    {
        m_font = std::make_unique<fb32::Image8880Font8x16>();
    }
}

