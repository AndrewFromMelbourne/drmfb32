
//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2023 Andrew Duncan
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

#include "image8880FreeType.h"

#include <stdexcept>

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

Image8880FreeType::Image8880FreeType()
:
    m_pixelSize{0},
    m_face(),
    m_library()
{
}

//-------------------------------------------------------------------------

Image8880FreeType::Image8880FreeType(
    const std::string& fontFile,
    int pixelSize)
:
    Image8880FreeType()
{
    if (FT_Init_FreeType(&m_library) != 0)
    {
        throw std::invalid_argument("FreeType initialization faied");
    }

    if (FT_New_Face(m_library, fontFile.c_str(), 0, &m_face) != 0)
    {
        throw std::invalid_argument("FreeType could not open " + fontFile);
    }

    setPixelSize(pixelSize);
}

//-------------------------------------------------------------------------

Image8880FreeType::~Image8880FreeType()
{
    FT_Done_FreeType(m_library);
}

//-------------------------------------------------------------------------

std::string
Image8880FreeType::getFontFamilyName() const
{
    return m_face->family_name;
}

//-------------------------------------------------------------------------

std::string
Image8880FreeType::getFontStyleName() const
{
    return m_face->style_name;
}

//-------------------------------------------------------------------------

int
Image8880FreeType::getPixelHeight() const
{
    return (m_face->size->metrics.ascender +
            abs(m_face->size->metrics.descender)) >> 6;
}

//-------------------------------------------------------------------------

int
Image8880FreeType::getPixelWidth() const
{
    return m_face->size->metrics.max_advance >> 6;
}

//-------------------------------------------------------------------------

bool
Image8880FreeType::setPixelSize(
    int pixelSize)
{
    if (pixelSize == m_pixelSize)
    {
        return true;
    }

    if (FT_Set_Pixel_Sizes(m_face, 0, pixelSize) == 0)
    {
        m_pixelSize = pixelSize;

        return true;
    }

    return false;
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawChar(
    const Interface8880Point& p,
    uint8_t c,
    const RGB8880& rgb,
    Interface8880& image)
{
    return drawString(p, std::string(1, c), rgb, image);
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawChar(
    const Interface8880Point& p,
    uint8_t c,
    uint32_t rgb,
    Interface8880& image)
{
    return drawChar(p, c, RGB8880(rgb), image);
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawString(
    const Interface8880Point& p,
    const char* string,
    const RGB8880& rgb,
    Interface8880& image)
{
    if (not string)
    {
        return p;
    }

    return drawString(p, std::string(string), rgb, image);
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawString(
    const Interface8880Point& p,
    const std::string& string,
    const RGB8880& rgb,
    Interface8880& image)
{
    Interface8880Point position{p};
    position.setY(position.y() + (m_face->size->metrics.ascender >> 6));

    auto slot = m_face->glyph;
    auto use_kerning = FT_HAS_KERNING(m_face);
    FT_UInt previous = 0;

    for (auto c : string)
    {
        if (c == '\n')
        {
            position.set(p.x(), position.y() + getPixelHeight());
        }
        else
        {
            auto glyph_index = FT_Get_Char_Index(m_face, c);

            if (use_kerning and previous and glyph_index)
            {
                FT_Vector delta;

                FT_Get_Kerning(m_face,
                               previous,
                               glyph_index,
                               ft_kerning_default,
                               &delta);

                position.setX(position.x() + (delta.x >> 6));
            }

            if (FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER) == 0)
            {
                auto slot = m_face->glyph;

                drawChar(position.x() + slot->bitmap_left,
                         position.y() - slot->bitmap_top,
                         slot->bitmap,
                         rgb,
                         image);


                position.setX(position.x() + (slot->advance.x >> 6));
                previous = glyph_index;
            }
        }
    }

    //-----------------------------------------------------------------

    auto advance = slot->bitmap.width - (slot->advance.x >> 6);

    if (advance > 0)
    {
        position.setX(position.x() + advance);
    }

    position.setY(position.y() - (m_face->size->metrics.ascender >> 6));

    return position;
}

//-------------------------------------------------------------------------

void
Image8880FreeType::drawChar(
        int xOffset,
        int yOffset,
        const FT_Bitmap& bitmap,
        const RGB8880& rgb,
        Interface8880& image)
{
    for (unsigned j = 0 ; j < bitmap.rows ; ++j)
    {
        uint8_t* row = bitmap.buffer + (j * bitmap.pitch);

        for (unsigned i = 0 ; i < bitmap.width ; ++i)
        {
            if (row[i] > 0)
            {
                const Interface8880Point p{static_cast<int>(i + xOffset),
                                       static_cast<int>(j + yOffset)};
                auto background = image.getPixelRGB(p);

                if (background.first)
                {
                    image.setPixelRGB(p,
                                      RGB8880::blend(row[i],
                                                     rgb,
                                                     background.second));
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

} // namespace fb32

