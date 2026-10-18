// tests/test_vox.cpp
#undef NDEBUG
// Decodes sound files of the game through the vox decoders (the format of each is told by its extension) and prints what they
// hold; the RMS of the whole track says the audio is not silent:
//   test_vox [-w <out dir>] <file.mpc|.wav|.vxn> ...
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "vox_decoder_mpc8.h"
#include "vox_decoder_mswav.h"
#include "vox_decoder_native.h"

using namespace vox;

static void WriteWav(const std::string &path, const std::vector<short> &pcm, int channels, int rate) {
    FILE *o = fopen(path.c_str(), "wb");
    assert(o);
    const unsigned dataBytes = (unsigned)(pcm.size() * 2), byteRate = rate * channels * 2, riff = 36 + dataBytes, fmtLen = 16;
    const unsigned short fmt = 1, ch = (unsigned short)channels, ba = (unsigned short)(channels * 2), bits = 16;
    fwrite("RIFF", 1, 4, o); fwrite(&riff, 4, 1, o); fwrite("WAVEfmt ", 1, 8, o); fwrite(&fmtLen, 4, 1, o);
    fwrite(&fmt, 2, 1, o); fwrite(&ch, 2, 1, o); fwrite(&rate, 4, 1, o); fwrite(&byteRate, 4, 1, o); fwrite(&ba, 2, 1, o); fwrite(&bits, 2, 1, o);
    fwrite("data", 1, 4, o); fwrite(&dataBytes, 4, 1, o); fwrite(pcm.data(), 2, pcm.size(), o);
    fclose(o);
}

int main(int argc, char **argv) {
    std::string outDir;
    int first = 1;
    if (argc > 2 && !strcmp(argv[1], "-w")) { outDir = argv[2]; first = 3; }
    for (int i = first; i < argc; i++) {
        const std::string path = argv[i];
        const size_t dot = path.rfind('.');
        const std::string ext = dot == std::string::npos ? "" : path.substr(dot);
        StreamCFile stream(path.c_str());
        stream.Init();
        DecoderInterface *dec = ext == ".mpc" ? DecoderMPC8Factory(0) : ext == ".vxn" ? DecoderNativeFactory(0) : DecoderMSWavFactory(0);
        StreamCursorInterface *sc = stream.CreateNewCursor();
        DecoderCursorInterface *dc = dec->CreateNewCursor(sc);
        const TrackParams &tp = dc->GetTrackParams();
        std::vector<short> pcm;
        short buf[4096];
        for (;;) {
            const int n = dc->Decode(buf, sizeof(buf));
            if (n <= 0) break;
            pcm.insert(pcm.end(), buf, buf + n / 2);
            if (pcm.size() > 200u * 48000 * 2) break;
        }
        double sum = 0;
        for (short s : pcm) sum += (double)s * s;
        const double rms = pcm.empty() ? 0 : sqrt(sum / pcm.size());
        printf("%s: %d ch %d Hz %d bit, header %d samples, decoded %zu samples (%.2fs), rms %.1f\n", path.c_str(), tp.numChannels, tp.sampleRate,
               tp.bitsPerSample, tp.numSamples, tp.numChannels ? pcm.size() / tp.numChannels : 0, tp.numChannels && tp.sampleRate ? (double)pcm.size() / tp.numChannels / tp.sampleRate : 0.0, rms);
        if (!outDir.empty()) {
            const size_t slash = path.rfind('/');
            WriteWav(outDir + "/" + (slash == std::string::npos ? path : path.substr(slash + 1)) + ".wav", pcm, tp.numChannels, tp.sampleRate);
        }
        dec->DestroyCursor(dc);
        stream.DestroyCursor(sc);
        delete dec;
    }
    return 0;
}
