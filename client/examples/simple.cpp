
#undef __USE_SDL2_VIDEO__


#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <time.h>
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


int main(int argc, char* argv[]) {

//
//
//  for (int i = 0; i < argc; i++) {
//    printf("arg %i\t%s\n", i, argv[i]);
//  };
//
//  if (argc < 2) {
//    fprintf(stderr, "Fatal Error! Please specify INI file to open.\n\n");
//    exit(127);
//  }

  const char *file = "simple.ini";

  NetworkDisplayConfig displayConfig = NetworkDisplay::GenerateConfig(file);

  displayConfig.Dump();

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);


  std::thread(interrupterThread).detach();
//
//  while (! interrupt_received) {
//    color++;// random() & UINT16_MAX;
////    printf("input = %lu, output = %lu\n", networkDisplay->GetTotalInputPixels(), networkDisplay->GetTotalOutputPixels());
//    uint16_t *inputBuffer = networkDisplay->GetInputBuffer();
//    memset(inputBuffer, color++, networkDisplay->GetInputBufferSize());
////    printf("Color %i\n", color);
////
//    for (unsigned short z = 0; z < networkDisplay->GetTotalInputPixels(); z++) {
//      inputBuffer[z] = random() & UINT16_MAX;
//    }
////    printf("color = %i, inputBufferSize = %lu\n", color, networkDisplay->GetInputBufferSize());
//    networkDisplay->Update();
////    usleep(10000);
//  }

//  uint16_t width = networkDisplay->
  uint16_t nY = 0;
  uint16_t nX = 0;
  uint16_t color = random() % UINT16_MAX;

  while (! interrupt_received) {
    color++;// random() & UINT16_MAX;
//    printf("input = %lu, output = %lu\n", networkDisplay->GetTotalInputPixels(), networkDisplay->GetTotalOutputPixels());
    uint16_t *inputBuffer = networkDisplay->GetInputBuffer();
    bzero(inputBuffer, networkDisplay->GetInputBufferSize());

//    printf("Color %i\n", color);
//

    const uint16_t  screenWidth = networkDisplay->GetOutputScreenWidth(),
                    screenHeight = networkDisplay->GetInputScreenHeight();


    if (nX > screenWidth) {
      nX = 0;
      nY++;
      if (nY > screenHeight) {
        nY = 0;
      }
    }

    inputBuffer[nX + (nY * screenWidth)] = color;


    nX++;


//    printf("nX(%i) nY(%i)\n", nX, nY);


    uint16_t width = networkDisplay->GetOutputScreenWidth(),
             height = networkDisplay->GetInputScreenHeight();
//    for (uint16_t i = 0; i < width; i++) {
//      inputBuffer[i] = color;
//    }

//    memset(inputBuffer, color, networkDisplay->GetInputBufferSize());

//    for (unsigned short z = 0; z < networkDisplay->GetTotalInputPixels(); z++) {
//      inputBuffer[z] = random() & UINT16_MAX;
//    }
//    printf("color = %i, inputBufferSize = %lu\n", color, networkDisplay->GetInputBufferSize());
    networkDisplay->Update();
//    usleep(10000 * 10);
  }


  delete networkDisplay;


  return 0;
}


