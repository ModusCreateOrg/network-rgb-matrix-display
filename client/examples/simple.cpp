
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
  for (int i = 0; i < argc; i++) {
    printf("arg %i\t%s\n", i, argv[i]);
  };

  if (argc < 2) {
    fprintf(stderr, "Fatal Error! Please specify INI file to open.\n\n");
    exit(127);
  }
  NetworkDisplayConfig displayConfig = NetworkDisplay::GenerateConfig(argv[2]);


//  const char *file = "simple.ini"; // This is used for debugging within CLion
//  NetworkDisplayConfig displayConfig = NetworkDisplay::GenerateConfig(file);

  displayConfig.Describe();

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);


  std::thread(interrupterThread).detach();

  uint16_t nY = 0;
  uint16_t nX = 0;
  uint16_t color = 0;

  while (! interrupt_received) {
    color++;
    uint16_t *inputBuffer = networkDisplay->GetInputBuffer();
    // Fills the screen with color. that's it.
    memset(inputBuffer, color, networkDisplay->GetInputBufferSize());

    nX++;

    networkDisplay->Update();
  }


  delete networkDisplay;


  return 0;
}


