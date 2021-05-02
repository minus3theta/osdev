#include "frame_buffer.hpp"

#include "error.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include <algorithm>
#include <cstring>
#include <memory>

namespace {
int BytesPerPixel(PixelFormat format) {
  switch (format) {
  case kPixelRGBResv8BitPerColor:
    return 4;
  case kPixelBGRResv8BitPerColor:
    return 4;
  }
  return -1;
}
uint8_t *FrameAddrAt(Vector2D<int> pos, const FrameBufferConfig &config) {
  return config.frame_buffer +
         BytesPerPixel(config.pixel_format) *
             (config.pixels_per_scan_line * pos.y + pos.x);
}

int BytesPerScanLine(const FrameBufferConfig &config) {
  return BytesPerPixel(config.pixel_format) * config.pixels_per_scan_line;
}

Vector2D<int> FrameBufferSize(const FrameBufferConfig &config) {
  return {static_cast<int>(config.horizontal_resolution),
          static_cast<int>(config.vertical_resolution)};
}
} // namespace

Error FrameBuffer::Initialize(const FrameBufferConfig &config) {
  this->config = config;

  const auto bytes_per_pixel = BytesPerPixel(config.pixel_format);
  if (bytes_per_pixel <= 0) {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  if (config.frame_buffer) {
    buffer.resize(0);
  } else {
    buffer.resize(bytes_per_pixel * config.horizontal_resolution *
                  config.vertical_resolution);
    this->config.frame_buffer = buffer.data();
    this->config.pixels_per_scan_line = config.horizontal_resolution;
  }

  switch (config.pixel_format) {
  case kPixelRGBResv8BitPerColor:
    writer = std::make_unique<RGBResv8BitPerColorPixelWriter>(this->config);
    break;
  case kPixelBGRResv8BitPerColor:
    writer = std::make_unique<BGRResv8BitPerColorPixelWriter>(this->config);
    break;
  default:
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  return MAKE_ERROR(Error::kSuccess);
}

Error FrameBuffer::Copy(Vector2D<int> dst_pos, const FrameBuffer &src,
                        const Rectangle<int> &src_area) {
  if (config.pixel_format != src.config.pixel_format) {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  const auto bytes_per_pixel = BytesPerPixel(config.pixel_format);
  if (bytes_per_pixel <= 0) {
    return MAKE_ERROR(Error::kUnknownPixelFormat);
  }

  const Rectangle<int> src_area_shifted{dst_pos, src_area.size};
  const Rectangle<int> src_outline{dst_pos - src_area.pos,
                                   FrameBufferSize(src.config)};
  const Rectangle<int> dst_outline{{0, 0}, FrameBufferSize(config)};
  const auto copy_area = dst_outline & src_outline & src_area_shifted;
  const auto src_start_pos = copy_area.pos - (dst_pos - src_area.pos);

  uint8_t *dst_buf = FrameAddrAt(copy_area.pos, config);
  const uint8_t *src_buf = FrameAddrAt(src_start_pos, src.config);

  for (int y = 0; y < copy_area.size.y; ++y) {
    memcpy(dst_buf, src_buf, bytes_per_pixel * copy_area.size.x);
    dst_buf += BytesPerScanLine(config);
    src_buf += BytesPerScanLine(src.config);
  }

  return MAKE_ERROR(Error::kSuccess);
}

void FrameBuffer::Move(Vector2D<int> dst_pos, const Rectangle<int> &src) {
  const auto bytes_per_pixel = BytesPerPixel(config.pixel_format);
  const auto bytes_per_scan_line = BytesPerScanLine(config);

  if (dst_pos.y < src.pos.y) {
    uint8_t *dst_buf = FrameAddrAt(dst_pos, config);
    const uint8_t *src_buf = FrameAddrAt(src.pos, config);
    for (int y = 0; y < src.size.y; ++y) {
      memcpy(dst_buf, src_buf, bytes_per_pixel * src.size.x);
      dst_buf += bytes_per_scan_line;
      src_buf += bytes_per_scan_line;
    }
  } else {
    uint8_t *dst_buf =
        FrameAddrAt(dst_pos + Vector2D<int>{0, src.size.y - 1}, config);
    const uint8_t *src_buf =
        FrameAddrAt(src.pos + Vector2D<int>{0, src.size.y - 1}, config);
    for (int y = 0; y < src.size.y; ++y) {
      memcpy(dst_buf, src_buf, bytes_per_pixel * src.size.x);
      dst_buf -= bytes_per_scan_line;
      src_buf -= bytes_per_scan_line;
    }
  }
}
