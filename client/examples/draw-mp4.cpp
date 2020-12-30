#undef __USE_SDL2_VIDEO__
// This example doesn't work just yet.
//FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#define INPUT_BUFER_SIZE 4096
}


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
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt) {
  char buf[1024];
  int result;
  result = avcodec_send_packet(dec_ctx, pkt);

  if (result < 0) {
    fprintf(stderr, "Error sending a packet for decoding\n");
    exit(1);
  }
  printf("result = %i\n", result);

  while (result >= 0) {
    result = avcodec_receive_frame(dec_ctx, frame);
    if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
      return;
    else if (result < 0) {
      fprintf(stderr, "Error during decoding\n");
      exit(1);
    }
    printf("saving frame %3d\n", dec_ctx->frame_number);
    fflush(stdout);
    /* the picture is allocated by the decoder. no need to
       free it */
    snprintf(buf, sizeof(buf), "%d", dec_ctx->frame_number);
    printf("%s\n", buf);
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
//    fprintf(stderr, "Fatal Error! Please specify INI mp4File to open.\n\n");
//    exit(127);
//  }

  const char *mp4File = "draw-mp4.ini";
  NetworkDisplayConfig displayConfig = NetworkDisplay::GenerateConfig(mp4File);
  displayConfig.Describe();

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);

  const uint16_t screenWidth = networkDisplay->GetOutputScreenWidth(),
                 screenHeight = networkDisplay->GetOutputScreenHeight();

  // Detach the interrupter thread (handles CTRL+C)
  std::thread(interrupterThread).detach();

  const AVCodec *decoder;
  AVCodecParserContext *parserContext;
  AVCodecContext *codecContext = NULL;

  FILE *fileHandler;
  AVFrame *frame;
  uint8_t inputBuffer[INPUT_BUFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
  uint8_t *data;
  size_t dataSize;
  int ret;
  AVPacket *packet;
//  if (argc <= 2) {
//    fprintf(stderr, "Usage: %s <input mp4File> <output mp4File>\n"
//                    "And check your input mp4File is encoded by mpeg1video please.\n", argv[0]);
//    exit(0);
//  }
  const char* fileName = "modite-adventure.mp4";

  AVFormatContext *formatContext = avformat_alloc_context();
  if (! formatContext) {
    fprintf(stderr, "Could not allocate formatContext\n");
    exit(1);
  }

  if (avformat_open_input(&formatContext, fileName, NULL, NULL) != 0) {
    fprintf(stderr, "Could not open %s", fileName);
    exit(1);
  }

  printf("format %s, duration %lld us, bit_rate %lld", formatContext->iformat->name, formatContext->duration, formatContext->bit_rate);

  if (avformat_find_stream_info(formatContext, NULL) < 0) {
    fprintf(stderr, "Could not get stream information for %s\n", fileName);
    exit(1);
  }


  fileHandler = fopen(fileName, "rb");
  if (! fileHandler) {
    fprintf(stderr, "Could not open %s\n", fileName);
    exit(1);
  }

  /*---------------*/

  packet = av_packet_alloc();
  if (! packet) {
    fprintf(stderr, "Could not allocate packet!\n");
    exit(1);
  }

  /* set end of buffer to 0 (this ensures that no over-reading happens for damaged MPEG streams) */
  memset(inputBuffer + INPUT_BUFER_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

  int videoWidth = formatContext->streams[0]->codecpar->width,
      videoHeight = formatContext->streams[0]->codecpar->height;

  /* find the MPEG-4 video decoder */
  decoder = avcodec_find_decoder(formatContext->streams[0]->codecpar->codec_id);
  if (! decoder) {
    fprintf(stderr, "Could not allocate decoder!\n");
    exit(1);
  }

  parserContext = av_parser_init(decoder->id);
  if (! parserContext) {
    fprintf(stderr, "Could not allocate parserContext!\n");
    exit(1);
  }

  codecContext = avcodec_alloc_context3(decoder);
  if (! codecContext) {
    fprintf(stderr, "Could not allocate video decoder context\n");
    exit(1);
  }


  if (avcodec_open2(codecContext, decoder, NULL) < 0) {
    fprintf(stderr, "Could not open decoder\n");
    exit(1);
  }

  frame = av_frame_alloc();
  if (! frame) {
    fprintf(stderr, "Could not allocate video frame\n");
    exit(1);
  }

  frame->nb_samples = codecContext->frame_size;
  frame->format = codecContext->sample_fmt;
  frame->channel_layout = codecContext->channel_layout;


  uint16_t x = 0;
  // Loop while we have not been interrupted.
  while (! interrupt_received) {
    x++;

    while (! feof(fileHandler)) {
      if (interrupt_received) {
        break;
      }

      /* read raw data from the input mp4File */
      dataSize = fread(inputBuffer, 1, INPUT_BUFER_SIZE, fileHandler);

      if (!dataSize)
        break;

      /* use the parserContext to split the data into frames */
      data = inputBuffer;
      int got_image = 0;
      while (dataSize > 0) {
        if (interrupt_received) {
          break;
        }

        ret = av_parser_parse2(
          parserContext,
          codecContext,
          &packet->data,
          &packet->size,
          data,
          dataSize,
          AV_NOPTS_VALUE,
          AV_NOPTS_VALUE,
          0
        );

        if (ret < 0) {
          fprintf(stderr, "Error while parsing\n");
          exit(1);
        }
        data      += ret;
        dataSize -= ret;

        printf("dataSize %i\n", dataSize);

        if (packet->size) {

          decode(codecContext, frame, packet);
          networkDisplay->Update();
        }
      }
    }

//    interrupt_received = true;



  }


  fclose(fileHandler);
  av_parser_close(parserContext);
  avcodec_free_context(&codecContext);
  av_frame_free(&frame);
  av_packet_free(&packet);

  delete networkDisplay;

  return 0;
}


