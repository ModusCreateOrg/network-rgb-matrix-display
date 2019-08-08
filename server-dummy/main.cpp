//
// blocking_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include <cstring>
#include <time.h>

#include <pthread.h>

using boost::asio::ip::tcp;


#include "SDL2/SDL.h"

pthread_mutex_t bufferMutex;


static const int32_t FRAMERATE = 30;
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 256

static int32_t sNow, sNext;

SDL_Window   *window   = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture  *texture  = nullptr;


const uint16_t matrixHeight = 64;
const uint16_t matrixWidth = 64;
const uint16_t matrixSize = (matrixHeight * matrixWidth);
const uint16_t numMatricesWide = 1;
const uint16_t numMatricesTall = 3;
const uint16_t totalPixels = matrixSize * numMatricesWide * numMatricesTall;
const size_t readBufferSize =  totalPixels * sizeof(uint16_t);
const size_t totalBytes = totalPixels * sizeof(uint16_t);

uint16_t *screenBuffer;
volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

void setupSDL() {

  printf("setupSDL\n");
  // initialize any hardware
  SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

  uint32_t flags =  SDL_WINDOW_OPENGL |  SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE| SDL_WINDOW_SHOWN;

  // Create an application window with the following settings:
  window = SDL_CreateWindow(
    "genus",                  // window title
    SDL_WINDOWPOS_UNDEFINED,           // initial resources position
    SDL_WINDOWPOS_UNDEFINED,           // initial y position
    SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2,   // width, in pixels
    flags                        // flags - see below
  );

  SDL_SetWindowMinimumSize(window, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);

  if (window == nullptr) {
    printf("Could not create window: %s\n", SDL_GetError());
    exit(1);
  }

  int w, h;
  SDL_GL_GetDrawableSize(window, &w, &h);
  printf("SDL window size %i x %i\n", w,h);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                              SCREEN_HEIGHT);

  if (!texture) {
    printf("Cannot create texture %s\n", SDL_GetError());
  }


  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_GL_SetSwapInterval(1);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  {
    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    SDL_SetWindowPosition(window, x+1, y+1);
  }


}

bool shouldQuit = false;

uint32_t toRGB888(uint16_t color) {

  uint8_t r = ((color >> 11) & 0x1F);
  uint8_t g = ((color >> 5) & 0x3F);
  uint8_t b = (color & 0x1F);

  r = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;
  g = ((((color >> 5) & 0x3F) * 259) + 33) >> 6;
  b = (((color & 0x1F) * 527) + 23) >> 6;

  return r << 16 | g << 8 | b;

}

void renderSDLWindow(void) {

  printf("renderWindow thread...");

  uint32_t color = 0;
  while(! shouldQuit) {
    color++;
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        shouldQuit = true;
        break;
      }
    }

    int screenX, screenY;
    SDL_GetWindowPosition(window, &screenX, &screenY);

    void *screenBuf;
    int pitch;

    if (0 == SDL_LockTexture(texture, nullptr, &screenBuf, &pitch)) {
      auto *screenBits = (int32_t *) screenBuf;

//      std::memset(screenBuf, color, (SCREEN_WIDTH * SCREEN_HEIGHT));

      pthread_mutex_lock(&bufferMutex);

      // Copy data to screenBuff

      int sbIndex = 0;
      for (int row = 0; row < matrixHeight * numMatricesTall; row++) {
        for (int col = 0; col < matrixWidth * numMatricesWide; col++) {
          *screenBits++ = toRGB888(screenBuffer[sbIndex]);
          sbIndex++;
        }

        screenBits += (SCREEN_WIDTH - (matrixWidth * numMatricesWide));
      }

      pthread_mutex_unlock(&bufferMutex);


      SDL_UnlockTexture(texture);
    }
    else {
      printf("Cannot lock texture!\n");
    }


    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr); // Render texture to entire window
    SDL_RenderPresent(renderer);              // Do update

    SDL_Delay(16);
  }
}


volatile uint32_t number_samples = 0;
volatile double total_delta = 0;
volatile double average;


void receivePayload(tcp::socket sock) {

//  printf("receivePayload thread...\n");

  int totalBytes = 0;
  int loopNum = 0;
  uint16_t sbIndex = 0;

  volatile clock_t start = 0;
  volatile clock_t end = 0;
  uint16_t data[readBufferSize];

  try   {
    while (! interrupt_received) {


      start = clock();
      boost::system::error_code error;
      size_t length = boost::asio::read(sock, boost::asio::buffer(data), boost::asio::transfer_exactly(totalPixels), error);

//      printf("loopNum = %i, length = %lu\n", loopNum++, length);

      if (error == boost::asio::error::eof) {
//        printf("screenBuffer[0] == %i\n", screenBuffer[0]);
//        printf("Finished %i bytes\n---------------\n\n", totalBytes);
        break; // Connection closed cleanly by peer.
      }
      else if (error) {
        throw boost::system::system_error(error); // Some other error.
      }


      pthread_mutex_lock(&bufferMutex);
      totalBytes += length;

      for (int i = 0; i < totalPixels; i++) {
        screenBuffer[sbIndex++] = data[i];
      }


      pthread_mutex_unlock(&bufferMutex);

      char *returnData = "K";
      boost::asio::write(sock, boost::asio::buffer(returnData, 1));

      if (sbIndex == totalPixels) {
        end = clock();

        number_samples++;
        double delta = (end - start);
//        printf("start: %lu, end: %lu, delta = %2f\r", start, end, delta);
        total_delta += delta;

        average = total_delta / number_samples;

//
//        printf("screenBuffer[0] == %i\n", screenBuffer[0]);
//        for (int i = 0; i < 3; i++) {
//          printf("screenBuffer[%i] == %i\n", i, screenBuffer[i]); fflush(stdout);
//        }
//        printf("Finished %lu bytes (BREAK!)\n---------------\n\n", totalPixels * sizeof(uint16_t));
        // free(data);
        break;
      }

    }
  }
  catch (std::exception& e)   {

    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void server(boost::asio::io_context& io_context, unsigned short port) {
  tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));

  while (! interrupt_received) {
    std::thread(receivePayload, a.accept()).detach();
  }
}

void serverStarterThread(unsigned short port) {
  boost::asio::io_context io_context;

//  printf("serverThread %i\n", port);
  server(io_context, port);
}

volatile double priorAverage = 0;
int retries = 0;
void interrupterThread() {
  while(1) {
    if (interrupt_received) {
      exit(1);
    }
    printf("avg %f          \r", average);

    if (average == priorAverage) {
      retries ++;
      if (retries > 5) {
        average = 0;
        retries = 0;
      }
    }
    priorAverage = average;


    usleep(10000);

  }


}

int main(int argc, char* argv[]) {


  std::cout << "\x1B[2J\x1B[H";
  fflush(stdout);
  
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);

  printf("Server starting & expecting (%lu bytes)...\n", readBufferSize); fflush(stdout);
  printf("%s\n", hostname);
  fflush(stdout);

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  std::thread(interrupterThread).detach();



  screenBuffer = (uint16_t *)std::malloc(sizeof(uint16_t) * totalPixels);

  try {
    unsigned short port = 9890;
//    for (int i = 0; i < 5; i++) {
//      printf("Starting thread %i:\n", i);
//      printf("  Listening on port %i\n", port);
//      printf("  Attempting to receive %lu bytes\n", sizeof(uint16_t) * totalPixels);
//
//      std::thread(serverStarterThread, port++).detach();
////      break;
//    }
    std::thread(serverStarterThread, port).detach();

//    renderSDLWindow();

  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }



  int n = 0;
//  while(n < 5000) {
//    n++;
//
//    SDL_Event e;
//    if (SDL_PollEvent(&e)) {
//      if (e.type == SDL_QUIT) {
//        break;
//      }
//    }
//
//    int screenX, screenY;
//    SDL_GetWindowPosition(screen, &screenX, &screenY);
//
//    void *screenBuf;
//    int pitch;
//
//    if (0 == SDL_LockTexture(texture, nullptr, &screenBuf, &pitch)) {
//      auto *screenBits = (int32_t *) screenBuf;
//
//      for (int y = 0; y < SCREEN_HEIGHT; y++) {
//        for (int x = 0; x < SCREEN_WIDTH; x++) {
//          *screenBits++ = (int32_t) (random() % INT32_MAX);
//        }
//      }
//
//
//      SDL_UnlockTexture(texture);
//    }
//    else {
//      printf("Cannot lock texture!\n");
//    }
//
//
//    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
//    SDL_RenderClear(renderer);
//    SDL_RenderCopy(renderer, texture, nullptr, nullptr); // Render texture to entire window
//    SDL_RenderPresent(renderer);              // Do update


//    SDL_Delay(10);
//  }

  while(! shouldQuit) {

    SDL_Delay(100);

  }



  printf("destroy SDL\n");
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  // Clean up
  SDL_Quit();
  return 0;
}
