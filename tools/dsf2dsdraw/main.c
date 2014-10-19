#include <stdio.h>
#include <stdlib.h>
#include <dsf.h>
#include "sacd_reader.h"
#include "scarletbook_output.h"

#define DSF_BUFFER_SIZE    2048
typedef struct
{
    uint8_t            *header;
    size_t              header_size;
    uint8_t            *footer;
    size_t              footer_size;

    uint64_t            audio_data_size;

    int                 channel_count;
    uint64_t            sample_count;

    uint8_t             buffer[MAX_CHANNEL_COUNT][SACD_BLOCK_SIZE_PER_CHANNEL];
    uint8_t            *buffer_ptr[MAX_CHANNEL_COUNT];
} dsf_handle_t;

dsd_chunk_header_t * dsf_open(char * fname) {
  dsd_chunk_header_t * dsf_head = malloc(sizeof(dsd_chunk_header_t));
  fprintf(stdout, "converting %s\n", fname);
  FILE * fin = fopen(fname, "r");
  fread(dsf_head, sizeof(dsd_chunk_header_t), 1, fin);
  fmt_chunk_t * fmt = malloc(sizeof(fmt_chunk_t));
  fread(fmt, sizeof(fmt_chunk_t), 1, fin);
  fprintf(stderr, "channel count: %lu\n", fmt->channel_count);
  fprintf(stderr, "sample freq:  %u\n", fmt->sample_frequency);
  fprintf(stderr, "bps: %u\n", fmt->bits_per_sample);
  fprintf(stderr, "sample count:  %u\n", fmt->sample_count);

  data_chunk_t * dta = malloc(sizeof(data_chunk_t));
  fread(dta, sizeof(data_chunk_t), 1, fin);


  uint64_t newbuf_size = 4096;
  uint64_t blocks_per_buf = newbuf_size / fmt->block_size_per_channel;

  uint8_t *buf = malloc(newbuf_size * fmt->channel_count);
  uint8_t *nbuf = malloc(newbuf_size * fmt->channel_count);

  uint64_t cnt = 0;
  while (fread(buf, fmt->block_size_per_channel, fmt->channel_count, fin) != 0) {

    int c, b, i;
    i = 0;
    for (b = 0; b <= fmt->block_size_per_channel; b++) {
      for (c = 0; c <= fmt->channel_count; c++) {
        nbuf[c + b * fmt->channel_count] = *(buf+c*fmt->block_size_per_channel+b);
      }
    }
    fwrite(nbuf, newbuf_size, fmt->channel_count, stdout);
    cnt ++;
    if ((cnt & 0b11111) == 1) {
      fprintf(stderr, " %llu / %llu       \r",
          cnt * fmt->block_size_per_channel * 8, fmt->sample_count);
      fflush(stderr);
    }
  }

  return dsf_head;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s file.dsf\n", argv[0]);
    return -1;
  }
  dsf_open(argv[1]);
  return 0;
};
