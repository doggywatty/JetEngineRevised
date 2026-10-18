// core/jet/include/stream/streamutil.h
#ifndef CORE_JET_INCLUDE_STREAM_STREAMUTIL_H
#define CORE_JET_INCLUDE_STREAM_STREAMUTIL_H

#include <memory>
#include <vector>
#include "core/pstring.h"

namespace jet {
namespace stream {

class IStream;
class IStreamFactory;
class IFileSystem;

String GetFixedCasePath(const String& path);  // lowercases the file name part
std::shared_ptr<IStreamFactory> CreateStreamFactoryFromPath(const String& path, const String& ext);
std::shared_ptr<IStreamFactory> CreateStreamFactoryFromPath(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext);
u32 ComputeCRC(const u8* data, unsigned size);
u32 ComputeCRC(IStream& s, unsigned size);
std::vector<String> EnumerateFiles(const String& dir, const String& filter);
std::vector<String> EnumerateFolders(const String& dir);
bool IsFile(const String& path);
bool IsDirectory(const String& path);
bool GetFileTime(const String& path, long& time);
bool SetFileTime(const String& path, const long& time);
bool MakeDirectory(const String& path);
bool DeleteDirectory(const String& path);
bool DeleteFile(const String& path);
int ReplaceBackslash(int c);
String GetNormalizedPath(const String& path);
bool Rename(const String& from, const String& to);

}
}

#endif
