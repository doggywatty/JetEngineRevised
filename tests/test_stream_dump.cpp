// Prints a (decrypted) stream of a data dir: test_stream_dump <data dir> <stream name> [encryption suffix]
#include "stream/streammgr.h"
#include "stream/istream.h"
#include <cstdio>
#include <string>
#include <vector>
using namespace jet;
int main(int argc, char **argv) {
    stream::StreamMgr &sm = stream::StreamMgr::GetInstance();
    sm.AddPath(String(argv[1]), String());
    if (argc > 3) sm.AddEncryptionPair(String("ERROR: invalid stream"), String(argv[3]));
    stream::IStream *s = sm.CreateStream(String(argv[2]));
    if (!s) return 1;
    s->Start();
    std::vector<char> b(s->Size());
    unsigned n = s->Read(b.data(), b.size());
    fwrite(b.data(), 1, n, stdout);
    return 0;
}
