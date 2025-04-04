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

#include <ifaddrs.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

#include "dynamicInfo.h"
#include "system.h"

//=========================================================================

namespace
{

//-------------------------------------------------------------------------

struct IpAddress
{
    std::string address;
    char interface;
};

//-------------------------------------------------------------------------

IpAddress
getIpAddress()
{
    IpAddress result{ .address = "   .   .   .   ", .interface = 'X'};

    ifaddrs* ifaddr{};
    ::getifaddrs(&ifaddr);

    if (not ifaddr)
    {
        return result;
    }

    for (auto ifa = ifaddr ; ifa != nullptr ; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            void *addr = &((sockaddr_in *)ifa->ifa_addr)->sin_addr;

            if (std::string(ifa->ifa_name) != std::string("lo"))
            {
                char buffer[INET_ADDRSTRLEN];
                ::inet_ntop(AF_INET, addr, buffer, sizeof(buffer));
                result.address = buffer;
                result.interface = ifa->ifa_name[0];
                break;
            }
        }
    }

    ::freeifaddrs(ifaddr);

    return result;
}

//-------------------------------------------------------------------------

std::string
getTemperature()
{
    return std::to_string(inf::getTemperature());
}

//-------------------------------------------------------------------------

std::string
getTime(
    time_t now)
{
    tm result;
    tm *lt = ::localtime_r(&now, &result);

    char buffer[128];
    std::strftime(buffer, sizeof(buffer), "%T", lt);

    return buffer;
}

//-------------------------------------------------------------------------

} // namespace

//=========================================================================

void
DynamicInfo::drawIpAddress(
    fb32::Interface8880Point& position,
    fb32::Interface8880Font& font)
{
    position = font.drawString(position,
                               "ip(",
                               m_heading,
                               getImage());

    const auto ipaddress = getIpAddress();

    position = font.drawChar(position,
                             ipaddress.interface,
                             m_foreground,
                             getImage());

    position = font.drawString(position,
                               ") ",
                               m_heading,
                               getImage());

    position = font.drawString(position,
                               ipaddress.address + " ",
                               m_foreground,
                               getImage());

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

    const std::string temperatureString{getTemperature()};

    position = font.drawString(position,
                               temperatureString,
                               m_foreground,
                               getImage());

    const auto degreeCode{fb32::Interface8880Font::CharacterCode::DEGREE_SYMBOL};
    const auto degreeSymbol = font.getCharacterCode(degreeCode);

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

    const std::string timeString = getTime(now);

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
