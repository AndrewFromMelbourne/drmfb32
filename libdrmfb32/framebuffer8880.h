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

#pragma once

//-------------------------------------------------------------------------

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "drmMode.h"
#include "point.h"
#include "fileDescriptor.h"
#include "interface8880.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

using Interface8880Point = Point<int32_t>;

//-------------------------------------------------------------------------

class Image8880;

//-------------------------------------------------------------------------

class FrameBuffer8880
:
    public Interface8880
{
public:

    //---------------------------------------------------------------------

    static constexpr std::size_t c_bytesPerPixel{4};

    //---------------------------------------------------------------------

    struct DumbBuffer
    {
        uint32_t* m_fbp{nullptr};
        uint32_t m_fbId{0};
        uint32_t m_fbHandle{0};
        int m_length{0};
        int m_lineLengthPixels{0};
    };

    //---------------------------------------------------------------------

    struct AtomicProperty
    {
        uint32_t m_objectId;
        uint32_t m_objectType;
        uint32_t m_propertyId;
        std::string m_propertyName;
        uint64_t m_value;
    };

    //---------------------------------------------------------------------

    explicit FrameBuffer8880(
        const std::string& device = "",
        uint32_t connectorId = 0);

    ~FrameBuffer8880() override;

    FrameBuffer8880(const FrameBuffer8880& fb) = delete;
    FrameBuffer8880& operator=(const FrameBuffer8880& fb) = delete;

    FrameBuffer8880(FrameBuffer8880&& fb) = delete;
    FrameBuffer8880& operator=(FrameBuffer8880&& fb) = delete;

    void clearBuffers(const RGB8880& rgb) { clearBuffers(rgb.get8880()); }
    void clearBuffers(uint32_t rgb = 0);

    [[nodiscard]] std::span<uint32_t> getBuffer() noexcept override;
    [[nodiscard]] std::span<const uint32_t> getBuffer() const noexcept override;

    [[nodiscard]] std::size_t getBufferSize() const noexcept;

    [[nodiscard]] drm::drmVersion_ptr getDrmVersion() noexcept { return drm::drmGetVersion(m_fd); }

    [[nodiscard]] int getWidth() const noexcept override { return m_width; }
    [[nodiscard]] int getHeight() const noexcept override { return m_height; }

    [[nodiscard]] bool hasAtomic() const noexcept { return m_hasAtomic; }
    [[nodiscard]] bool hasUniversalPlanes() const noexcept { return m_hasUniversalPlanes; }

    [[nodiscard]] std::size_t offset(const Interface8880Point& p) const noexcept override;

    void update();

private:

    void createDumbBuffer(int index);
    void destroyDumbBuffer(int index);
    void setDumbBuffer(int index);

    void
    addAtomicProperties(
        drm::drmModeAtomicReq_ptr& atomicRequest,
        uint32_t fbId);
    void
    addAtomicRequest(
        uint32_t objectId,
        uint32_t objectType,
        const std::string& propertyName,
        uint64_t value);
    void createAtomicRequests();

    void findResources(uint32_t connectorId);

    [[nodiscard]] bool useAtomic() const noexcept { return m_hasAtomic and m_hasUniversalPlanes; }

    int m_width;
    int m_height;

    FileDescriptor m_fd;

    std::array<DumbBuffer, 2> m_dbs;
    int m_dbFront;
    int m_dbBack;

    bool m_hasAtomic;
    bool m_hasUniversalPlanes;
    std::vector<AtomicProperty> m_atomicProperties;
    uint32_t m_blobId;
    uint32_t m_connectorId;
    uint32_t m_crtcId;
    uint32_t m_planeId;
    drmModeModeInfo m_mode;
    drm::drmModeCrtc_ptr m_originalCrtc;
};

//-------------------------------------------------------------------------

} // namespace fb32

