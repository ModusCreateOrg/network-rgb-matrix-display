
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

uint16_t color = random() % UINT16_MAX;

int main(int argc, char* argv[]) {

  for (int i = 0; i < argc; i++) {
    printf("arg %i\t%s\n", i, argv[i]);
  };

  if (argc < 2) {
    fprintf(stderr, "Fatal Error! Please specify INI file to open.\n\n");
    exit(127);
  }

  NetworkDisplayConfig displayConfig = NetworkDisplay::GenerateConfig(argv[1]);

  displayConfig.Dump();


  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);


  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);


  std::thread(interrupterThread).detach();

  while (! interrupt_received) {
    color = random() & UINT16_MAX;
//    printf("input = %lu, output = %lu\n", networkDisplay->GetTotalInputPixels(), networkDisplay->GetTotalOutputPixels());
    uint16_t *inputBuffer = networkDisplay->GetInputBuffer();
    memset(inputBuffer, color++, networkDisplay->GetInputBufferSize());
//    printf("Color %i\n", color);

//    for (uint16_t z = 0; z < networkDisplay->GetTotalInputPixels(); z++) {
//      inputBuffer[z] = color++;
//    }
//    printf("color = %i, inputBufferSize = %lu\n", color, networkDisplay->GetInputBufferSize());
    networkDisplay->Update();
  }




  return 0;
}


