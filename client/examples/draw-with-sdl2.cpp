#undef __USE_SDL2_VIDEO__


#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <thread>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_render.h>

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

//  for (int i = 0; i < argc; i++) {
//    printf("arg %i\t%s\n", i, argv[i]);
//  };
//
//  if (argc < 2) {
//    fprintf(stderr, "Fatal Error! Please specify INI file to open.\n\n");
//    exit(127);
//  }

  const char *file = "draw-with-sdl2.ini";

  NetworkDisplayConfig displayConfig = NetworkDisplay::GenerateConfig(file);

  displayConfig.Describe();

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);

  const uint16_t screenWidth = networkDisplay->GetOutputScreenWidth(),
                 screenHeight = networkDisplay->GetOutputScreenHeight();

  // Detach the interrupter thread (handles CTRL+C)
  std::thread(interrupterThread).detach();

  // Create the SDL2 RGB Surface with 16bit color depth
  SDL_Surface *surface = SDL_CreateRGBSurface(
    0,
    networkDisplay->GetOutputScreenWidth(),
    networkDisplay->GetInputScreenHeight(),
    16,   // Bit depth
    0,    // Auto Red Mask
    0,    // Auto Blue Mask
    0,    // Auto Green Mask
    0
  );


  // Exit the program if we can't create the SDL2 surface
  if(surface == NULL) {
    fprintf(stderr, "CreateRGBSurface failed: %s\n ", SDL_GetError());
    exit(1);
  }

  // Create SDL Software Renderer
  SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);

  int y = 0;
  uint16_t color = 0;
  int direction = 1;

  // Loop while we have not been interrupted.
  // This will draw a line for the entire width of the screen and bounce it down and up again.
  while (! interrupt_received) {
    color++;

    // Clear the screen (fill the buffer with black)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Draw random RGB values for
    SDL_SetRenderDrawColor(renderer, random() & 0xFF, random() & 0xFF, random() & 0xFF,0);

    SDL_RenderDrawLine(renderer, 0, y, screenWidth, y);

    y += direction;

    if (y > screenHeight) {
      direction = -1;
      y = screenHeight;
    }

    if (y < 0) {
      direction = 1;
      y = 0;
    }

    uint16_t *inputBuffer = networkDisplay->GetInputBuffer();
    memcpy(inputBuffer, surface->pixels, networkDisplay->GetInputBufferSize());


    networkDisplay->Update();
  }

  delete surface;
  delete networkDisplay;

  return 0;
}


