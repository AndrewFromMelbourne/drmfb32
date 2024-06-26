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

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <system_error>

#include "joystick.h"

//-------------------------------------------------------------------------

namespace
{

bool
readJoystickEvent(
    fb32::FileDescriptor& joystickFd,
    js_event& event)
{
    const auto bytes{::read(joystickFd.fd(), &event, sizeof(event))};
    return (bytes != -1) and (bytes == sizeof(event));
}

}

//-------------------------------------------------------------------------

fb32::Joystick:: Joystick(bool blocking)
:
    Joystick("/dev/input/js0", blocking)
{
}

//-------------------------------------------------------------------------

fb32::Joystick:: Joystick(const std::string& device, bool blocking)
:
    m_joystickFd{::open(device.c_str(), O_RDONLY | ((blocking) ? 0 : O_NONBLOCK))},
    m_blocking(blocking),
    m_buttonCount(0),
    m_joystickCount(0),
    m_buttons(),
    m_joysticks()
{
    init();
}

//-------------------------------------------------------------------------

void
fb32::Joystick:: init()
{
    if (m_joystickFd.fd() == -1)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "cannot open joystick device"};
    }

    char joystickCount = 0;
    if (ioctl(m_joystickFd.fd(), JSIOCGAXES, &joystickCount) == -1)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "reading number of axes"};
    }
    m_joystickCount = joystickCount / 2;

    char buttonCount = 0;
    if (ioctl(m_joystickFd.fd(), JSIOCGBUTTONS, &buttonCount) == -1)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "reading number of buttons"};
    }
    m_buttonCount = buttonCount;

    m_buttons.resize(m_buttonCount, ButtonState{ false, false });
    m_joysticks.resize(m_joystickCount, JoystickAxes{ 0, 0 });
}

//-------------------------------------------------------------------------

int
fb32::Joystick:: numberOfButtons() const
{
    return m_buttonCount;
}

//-------------------------------------------------------------------------

int
fb32::Joystick:: numberOfAxes() const
{
    return m_joystickCount;
}

//-------------------------------------------------------------------------

bool
fb32::Joystick:: buttonPressed(int button)
{
    if (button >= numberOfButtons())
    {
        return false;
    }

    const auto pressed = m_buttons.at(button).pressed;

    if (pressed)
    {
        m_buttons[button].pressed = false;
    }

    return pressed;
}

//-------------------------------------------------------------------------

bool
fb32::Joystick:: buttonDown(int button) const
{
    if (button >= numberOfButtons())
    {
        return false;
    }

    return m_buttons.at(button).down;
}

//-------------------------------------------------------------------------

fb32::JoystickAxes
fb32::Joystick:: getAxes(int joystickNumber) const
{
    return m_joysticks.at(joystickNumber);
}

//-------------------------------------------------------------------------

void
fb32::Joystick:: read()
{
    js_event event{};

    if (m_blocking)
    {
        if (readJoystickEvent(m_joystickFd, event))
        {
            process(event);
        }
    }
    else
    {
        while (readJoystickEvent(m_joystickFd, event))
        {
            process(event);
        }
    }
}

//-------------------------------------------------------------------------

void
 fb32::Joystick:: process(const js_event& event)
{
    switch (event.type)
    {
        case JS_EVENT_BUTTON:

            if (event.value)
            {
                m_buttons.at(event.number) = ButtonState{ true, true };
            }
            else
            {
                m_buttons.at(event.number).down = false;
            }

            break;

        case JS_EVENT_AXIS:
        {
            const auto axis{event.number / 2};

            if (axis < 3)
            {
                auto axes = m_joysticks.at(axis);

                if (event.number % 2 == 0)
                {
                    axes.x = event.value;
                }
                else
                {
                    axes.y = event.value;
                }

                m_joysticks[axis] = axes;
            }

            break;
        }
    }
}

