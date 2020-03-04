#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <pthread.h>

#include "led-matrix.h"
#include "graphics.h"

#include <getopt.h>
#include <signal.h>

#include "NetworkServer.h"
#include "NetworkServerConfig.h"
#include "MatrixSegment.h"
#include "ini.h"

using std::min;
using std::max;
using boost::asio::ip::tcp;
using namespace rgb_matrix;


pthread_mutex_t bufferMutex;


/***********************************************************************/

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

NetworkServer *server;
double priorAverage = 0;

int retries = 0;

bool shouldQuit = false;
MatrixSegment *matrixStrip = nullptr;

// This function will loop and help exit gracefully if an interrupt is received.
void interrupterThread() {

  while(1) {
    if (interrupt_received) {
      server->StopThread();
      exit(1);
    }


    if (server->mAverage == priorAverage) {
      retries++;
      if (retries > 4) {
//        printf("\n**CLEAR BUFFERS!! retries=%i\n",retries);

        server->mAverage = 0;
        retries = 0;
        if (matrixStrip) {
          matrixStrip->ClearBuffers();
        }
      }

    }

    priorAverage = server->mAverage;
    usleep(500000);
  }
}

/***********************************************************************/


void start_matrix(NetworkServerConfig *aServerConfig) {

  RGBMatrix::Options matrix_options;


  matrix_options.chain_length = aServerConfig->numPanelsWide;
  matrix_options.cols = aServerConfig->singlePanelWidth;
  matrix_options.rows = aServerConfig->singlePanelHeight;

  matrix_options.hardware_mapping = strdup(aServerConfig->matrix_options.hardware_mapping);
  matrix_options.chain_length = aServerConfig->matrix_options.chain_length;
  matrix_options.parallel = aServerConfig->matrix_options.parallel;
  matrix_options.pwm_bits = aServerConfig->matrix_options.pwm_bits;
  matrix_options.pwm_lsb_nanoseconds = aServerConfig->matrix_options.pwm_lsb_nanoseconds;
  matrix_options.pwm_dither_bits = aServerConfig->matrix_options.pwm_dither_bits;
  matrix_options.brightness = aServerConfig->matrix_options.brightness;
  matrix_options.scan_mode = aServerConfig->matrix_options.scan_mode;
  matrix_options.multiplexing = aServerConfig->matrix_options.multiplexing;
  matrix_options.disable_hardware_pulsing = aServerConfig->matrix_options.disable_hardware_pulsing;
  matrix_options.row_address_type = aServerConfig->matrix_options.row_address_type;

  matrix_options.show_refresh_rate = aServerConfig->matrix_options.show_refresh_rate;
  matrix_options.inverse_colors = aServerConfig->matrix_options.inverse_colors;
  matrix_options.led_rgb_sequence = strdup(aServerConfig->matrix_options.led_rgb_sequence);

  matrix_options.pixel_mapper_config = strdup(aServerConfig->matrix_options.pixel_mapper_config);


  rgb_matrix::RuntimeOptions runtime_opt;
  runtime_opt.gpio_slowdown = aServerConfig->runtime_options.gpio_slowdown;
//  runtime_opt.daemon = aServerConfig->runtime_options.daemon;
//  runtime_opt.drop_privileges = aServerConfig->runtime_options.drop_privileges;
//  runtime_opt.do_gpio_init = aServerConfig->runtime_options.do_gpio_init;

  RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);

  if (matrix == nullptr) {
    printf("ERROR! Could not create RGBMatrix instance!!!!\n");
    exit(1);
  }

  printf("Size: %dx%d. Hardware gpio mapping: %s\n",
         matrix->width(), matrix->height(), matrix_options.hardware_mapping);

  matrixStrip = new MatrixSegment(matrix);
  matrixStrip->mTotalPixels = aServerConfig->totalPixels;

  printf("matrixStrip->Start()\n"); fflush(stdout);
  matrixStrip->Start();
}


int main(int argc, char* argv[]) {
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);
  std::cout << "\x1B[2J\x1B[H";
  printf("%s\n", hostname);
  fflush(stdout);

//  if (argc < 2) {
//    fprintf(stderr, "Fatal Error! Please specify INI file to open.\n\n");
//    exit(127);
//  }

  const char *file = "rgb-server.ini";
  NetworkServerConfig *serverConfig = NetworkServerConfig::FromIniFile(file);
  serverConfig->Describe();
  printf("serverConfig %p\n", serverConfig); fflush(stdout);
  start_matrix(serverConfig);
  serverConfig->matrixStripInstance = matrixStrip;

  server = new NetworkServer(serverConfig);
  server->Describe();
  server->StartThread();

  std::thread(interrupterThread).detach();


  while (!interrupt_received) {
//    printf("Main sleeping\n");fflush(stdout);
    sleep(1); // Time doesn't really matter. The syscall will be interrupted.
  }
  printf("\n");fflush(stdout);

  return 0;
}