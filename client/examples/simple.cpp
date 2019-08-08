
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
  uint16_t screenHeight = 240;

  NetworkDisplayConfig displayConfig;

  displayConfig.frameRate = 60; // -1 to disable

  displayConfig.inputScreenWidth = screenWidth;
  displayConfig.inputScreenHeight = screenHeight;

  // 5 matrix strip. Each strip is FOUR matrices tall.
  displayConfig.singlePanelWidth = 64;
  displayConfig.singlePanelHeight = 64;

  displayConfig.segmentPanelsTall = 3;
  displayConfig.segmentPanelsWide = 1;

  displayConfig.totalPanelsWide = 5;
  displayConfig.totalPanelsTall = 3;

  displayConfig.totalSegments = 5;

  displayConfig.destinationPort = "9890";
  displayConfig.destinationIP = "10.0.1.20%i";
  displayConfig.destinationIpStartDigit = 1;

  displayConfig.outputScreenWidth = displayConfig.singlePanelWidth * displayConfig.totalPanelsWide;
  displayConfig.outputScreenHeight = displayConfig.singlePanelHeight * displayConfig.totalPanelsTall;


  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);


  std::thread(interrupterThread).detach();
  uint16_t color = 0;

  while (! interrupt_received) {
    memset(networkDisplay->GetInputBuffer(), color += 1, networkDisplay->GetInputBufferSize());
    networkDisplay->Update();
  }





  return 0;
}


