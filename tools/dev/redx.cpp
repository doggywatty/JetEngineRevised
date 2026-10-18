#include "stream/redundantstream.h"
#include "stream/filestream.h"
#include <cstdio>
#include <vector>
using namespace jet::stream;
int main(int, char** v)
{
    FileStream f(jet::String(v[1]), FileStream::k_in, false);
    RedundantStream r;
    r.SetSource(f, true);
    r.Start();
    printf("started %d size %u\n", r.IsStarted(), r.Size());
    std::vector<unsigned char> b(r.Size());
    r.Read(b.data(), unsigned(b.size()));
    for (size_t i = 0; i < 24 && i < b.size(); ++i) printf("%02x ", b[i]);
    printf("\n");
    FILE* o = fopen(v[2], "wb"); fwrite(b.data(), 1, b.size(), o); fclose(o);
    r.Finish();
}
