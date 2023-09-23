#pragma once

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

#include <vector>

//-------------------------------------------------------------------------

static constexpr int tileWidth = 30;
static constexpr int tileHeight = 30;
static constexpr int tileCount = 15;

static constexpr int batteryWidth = 14;
static constexpr int batteryHeight = 8;

//-------------------------------------------------------------------------

extern std::vector<uint32_t> emptyImage;
extern std::vector<uint32_t> passageImage;
extern std::vector<uint32_t> boxImage;
extern std::vector<uint32_t> playerImage;
extern std::vector<uint32_t> wallImage;
extern std::vector<uint32_t> passageWithTargetImage;
extern std::vector<uint32_t> boxOnTargetImage;
extern std::vector<uint32_t> playerOnTargetImage;

extern std::vector<uint32_t> batteryImage;

