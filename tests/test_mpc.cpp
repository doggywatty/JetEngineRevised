// Decodes a .mpc (Musepack SV8) file through the vendored libmpcdec and writes a PCM16 .wav:
//   test_mpc <in.mpc> <out.wav>
#include <mpc/mpcdec.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>

static void WriteWav(const char *path, const std::vector<short> &pcm, unsigned channels, unsigned rate) {
    FILE *o = fopen(path, "wb");
    unsigned dataBytes = (unsigned)(pcm.size() * sizeof(short));
    unsigned byteRate = rate * channels * 2, blockAlign = channels * 2;
    unsigned chunkSize = 36 + dataBytes;
    fwrite("RIFF", 1, 4, o); fwrite(&chunkSize, 4, 1, o); fwrite("WAVE", 1, 4, o);
    fwrite("fmt ", 1, 4, o);
    unsigned fmtLen = 16; fwrite(&fmtLen, 4, 1, o);
    unsigned short fmt = 1; fwrite(&fmt, 2, 1, o);
    unsigned short ch = (unsigned short)channels; fwrite(&ch, 2, 1, o);
    fwrite(&rate, 4, 1, o); fwrite(&byteRate, 4, 1, o);
    unsigned short ba = (unsigned short)blockAlign; fwrite(&ba, 2, 1, o);
    unsigned short bits = 16; fwrite(&bits, 2, 1, o);
    fwrite("data", 1, 4, o); fwrite(&dataBytes, 4, 1, o);
    fwrite(pcm.data(), 2, pcm.size(), o);
    fclose(o);
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: test_mpc <in.mpc> <out.wav>\n"); return 1; }

    mpc_reader reader;
    if (mpc_reader_init_stdio(&reader, argv[1]) != MPC_STATUS_OK) { fprintf(stderr, "could not open %s\n", argv[1]); return 1; }

    mpc_demux *demux = mpc_demux_init(&reader);
    if (!demux) { fprintf(stderr, "mpc_demux_init failed\n"); return 1; }
    mpc_streaminfo info;
    mpc_demux_get_info(demux, &info);
    printf("channels=%u sample_freq=%u samples=%llu (approx %.2fs) avg_bitrate=%g kbps encoder='%s'\n",
           info.channels, info.sample_freq, (unsigned long long)info.samples,
           (double)info.samples / info.sample_freq, info.average_bitrate / 1000.0, info.encoder);

    std::vector<short> pcm;
    MPC_SAMPLE_FORMAT buf[MPC_DECODER_BUFFER_LENGTH];
    for (;;) {
        mpc_frame_info fi;
        fi.buffer = buf;
        mpc_status err = mpc_demux_decode(demux, &fi);
        if (err != MPC_STATUS_OK) { fprintf(stderr, "decode error %d\n", err); break; }
        if (fi.bits == -1) break; // end of stream
        unsigned n = fi.samples * info.channels;
        for (unsigned i = 0; i < n; i++) {
#ifdef MPC_FIXED_POINT
            long v = buf[i] >> (MPC_FIXED_POINT_SCALE_SHIFT - 15);
#else
            long v = (long)(buf[i] * 32767.0f);
#endif
            if (v > 32767) v = 32767; if (v < -32768) v = -32768;
            pcm.push_back((short)v);
        }
    }
    mpc_demux_exit(demux);
    mpc_reader_exit_stdio(&reader);
    printf("decoded %zu samples (%.2fs)\n", pcm.size() / info.channels, (double)(pcm.size() / info.channels) / info.sample_freq);
    WriteWav(argv[2], pcm, info.channels, info.sample_freq);
    printf("wrote %s\n", argv[2]);
    return 0;
}
