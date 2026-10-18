// libs/vox1.1/include/stb_vorbis.h
#ifndef LIBS_VOX1_1_INCLUDE_STB_VORBIS_H
#define LIBS_VOX1_1_INCLUDE_STB_VORBIS_H

struct stb_vorbis {  // sizeof 0x5A8
    unsigned sample_rate;  // 0x0
    int channels;  // 0x4
    unsigned setup_memory_required;  // 0x8
    unsigned temp_memory_required;  // 0xC
    unsigned setup_temp_memory_required;  // 0x10
    vox::StreamCursorInterface *f;  // 0x14
    unsigned f_start;  // 0x18
    int close_on_free;  // 0x1C
    unsigned char* stream;  // 0x20
    unsigned char* stream_start;  // 0x24
    unsigned char* stream_end;  // 0x28
    unsigned stream_len;  // 0x2C
    unsigned char push_mode;  // 0x30
    unsigned first_audio_page_offset;  // 0x34
    ProbedPage p_first;  // 0x38
    ProbedPage p_last;  // 0x4C
    stb_vorbis_alloc alloc;  // 0x60
    int setup_offset;  // 0x68
    int temp_offset;  // 0x6C
    int eof;  // 0x70
    STBVorbisError error;  // 0x74
    int blocksize[2];  // 0x78
    int blocksize_0;  // 0x80
    int blocksize_1;  // 0x84
    int codebook_count;  // 0x88
    Codebook *codebooks;  // 0x8C
    int floor_count;  // 0x90
    unsigned short floor_types[64];  // 0x94
    Floor *floor_config;  // 0x114
    int residue_count;  // 0x118
    unsigned short residue_types[64];  // 0x11C
    Residue *residue_config;  // 0x19C
    int mapping_count;  // 0x1A0
    Mapping *mapping;  // 0x1A4
    int mode_count;  // 0x1A8
    Mode mode_config[64];  // 0x1AC
    unsigned total_samples;  // 0x32C
    float* channel_buffers[16];  // 0x330
    float* outputs[16];  // 0x370
    float* previous_window[16];  // 0x3B0
    int previous_length;  // 0x3F0
    short* finalY[16];  // 0x3F4
    unsigned current_loc;  // 0x434
    int current_loc_valid;  // 0x438
    float* A[2];  // 0x43C
    float* B[2];  // 0x444
    float* C[2];  // 0x44C
    float* window[2];  // 0x454
    unsigned short* bit_reverse[2];  // 0x45C
    unsigned serial;  // 0x464
    int last_page;  // 0x468
    int segment_count;  // 0x46C
    unsigned char segments[255];  // 0x470
    unsigned char page_flag;  // 0x56F
    unsigned char bytes_in_seg;  // 0x570
    unsigned char first_decode;  // 0x571
    int next_seg;  // 0x574
    int last_seg;  // 0x578
    int last_seg_which;  // 0x57C
    unsigned acc;  // 0x580
    int valid_bits;  // 0x584
    int packet_bytes;  // 0x588
    int end_seg_with_known_loc;  // 0x58C
    unsigned known_loc_for_packet;  // 0x590
    int discard_samples_deferred;  // 0x594
    unsigned samples_output;  // 0x598
    int page_crc_tests;  // 0x59C
    int channel_buffer_start;  // 0x5A0
    int channel_buffer_end;  // 0x5A4
};

#endif
