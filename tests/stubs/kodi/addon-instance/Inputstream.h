#pragma once
#include <cstdint>
#include <cstddef>

// Minimal Kodi host shim. The production ARM build uses the real pinned SDK.
constexpr double STREAM_TIME_BASE = 1000000.0;
constexpr double STREAM_NOPTS_VALUE = -1e300;
struct DEMUX_CRYPTO_INFO
{
  uint16_t numSubSamples = 0, flags = 0;
  uint16_t* clearBytes = nullptr;
  uint32_t* cipherBytes = nullptr;
  uint8_t iv[16]{}, kid[16]{};
};
struct DEMUX_PACKET
{
  uint8_t* pData = nullptr;
  int iSize = 0, iStreamId = 0, iGroupId = 0;
  int64_t demuxerId = 0;
  void* pSideData = nullptr;
  int iSideDataElems = 0;
  double pts = STREAM_NOPTS_VALUE, dts = STREAM_NOPTS_VALUE, duration = 0;
  int dispTime = 0;
  bool recoveryPoint = false;
  DEMUX_CRYPTO_INFO* cryptoInfo = nullptr;
};
