//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2025 Andrew Duncan
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

#pragma once

//-------------------------------------------------------------------------

#include "image8880.h"
#include "interface8880.h"
#include "rgb8880.h"
#include "point.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

[[nodiscard]] Image8880
boxBlur(
    const Interface8880& input,
    int radius);

[[nodiscard]] Image8880
enlighten(
    const Interface8880& input,
    double strength);

[[nodiscard]] Image8880
maxRGB(
    const Interface8880& input);

[[nodiscard]] Image8880
resizeBilinearInterpolation(
    const Interface8880& input,
    int width,
    int height);

[[nodiscard]] Image8880
resizeLanczos3Interpolation(
    const Interface8880& input,
    int width,
    int height);

[[nodiscard]] Image8880
resizeNearestNeighbour(
    const Interface8880& input,
    int width,
    int height);

Image8880&
resizeToBilinearInterpolation(
    const Interface8880& input,
    Image8880& output);

Image8880&
resizeToLanczos3Interpolation(
    const Interface8880& input,
    Image8880& output);

Image8880&
resizeToNearestNeighbour(
    const Interface8880& input,
    Image8880& output);

[[nodiscard]] Image8880
scaleUp(
    const Interface8880& input,
    uint8_t scale);

//-------------------------------------------------------------------------

} // namespace fb32

