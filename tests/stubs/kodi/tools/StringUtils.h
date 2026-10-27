#pragma once
#include <cstdio>
#include <string>
#include <vector>
namespace kodi::tools
{
struct StringUtils
{
  template<class... Args> static std::string Format(const char* fmt, Args... args)
  {
    int n = std::snprintf(nullptr, 0, fmt, args...);
    std::vector<char> b(n + 1);
    std::snprintf(b.data(), b.size(), fmt, args...);
    return std::string(b.data(), n);
  }
  static bool EndsWith(const std::string& s, const std::string& tail)
  { return s.size() >= tail.size() && s.compare(s.size()-tail.size(), tail.size(), tail) == 0; }
  static std::vector<std::string> Split(const std::string& s, const std::string& delimiter)
  {
    std::vector<std::string> result;
    size_t start = 0, end;
    while ((end = s.find(delimiter, start)) != std::string::npos)
    { result.push_back(s.substr(start, end-start)); start = end+delimiter.size(); }
    result.push_back(s.substr(start)); return result;
  }
};
}
