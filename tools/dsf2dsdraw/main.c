#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <dsf.h>
#include "sacd_reader.h"
#include "scarletbook_output.h"

enum {
  PAUSE, PLAY
};

void get_key_init(struct termios * orig_term_attr) {
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), orig_term_attr);
    memcpy(&new_term_attr, orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
}

int getkey() {

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    return fgetc(stdin);

}

void get_key_deinit(struct termios * orig_term_attr) {
    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, orig_term_attr);
}

// returns amount of converted samples for all channels
uint64_t convert(char * fname, int verbose, int ui) {
  dsd_chunk_header_t * dsf_head = malloc(sizeof(dsd_chunk_header_t));
  if (verbose && (!ui)) fprintf(stdout, "converting %s\n", fname);

  FILE * fin = fopen(fname, "r");
  fread(dsf_head, sizeof(dsd_chunk_header_t), 1, fin);
  fmt_chunk_t * fmt = malloc(sizeof(fmt_chunk_t));

  fread(fmt, sizeof(fmt_chunk_t), 1, fin);
  if (verbose && (!ui)) fprintf(stderr, "channel count: %u\n", fmt->channel_count);
  if (verbose && (!ui)) fprintf(stderr, "sample freq:  %u\n", fmt->sample_frequency);
  if (verbose && (!ui)) fprintf(stderr, "bps: %u\n", fmt->bits_per_sample);
  if (verbose && (!ui)) fprintf(stderr, "sample count:  %lu\n", fmt->sample_count);

  data_chunk_t * dta = malloc(sizeof(data_chunk_t));
  fread(dta, sizeof(data_chunk_t), 1, fin);


  uint64_t newbuf_size = 4096;

  uint8_t *buf = malloc(newbuf_size * fmt->channel_count);
  uint8_t *nbuf = malloc(newbuf_size * fmt->channel_count);

  int64_t cnt = 0;

  int state = PLAY;
  char *ccc[] = {" PAUSED", "PLAYING"};
  const long stream_begin = ftell(fin);

  while (fread(buf, fmt->block_size_per_channel, fmt->channel_count, fin) != 0) {

    uint64_t c, b;
    for (b = 0; b <= fmt->block_size_per_channel; b++) {
      for (c = 0; c <= fmt->channel_count; c++) {
        nbuf[c + b * fmt->channel_count] = *(buf+c*fmt->block_size_per_channel+b);
      }
    }
    fwrite(nbuf, newbuf_size, fmt->channel_count, stdout);
    cnt ++;
    long npos = 0;
    if (ui) {
      do {
        int cmd = getkey();
        switch (cmd) {
          case ' ': state = state == PAUSE ? PLAY : PAUSE; break;
          case ',': npos = ftell(fin) - 35 * fmt->block_size_per_channel * fmt->channel_count;
                    fseek(fin, npos < stream_begin ? stream_begin : npos, SEEK_SET);
                    cnt -= 35;
                    break;
          case '.': npos = ftell(fin) + 20 * fmt->block_size_per_channel * fmt->channel_count;
                    fseek(fin, npos >= fmt->sample_count + stream_begin ?
                        fmt->sample_count + stream_begin : npos, SEEK_SET);
                    cnt += 20;
                    break;
          case '<': npos = ftell(fin) - 215 * fmt->block_size_per_channel * fmt->channel_count;
                    fseek(fin, npos < stream_begin ? stream_begin : npos, SEEK_SET);
                    cnt -= 215;
                    break;
          case '>': npos = ftell(fin) + 200 * fmt->block_size_per_channel * fmt->channel_count;
                    fseek(fin, npos >= fmt->sample_count + stream_begin ?
                        fmt->sample_count + stream_begin : npos, SEEK_SET);
                    cnt += 200;
                    break;
          case 'n': fseek(fin, 0, SEEK_END); break;
          case 'h': fseek(fin, stream_begin, SEEK_SET); cnt = 0; break;
        }

        if (cnt > fmt->sample_count / fmt->block_size_per_channel)
          cnt = fmt->sample_count / fmt->block_size_per_channel;
        if (cnt < 0)
          cnt = 0;

        if ((verbose && (cnt & 0b111) == 1) || (verbose && (state == PAUSE))) {
          fprintf(stderr, " %s   % 4.3f / % 4.3f   %s      \r",
              ccc[state], cnt * fmt->block_size_per_channel * 8 / (double)fmt->sample_frequency,
              fmt->sample_count / (double)fmt->sample_frequency, fname);
        }
        if (state == PAUSE) usleep(1000);
      } while ((state == PAUSE));
    }

    if ((!ui) && verbose && ((cnt & 0b11111) == 1)) {
      fprintf(stderr, " %lu / %lu       \r",
          cnt * fmt->block_size_per_channel * 8, fmt->sample_count);
      fflush(stderr);
    }
  }

  fclose(fin);

  return cnt * fmt->block_size_per_channel * 8 * fmt->channel_count;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s [-q] [-u] file.dsf\n"
        "\t-u turns on UI and enables controls\n"
        "\t-q is quiet.\n"
        "Controls are: SPACE - toggle play/pause\n"
        "              , .   - rewind back/forth slow\n"
        "              < >   - rewind back/forth fast\n"
        "              n     - seek at the end of file\n"
        "              h     - seek at the start of file\n"
        "Read SONY DSF file and produce RAW multichannel DSD stream to stdout\n", argv[0]);
    return EXIT_FAILURE;
  }
  int verbose = (argc == 3) && (argv[1][0] == '-') && (argv[1][1] == 'q') ? 0 : 1;
  int ui = ((argc == 4) && (argv[2][0] == '-') && (argv[2][1] == 'u'))  ||
           ((argc == 3) && (argv[1][0] == '-') && (argv[1][1] == 'u'));

  struct termios o_attr;
  if (ui) get_key_init(&o_attr);

  convert(argv[argc-1], verbose, ui);

  if (ui) get_key_deinit(&o_attr);
  if (verbose) fprintf(stderr, "\n");

  return EXIT_SUCCESS;
};
