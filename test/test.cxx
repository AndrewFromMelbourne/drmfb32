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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <system_error>
#include <thread>

#include <getopt.h>
#include <libgen.h>
#include <unistd.h>

#include "framebuffer8880.h"
#include "image8880.h"
#include "image8880Font8x16.h"
#include "image8880Graphics.h"
#include "point.h"

//-------------------------------------------------------------------------

using namespace fb32;
using namespace std::chrono_literals;

//-------------------------------------------------------------------------

#define TEST(expression, message) \
    if (!expression) \
    { \
        std::cerr \
            << __FILE__ "(" \
            << __LINE__ \
            << ") : " message " : " #expression " : test failed\n"; \
        exit(EXIT_FAILURE); \
    } \

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

    try
    {
        FrameBuffer8880 fb{device};
        fb.clear();

        //-----------------------------------------------------------------

        RGB8880 red{255, 0, 0};
        RGB8880 green{0, 255, 0};

        std::cout << "  red: 0x" << std::setfill('0') << std::setw(8) << std::hex << red.get8880() << "\n";
        std::cout << "green: 0x" << std::setfill('0') << std::setw(8) << std::hex << green.get8880() << "\n";

        //-----------------------------------------------------------------

        Image8880 image{48, 48};
        image.clear(red);

        auto rgb = image.getPixelRGB(Interface8880Point(0,0));

        TEST((rgb), "Image8880::getPixelRGB()");
        TEST((*rgb == red), "Image8880::getPixelRGB()");

        line(image,
             Interface8880Point(0,0),
             Interface8880Point(47,47),
             green);

        Interface8880Point imageLocation
        {
            (fb.getWidth() - image.getWidth()) / 2,
            (fb.getHeight() - image.getHeight()) / 2
        };

        fb.putImage(imageLocation, image);

        rgb = fb.getPixelRGB(imageLocation);

        TEST((rgb), "FrameBuffer8880::getPixelRGB()");
        TEST((*rgb == green), "FrameBuffer8880::getPixelRGB()");

        //-----------------------------------------------------------------

        RGB8880 darkBlue{0, 0, 63};
        RGB8880 white{255, 255, 255};

        std::cout << "Dblue: 0x" << std::setfill('0') << std::setw(8) << std::hex << darkBlue.get8880() << "\n";
        std::cout << "white: 0x" << std::setfill('0') << std::setw(8) << std::hex << white.get8880() << "\n";

        Image8880 textImage(168, 16);
        textImage.clear(darkBlue);

        Interface8880Point textLocation
        {
            (fb.getWidth() - textImage.getWidth()) / 2,
            (fb.getHeight() - textImage.getHeight()) / 3
        };

        Image8880Font8x16 font;

        font.drawString(
            Interface8880Point{0, 0},
            "This is a test string",
            white,
            textImage);

        fb.putImage(textLocation, textImage);

        //-----------------------------------------------------------------

        std::this_thread::sleep_for(10s);

        fb.clear();
    }
    catch (std::exception& error)
    {
        std::cerr << "Error: " << error.what() << "\n";
        exit(EXIT_FAILURE);
    }

    return 0;
}
