#undef __USE_SDL2_VIDEO__

//FFMPEG
#include <libavcodec/avcodec.h>
#define INBUF_SIZE 4096

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <thread>
#include <unistd.h>

#include "NetworkDisplay.h"
#include "NetworkDisplayConfig.h"

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}


void interrupterThread() {
  printf("Hit CTRL + C to end!\n");

  while(true) {
    if (interrupt_received) {
      exit(0);
    }
    usleep(1000);
  }
}


// Copied from https://ffmpeg.org/doxygen/trunk/decode_video_8c-example.html (and modified)
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, const char *filename) {
  char buf[1024];
  int ret;
  ret = avcodec_send_packet(dec_ctx, pkt);
  if (ret < 0) {
    fprintf(stderr, "Error sending a packet for decoding\n");
    exit(1);
  }
  while (ret >= 0) {
    ret = avcodec_receive_frame(dec_ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return;
    else if (ret < 0) {
      fprintf(stderr, "Error during decoding\n");
      exit(1);
    }
    printf("saving frame %3d\n", dec_ctx->frame_number);
    fflush(stdout);
    /* the picture is allocated by the decoder. no need to
       free it */
    snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
//    pgm_save(frame->data[0], frame->linesize[0],
//             frame->width, frame->height, buf);
  }
}

int main(int argc, char* argv[]) {

//  for (int i = 0; i < argc; i++) {
//    printf("arg %i\t%s\n", i, argv[i]);
//  };
//
//  if (argc < 2) {
//    fprintf(stderr, "Fatal Error! Please specify INI file to open.\n\n");
//    exit(127);
//  }

  const char *file = "draw-mp4.ini";
  NetworkDisplayConfig displayConfig = NetworkDisplay::GenerateConfig(file);
  displayConfig.Describe();

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);

  const uint16_t screenWidth = networkDisplay->GetOutputScreenWidth(),
                 screenHeight = networkDisplay->GetOutputScreenHeight();

  // Detach the interrupter thread (handles CTRL+C)
  std::thread(interrupterThread).detach();

  const AVCodec *codec;
  AVCodecParserContext *parser;
  AVCodecContext *c = NULL;

  FILE *f;
  AVFrame *frame;
  uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
  uint8_t *data;
  size_t data_size;
  int ret;
  AVPacket *pkt;
  if (argc <= 2) {
    fprintf(stderr, "Usage: %s <input file> <output file>\n"
                    "And check your input file is encoded by mpeg1video please.\n", argv[0]);
    exit(0);
  }
  const char* filename = "modite-adventure.mp4";

  pkt = av_packet_alloc();
  if (!pkt)
    exit(1);
  /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
  memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
  /* find the MPEG-1 video decoder */
  codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
  if (!codec) {
    fprintf(stderr, "Codec not found\n");
    exit(1);
  }
  parser = av_parser_init(codec->id);
  if (!parser) {
    fprintf(stderr, "parser not found\n");
    exit(1);
  }
  c = avcodec_alloc_context3(codec);
  if (!c) {
    fprintf(stderr, "Could not allocate video codec context\n");
    exit(1);
  }
  /* For some codecs, such as msmpeg4 and mpeg4, width and height
     MUST be initialized there because this information is not
     available in the bitstream. */
  /* open it */
  if (avcodec_open2(c, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }
  f = fopen(filename, "rb");
  if (!f) {
    fprintf(stderr, "Could not open %s\n", filename);
    exit(1);
  }
  frame = av_frame_alloc();
  if (!frame) {
    fprintf(stderr, "Could not allocate video frame\n");
    exit(1);
  }


  // Loop while we have not been interrupted.
  while (! interrupt_received) {




    networkDisplay->Update();
  }


  fclose(f);
  av_parser_close(parser);
  avcodec_free_context(&c);
  av_frame_free(&frame);
  av_packet_free(&pkt);

  delete networkDisplay;

  return 0;
}


