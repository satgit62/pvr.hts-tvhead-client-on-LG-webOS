#include "stream/TimeshiftBuffer.h"
#include "stream/TimeshiftSegment.h"
#include "stream/url/URL.h"
#include "utils/Log.h"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

void Log(LogLevel, const char*, ...) {}
std::string CURL::GetRedacted(const std::string& path) { return path; }
using namespace ffmpegdirect;
void require(bool value, const char* message)
{ if (!value) throw std::runtime_error(message); }
struct PacketManager : IManageDemuxPacket
{
  DEMUX_PACKET* AllocateDemuxPacketFromInputStreamAPI(int n) override
  { auto p = new DEMUX_PACKET{}; p->pData = new uint8_t[n]; p->iSize = n; return p; }
  DEMUX_PACKET* AllocateEncryptedDemuxPacketFromInputStreamAPI(int n, unsigned int) override
  { return AllocateDemuxPacketFromInputStreamAPI(n); }
  void FreeDemuxPacketFromInputStreamAPI(DEMUX_PACKET* p) override
  { if (p) { delete[] p->pData; delete p; } }
  DEMUX_PACKET* packet(int second)
  {
    auto p = AllocateDemuxPacketFromInputStreamAPI(sizeof(second));
    std::memcpy(p->pData, &second, sizeof(second));
    p->pts = p->dts = second * STREAM_TIME_BASE;
    return p;
  }
};
struct TemporaryDirectory
{
  std::string path;
  TemporaryDirectory()
  {
    char p[] = "/tmp/webos-timeshift-test-XXXXXX";
    auto result = mkdtemp(p); require(result, "mkdtemp failed"); path = result;
    fakekodi::bufferPath = path; fakekodi::denyWrites = false; fakekodi::segmentReadOpens = 0; fakekodi::maxWriteSize = 0;
  }
  ~TemporaryDirectory() { std::filesystem::remove_all(path); }
};
void diskSeek()
{
  TemporaryDirectory d; PacketManager manager;
  {
    TimeshiftBuffer buffer(&manager); require(buffer.Start("stream"), "buffer start failed");
    for (int s = 0; s < 180; ++s) buffer.AddPacket(manager.packet(s));
    require(buffer.Seek(30000), "seek into evicted segment failed");
    require(fakekodi::segmentReadOpens > 0, "older rewind data is still resident in RAM");
    auto p = buffer.ReadPacket();
    require(p && p->pts == 30 * STREAM_TIME_BASE, "disk seek landed at segment boundary instead of requested second");
    manager.FreeDemuxPacketFromInputStreamAPI(p);
    for (int second : {7, 59, 25, 130, 1, 172, 44})
    {
      require(buffer.Seek(second * 1000), "repeated seek failed");
      p = buffer.ReadPacket(); require(p && p->pts == second * STREAM_TIME_BASE, "repeated seek returned wrong packet");
      manager.FreeDemuxPacketFromInputStreamAPI(p);
    }
  }
  for (const auto& entry : std::filesystem::directory_iterator(d.path))
    require(entry.path().extension() != ".seg" && entry.path().extension() != ".idx", "buffer files leaked on stop");
}
void reloadSegment()
{
  TemporaryDirectory d; PacketManager manager;
  {
    TimeshiftSegment writer(&manager, "reload", 0, d.path);
    for (int s = 0; s < 12; ++s) writer.AddPacket(manager.packet(s));
    writer.MarkAsComplete();
    auto p = writer.ReadPacket(); manager.FreeDemuxPacketFromInputStreamAPI(p);
    writer.ClearPackets(); writer.ForceLoadSegment();
    p = writer.ReadPacket(); require(p && p->pts == 0, "clearing packets did not reset read cursor");
    manager.FreeDemuxPacketFromInputStreamAPI(p);
  }
  TimeshiftSegment reader(&manager, "reload", 0, d.path); reader.ForceLoadSegment();
  require(reader.Seek(7000), "reloaded segment has no seek index");
  auto p = reader.ReadPacket(); require(p && p->pts == 7 * STREAM_TIME_BASE, "reloaded segment sought to wrong packet");
  manager.FreeDemuxPacketFromInputStreamAPI(p);
}
void unavailableStorage()
{
  TemporaryDirectory d; PacketManager manager; fakekodi::denyWrites = true;
  TimeshiftBuffer buffer(&manager);
  require(!buffer.Start("missing-usb"), "unwritable USB accepted");
  require(!buffer.Seek(0), "empty buffer accepted seek");
  buffer.SetPaused(true); buffer.SetPaused(false);
}
void batchedWrites()
{
  TemporaryDirectory d; PacketManager manager;
  TimeshiftSegment writer(&manager, "batched", 0, d.path);
  fakekodi::writeCalls = 0;
  for (int s = 0; s < 10; ++s) writer.AddPacket(manager.packet(s));
  require(fakekodi::writeCalls == 10, "packet serialization performed tiny field writes");
  writer.MarkAsComplete(); writer.ClearPackets(); writer.ForceLoadSegment();
  for (int s = 0; s < 10; ++s)
  {
    auto p = writer.ReadPacket();
    require(p && p->pts == s * STREAM_TIME_BASE && p->iSize == sizeof(s), "batched serialization changed packet fields");
    int payload = -1; std::memcpy(&payload, p->pData, sizeof(payload));
    require(payload == s, "batched serialization changed packet payload");
    manager.FreeDemuxPacketFromInputStreamAPI(p);
  }
  // The VFS may return a short write. The remaining bytes must be retried.
  TimeshiftSegment shortWriter(&manager, "short", 0, d.path);
  fakekodi::maxWriteSize = 7;
  shortWriter.AddPacket(manager.packet(9));
  shortWriter.MarkAsComplete(); shortWriter.ClearPackets(); shortWriter.ForceLoadSegment();
  auto p = shortWriter.ReadPacket();
  require(p && p->pts == 9 * STREAM_TIME_BASE, "short write truncated serialized packet");
  manager.FreeDemuxPacketFromInputStreamAPI(p);
  fakekodi::maxWriteSize = 0;
}
void emptySegment()
{
  TemporaryDirectory d; PacketManager manager;
  TimeshiftSegment segment(&manager, "empty", 0, d.path);
  require(!segment.Seek(0), "empty segment accepted seek");
}
int main(int argc, char** argv)
{
  const std::pair<const char*, void(*)()> tests[] = {
    {"disk-seek", diskSeek}, {"reload-segment", reloadSegment},
    {"batched-writes", batchedWrites}, {"unavailable-storage", unavailableStorage}, {"empty-segment", emptySegment}
  };
  for (const auto& test : tests)
  {
    if (argc > 1 && std::string(argv[1]) != test.first) continue;
    try { test.second(); std::cout << "PASS " << test.first << '\n'; }
    catch (const std::exception& e) { std::cerr << "FAIL " << test.first << ": " << e.what() << '\n'; return 1; }
  }
}
