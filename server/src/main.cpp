/*
@Author Jay Garcia https://github.com/jaygarcia/

@Purpose: Is the main program for an RGB Matrix "Strip".
          See How_it_works.md for more details
*/

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include <cstring>
#include <pthread.h>
#include <time.h>

#include "led-matrix.h"
#include "pixel-mapper.h"
#include "graphics.h"

#include <assert.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>

#include "NetworkServer.h"
#include "MatrixSegment.h"

using std::min;
using std::max;
using boost::asio::ip::tcp;
using namespace rgb_matrix;


pthread_mutex_t bufferMutex;

// Change the following constants to your liking. The number of bytes that are sent
// will need to match readBufferSize!
const uint16_t singlePanelWidth = 64;
const uint16_t singlePanelHeight = 64;
const uint16_t totalSinglePanelSize = (singlePanelHeight * singlePanelWidth);
const uint16_t numMatricesWide = 4;
const uint16_t numMatricesTall = 1; // Should be 4!!
const uint16_t totalPixels = totalSinglePanelSize * numMatricesWide * numMatricesTall;
const size_t readBufferSize = totalPixels * sizeof(uint16_t);



/***********************************************************************/

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

NetworkServer *server;
double priorAverage = 0;

int retries = 0;

bool shouldQuit = false;
MatrixSegment *matrixStrip = NULL;

// This function will loop and help exit gracefully if an interrupt is received.
void interrupterThread() {

  while(1) {
    if (interrupt_received) {
      server->StopThread();
      exit(1);
    }

//    server->LockMutex();
//    printf("avg %f      \r", server->mAverage);

    if (server->mAverage == priorAverage) {
      retries ++;
      if (retries > 1) {
        server->mAverage = 0;
        retries = 0;
        matrixStrip->ClearBuffers();
//        printf("**CLEAR BUFFERS!! retries=%i\n",retries);
      }

    }

    priorAverage = server->mAverage;
//    server->UnlockMutex();
    usleep(5000000);
  }
}

/***********************************************************************/


void start_matrix() {

  RGBMatrix::Options matrix_options;


  // I hard coded options here. You'll need to change this per your own specs!
  matrix_options.chain_length = numMatricesWide;
  matrix_options.cols = singlePanelWidth;
  matrix_options.rows = singlePanelHeight;
  matrix_options.parallel = 1;
  // 0 progressive, 1 interlaced
//  matrix_options.scan_mode = 1;
  matrix_options.show_refresh_rate = true;
#ifdef __MODUS_PI_VERSION_3__
  matrix_options.pwm_bits = 8;
#endif

#ifdef __MODUS_PI_VERSION_4__
  matrix_options.pwm_bits = 8;
#endif


  rgb_matrix::RuntimeOptions runtime_opt;

  // Detect raspberry pi 4 & slow down GPIO
#ifdef __MODUS_PI_VERSION_4__
  printf("Raspberry Pi 4 detected!\n");
  runtime_opt.gpio_slowdown = 3;
#endif


  RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) {
    printf("ERROR! Could not create RGBMatrix instance!!!!\n");
    exit(1);
  }

  printf("Size: %dx%d. Hardware gpio mapping: %s\n",
         matrix->width(), matrix->height(), matrix_options.hardware_mapping);

  matrixStrip = new MatrixSegment(matrix);
  matrixStrip->mTotalPixels = totalPixels;

  matrixStrip->Start();
}



/**** MAIN *****/
int main(int argc, char* argv[]) {
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

//  printf("Server starting & expecting (%lu bytes)...\n", readBufferSize); fflush(stdout);
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);
  std::cout << "\x1B[2J\x1B[H";
  printf("%s\n", hostname);
  fflush(stdout);


  start_matrix();

  NetworkServerConfig serverConfig;
  serverConfig.incomingPort = 9890;
  serverConfig.numPanelsWide = numMatricesWide;
  serverConfig.numPanelsTall = numMatricesTall;
  serverConfig.singlePanelWidth = singlePanelWidth;
  serverConfig.singlePanelHeight = singlePanelHeight;
  serverConfig.segmentId = 1;
  serverConfig.matrixStripInstance = matrixStrip;

//  usleep(1000);
  server = new  NetworkServer(serverConfig);
  server->StartThread();

//  usleep(10);
  std::thread(interrupterThread).detach();


  while (!interrupt_received) {
//    printf("Main sleeping\n");
    sleep(1); // Time doesn't really matter. The syscall will be interrupted.
  }

  return 0;
}