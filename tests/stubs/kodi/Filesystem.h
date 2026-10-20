#pragma once
#include <cstdio>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <sys/types.h>

constexpr int ADDON_READ_NO_CACHE = 0;
namespace fakekodi
{
inline std::string bufferPath;
inline bool denyWrites = false;
inline int segmentReadOpens = 0;
inline int writeCalls = 0;
inline size_t maxWriteSize = 0;
}
namespace kodi::addon
{
inline std::string GetSettingString(const std::string&) { return fakekodi::bufferPath; }
inline float GetSettingFloat(const std::string&) { return 0.25f; }
inline bool CheckSettingBoolean(const std::string&, bool& value) { value = true; return true; }
}
namespace kodi::vfs
{
inline bool FileExists(const std::string& p) { return std::filesystem::is_regular_file(p); }
inline bool DirectoryExists(const std::string& p) { return std::filesystem::is_directory(p); }
inline bool CreateDirectory(const std::string& p)
{
  if (fakekodi::denyWrites) return false;
  std::error_code e;
  return std::filesystem::create_directories(p, e) || (!e && DirectoryExists(p));
}
inline bool DeleteFile(const std::string& p)
{
  std::error_code e;
  return std::filesystem::remove(p, e);
}
inline bool GetDiskSpace(const std::string& p, uint64_t& capacity, uint64_t& free, uint64_t& available)
{
  std::error_code e;
  auto s = std::filesystem::space(p, e);
  if (e) return false;
  capacity = s.capacity; free = s.free; available = s.available;
  return true;
}
class CFile
{
  std::FILE* f = nullptr;
public:
  ~CFile() { Close(); }
  bool OpenFileForWrite(const std::string& p, bool)
  {
    Close();
    if (fakekodi::denyWrites) return false;
    f = std::fopen(p.c_str(), "w+b");
    return f != nullptr;
  }
  bool OpenFile(const std::string& p, int)
  {
    Close(); f = std::fopen(p.c_str(), "rb");
    if (f && p.size() > 4 && p.substr(p.size() - 4) == ".seg") ++fakekodi::segmentReadOpens;
    return f != nullptr;
  }
  bool IsOpen() const { return f != nullptr; }
  void Close() { if (f) std::fclose(f); f = nullptr; }
  int64_t Seek(int64_t p, int whence = SEEK_SET)
  {
    if (!f || std::fseek(f, p, whence)) return -1;
    return std::ftell(f);
  }
  ssize_t Write(const void* p, size_t n)
  {
    if (!f) return -1;
    ++fakekodi::writeCalls;
    if (fakekodi::maxWriteSize) n = std::min(n, fakekodi::maxWriteSize);
    auto wrote = std::fwrite(p, 1, n, f); std::fflush(f);
    return wrote;
  }
  ssize_t Read(void* p, size_t n) { return f ? std::fread(p, 1, n, f) : -1; }
  bool ReadLine(std::string& line)
  {
    line.clear(); if (!f) return false;
    int ch;
    while ((ch = std::fgetc(f)) != EOF) { if (ch == '\n') return true; line += char(ch); }
    return !line.empty();
  }
};
}
