
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
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  uint16_t screenWidth = 320;
  uint16_t screenHeight = 256;

  NetworkDisplayConfig displayConfig;

  displayConfig.frameRate = 60; // -1 to disable
//  displayConfig.frameRate = -1; // Skip framerate

  displayConfig.inputScreenWidth = screenWidth;
  displayConfig.inputScreenHeight = screenHeight;

  // 5 matrix strip. Each strip is FOUR matrices tall.
  displayConfig.singlePanelWidth = 64;
  displayConfig.singlePanelHeight = 64;

  displayConfig.segmentPanelsTall = 4;
  displayConfig.segmentPanelsWide = 5;

  displayConfig.totalPanelsWide = 5;
  displayConfig.totalPanelsTall = displayConfig.segmentPanelsTall;

  displayConfig.totalSegments = 5;

  displayConfig.destinationPort = "9890";
  displayConfig.destinationIP = "10.1.10.20%i";
  displayConfig.destinationIpStartDigit = 1;

  displayConfig.outputScreenWidth = displayConfig.singlePanelWidth * displayConfig.totalPanelsWide;
  displayConfig.outputScreenHeight = displayConfig.singlePanelHeight * displayConfig.totalPanelsTall;


  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);


  std::thread(interrupterThread).detach();
  uint32_t color = 0;

  while (! interrupt_received) {
    printf("input = %lu, output = %lu\n", networkDisplay->GetTotalInputPixels(), networkDisplay->GetTotalOutputPixels());
    color--;
    uint16_t *inputBuffer = networkDisplay->GetInputBuffer();
    memset(networkDisplay->GetInputBuffer(), color++, networkDisplay->GetInputBufferSize());

//    for (uint16_t z = 0; z < networkDisplay->GetTotalInputPixels(); z++) {
//      inputBuffer[z] = color++;
//    }
    printf("color = %i, inputBufferSize = %lu\n", color, networkDisplay->GetInputBufferSize());
    networkDisplay->Update();
  }





  return 0;
}


