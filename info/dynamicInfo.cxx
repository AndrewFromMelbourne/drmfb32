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

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

#include <ifaddrs.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

#include "dynamicInfo.h"
#include "system.h"

//-------------------------------------------------------------------------

std::string
DynamicInfo::getIpAddress(
    char& interface)
{
    interface = 'X';
    std::string address{"   .   .   .   "};

    ifaddrs* ifaddr{};
    ::getifaddrs(&ifaddr);

    if (not ifaddr)
    {
        return address;
    }

    for (auto ifa = ifaddr ; ifa != nullptr ; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            void *addr = &((sockaddr_in *)ifa->ifa_addr)->sin_addr;

            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                char buffer[INET_ADDRSTRLEN];
                ::inet_ntop(AF_INET, addr, buffer, sizeof(buffer));
                address = buffer;
                interface = ifa->ifa_name[0];
                break;
            }
        }
    }

    ::freeifaddrs(ifaddr);

    return address;
}

//-------------------------------------------------------------------------

void
DynamicInfo::drawIpAddress(
    fb32::Interface8880Point& position,
    fb32::Interface8880Font& font)
{
    position = font.drawString(position,
                               "ip(",
                               m_heading,
                               getImage());

    char interface = ' ';
    std::string ipaddress = getIpAddress(interface);

    position = font.drawChar(position,
                             interface,
                             m_foreground,
                             getImage());

    position = font.drawString(position,
                               ") ",
                               m_heading,
                               getImage());

    position = font.drawString(position,
                               ipaddress + " ",
                               m_foreground,
                               getImage());

}

//-------------------------------------------------------------------------

std::string
DynamicInfo::getTemperature()
{
    return std::to_string(inf::getTemperature());
}

//-------------------------------------------------------------------------

void
DynamicInfo::drawTemperature(
    fb32::Interface8880Point& position,
    fb32::Interface8880Font& font)
{
    position = font.drawString(position,
                               "temperature ",
                               m_heading,
                               getImage());

    std::string temperatureString{getTemperature()};

    position = font.drawString(position,
                               temperatureString,
                               m_foreground,
                               getImage());

    auto degreeSymbol = font.getCharacterCode(fb32::Interface8880Font::CharacterCode::DEGREE_SYMBOL);

    if (degreeSymbol)
    {
        position = font.drawChar(position,
                                *degreeSymbol,
                                m_foreground,
                                getImage());
    }

    position = font.drawString(position,
                               "C ",
                               m_foreground,
                               getImage());

}

//-------------------------------------------------------------------------

std::string
DynamicInfo::getTime(
    time_t now)
{
    tm result;
    tm *lt = ::localtime_r(&now, &result);

    char buffer[128];
    std::strftime(buffer, sizeof(buffer), "%T", lt);

    return buffer;
}

//-------------------------------------------------------------------------

void
DynamicInfo::drawTime(
    fb32::Interface8880Point& position,
    fb32::Interface8880Font& font,
    time_t now)
{
    position = font.drawString(position,
                               "time ",
                               m_heading,
                               getImage());

    std::string timeString = getTime(now);

    position = font.drawString(position,
                               timeString + " ",
                               m_foreground,
                               getImage());
}

//-------------------------------------------------------------------------

DynamicInfo::DynamicInfo(
    int width,
    int fontHeight,
    int yPosition)
:
    Panel{width, fontHeight + 4, yPosition},
    m_heading(255, 255, 0),
    m_foreground(255, 255, 255),
    m_warning(255, 0, 0),
    m_background(0, 0, 0)
{
}

//-------------------------------------------------------------------------

void
DynamicInfo::init(
    fb32::Interface8880Font& font)
{
}

//-------------------------------------------------------------------------

void
DynamicInfo::update(
    time_t now,
    fb32::Interface8880Font& font)
{
    getImage().clear(m_background);

    //---------------------------------------------------------------------

    fb32::Interface8880Point position = { 0, 0 };
    drawIpAddress(position, font);
    drawTime(position, font, now);
    drawTemperature(position, font);
}
