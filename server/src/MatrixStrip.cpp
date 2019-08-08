/**
 *
 * This class is used to Paint the RGB Matrix and is meant to be as efficient as possible.
 * All color processing should be done via the NetworkServer class.
 */

#include "MatrixStrip.h"
#include "unistd.h"


MatrixStrip::MatrixStrip(RGBMatrix *m) : ThreadedCanvasManipulator(m), mMatrix(m)  {
#ifdef __linux__
  pthread_mutex_destroy(&mMutex);
  pthread_mutex_init(&mMutex, nullptr);
#endif


  mShouldRun = true;

  mCanvas1 = m->CreateFrameCanvas();
  mRenderCanvas = mCanvas1;

  mCanvas2 = m->CreateFrameCanvas();
  mDisplayCanvas = mCanvas2;

  mCanvasWidth = mRenderCanvas->width();
  mCanvasHeight = mRenderCanvas->height();
  mFrameCount = 0;

  Describe();
  fflush(stdout);
}
static uint16_t msColor = 0;

void MatrixStrip::Run() {

  volatile uint32_t currentFrameCount = 0;

  while (running() && mShouldRun) {
//    msColor++;
    if(mFrameCount != currentFrameCount) {
      SwapBuffers();
      mDisplayCanvas = mMatrix->SwapOnVSync(mDisplayCanvas, 1);
      currentFrameCount = mFrameCount;
    }
    else {
      usleep(100);
    }

//    uint8_t r = (msColor & 0xF800) >> 8;       // rrrrr... ........ -> rrrrr000
//    uint8_t g = (msColor & 0x07E0) >> 3;       // .....ggg ggg..... -> gggggg00
//    uint8_t b = (msColor & 0x1F) << 3;         // ............bbbbb -> bbbbb000
//    mDisplayCanvas->Fill(r,g,b);
//    LockMutex();
//    mDisplayCanvas = mMatrix->SwapOnVSync(mDisplayCanvas);
//    UnlockMutex();
  }

  printf("MatrixStrip::Run() ended!!\n");
}

// Used for debug purposes only.
void MatrixStrip::Describe() {
  printf("MatrixStrip %p\n", this);

  printf("\tmTotalPixels = %lu\n", mTotalPixels);
  printf("\tmCanvasWidth = %i\n", mCanvasWidth);
  printf("\tmCanvasHeight = %i\n", mCanvasHeight);
}