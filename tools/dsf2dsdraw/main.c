#include <stdio.h>
#include <stdlib.h>
#include <dsf.h>
#include "sacd_reader.h"
#include "scarletbook_output.h"

// returns amount of converted samples for all channels
uint64_t convert(char * fname, int verbose) {
  dsd_chunk_header_t * dsf_head = malloc(sizeof(dsd_chunk_header_t));
  if (verbose) fprintf(stdout, "converting %s\n", fname);

  FILE * fin = fopen(fname, "r");
  fread(dsf_head, sizeof(dsd_chunk_header_t), 1, fin);
  fmt_chunk_t * fmt = malloc(sizeof(fmt_chunk_t));

  fread(fmt, sizeof(fmt_chunk_t), 1, fin);
  if (verbose) printf(stderr, "channel count: %lu\n", fmt->channel_count);
  if (verbose) printf(stderr, "sample freq:  %u\n", fmt->sample_frequency);
  if (verbose) printf(stderr, "bps: %u\n", fmt->bits_per_sample);
  if (verbose) printf(stderr, "sample count:  %u\n", fmt->sample_count);

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
    if (verbose && ((cnt & 0b11111) == 1)) {
      fprintf(stderr, " %llu / %llu       \r",
          cnt * fmt->block_size_per_channel * 8, fmt->sample_count);
      fflush(stderr);
    }
  }

  return cnt * fmt->block_size_per_channel * 8 * fmt->channel_count;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s [-q] file.dsf\n\n\t-q is quiet\nRead SONY DSF file and produce RAW multichannel DSD stream to stdout\n", argv[0]);
    return -1;
  }
  int verbose = (argc == 3) && (argc[1][0] == '-') && (argc[1][1] == 'q') ? 0 : 1;
  convert(argv[1], verbose);
  return 0;
};
