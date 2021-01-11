#undef __USE_SDL2_VIDEO__

// This example draws a bitmap

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <thread>
#include <unistd.h>

#include "NetworkDisplay.h"
#include "NetworkDisplayConfig.h"

#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_render.h>


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


int lineY = 0;
int lineX = 0;
int directionY = 1;
int directionX = 1;

SDL_Rect imgRect;

// This function simply draws two lines using random colors that bounce off the X & Y boundaries.
void drawLines(SDL_Renderer *renderer, uint16_t screenWidth, uint16_t screenHeight) {

  // Clear the screen (fill the buffer with black)
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  // Draw random RGB values for
  SDL_SetRenderDrawColor(renderer, random() & 0xFF, random() & 0xFF, random() & 0xFF,0);
  SDL_RenderDrawLine(renderer, 0, lineY, screenWidth, lineY);

  SDL_SetRenderDrawColor(renderer, random() & 0xFF, random() & 0xFF, random() & 0xFF,0);
  SDL_RenderDrawLine(renderer, lineX, 0, lineX, screenHeight);

  lineY += directionY;

  if (lineY > screenHeight) {
    directionY = -1;
    lineY = screenHeight;
  }

  if (lineY < 0) {
    directionY = 1;
    lineY = 0;
  }

  lineX += directionX;
  if (lineX > screenWidth) {
    directionX = -1;
    lineX = screenWidth;
  }

  if (lineX < 0) {
    directionX = 1;
    lineX = 0;
  }
}

float randomFloat(float minimum, float maximum ) {
  float floatScale = (float)rand() / (float)RAND_MAX;
  return minimum + floatScale * (maximum - minimum);
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

  const char *file = "draw-bitmap.ini";

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
    screenWidth,
    screenHeight,
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

  // Copied from https://stackoverflow.com/a/9296467
  FILE* imgFile = fopen("image.bmp", "rb");

  if (! imgFile) {
    fprintf(stderr, "Could not open bitmap image!");
  }
  unsigned char imageInfo[54];

  // read the 54-byte header
  fread(imageInfo, sizeof(unsigned char), 54, imgFile);

  // extract image imgHeight and imgWidth from header
  int imgWidth = *(int*)&imageInfo[18];
  int imgHeight = *(int*)&imageInfo[22];

  // allocate 3 bytes per pixel
  int numBytes = 3 * imgWidth * imgHeight;
  auto* imageData = new unsigned char[numBytes];

  // read the rest of the imageData at once
  fread(imageData, sizeof(unsigned char), numBytes, imgFile);
  fclose(imgFile);

  for(int i = 0; i < numBytes; i += 3) {
    // flip the order of every 3 bytes
    unsigned char tmp = imageData[i];
    imageData[i] = imageData[i + 2];
    imageData[i + 2] = tmp;
  }

  float imgY = 0,
        imgX = 0;

  float imgDirectionX = 1.25f,
        imgDirectionY = 1.15f;

  // Loop while we have not been interrupted.
  // This will draw a line for the entire imgWidth of the screen and bounce it down and up again.
  while (! interrupt_received) {

    drawLines(renderer, screenWidth, screenHeight);

    uint16_t *inputBuffer = networkDisplay->GetInputBuffer();
    memcpy(inputBuffer, surface->pixels, networkDisplay->GetInputBufferSize());

    imgX += imgDirectionX;

    //  Reverse the image X direction if we hit the right-most boundary
    if (imgX > float(screenWidth - imgWidth)) {
      imgDirectionX *= -1;
      imgX = (float)screenWidth - (float)imgWidth;
    }

    // Reverse the image X direction if we hit the left-most boundary
    if (imgX < 0) {
      imgDirectionX *= -1;
      imgX = 0;
    }

    imgY += imgDirectionY;
    //  Reverse the image Y direction if we hit the right-most boundary
    if (imgY > float(screenHeight - imgHeight)) {
      imgDirectionY *= -1;
      imgY = (float)screenHeight - (float)imgHeight;
    }

    // Reverse the image Y direction if we hit the left-most boundary
    if (imgY < 0) {
      imgDirectionY *= -1;
      imgY = 0;
    }

    for (int row = 0; row < imgHeight; row++) {
      for (int col = 0; col < imgWidth; col++) {
        uint16_t r = imageData[3 * (col * imgWidth + row)],
                 g = imageData[3 * (col * imgWidth + row) + 1],
                 b = imageData[3 * (col * imgWidth + row) + 2];

        uint16_t pixel = ((uint16_t(r & 0b11111000) << 8)) | ((uint16_t(g & 0b11111100) << 3)) | (uint16_t(b) >> 3);

        uint16_t pixelIndex = (int)(col + (int)imgY) * screenWidth + (int)(row + (int)imgX);
        inputBuffer[pixelIndex] = pixel;
      }
    }

    // Send out the data buffer to paint on the server Pi (the ones connected to the display)
    networkDisplay->Update();
  }

  delete[] imageData;
  delete surface;
  delete networkDisplay;

  return 0;
}


