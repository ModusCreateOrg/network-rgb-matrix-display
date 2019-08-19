/**
 *
 * This class is used to Paint the RGB Matrix and is meant to be as efficient as possible.
 * All color processing should be done via the NetworkServer class.
 */

#include "MatrixSegment.h"
#include "unistd.h"


MatrixSegment::MatrixSegment(RGBMatrix *m) : ThreadedCanvasManipulator(m), mMatrix(m)  {
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

void MatrixSegment::Run() {

  volatile uint32_t currentFrameCount = 0;

  while (running() && mShouldRun) {
    if(mFrameCount != currentFrameCount) {

//      msColor++;
//      uint8_t r = (msColor & 0xF800) >> 8;       // rrrrr... ........ -> rrrrr000
//      uint8_t g = (msColor & 0x07E0) >> 3;       // .....ggg ggg..... -> gggggg00
//      uint8_t b = (msColor & 0x1F) << 3;         // ............bbbbb -> bbbbb000
//      mDisplayCanvas->Fill(r,g,b);

      SwapBuffers();
      mDisplayCanvas = mMatrix->SwapOnVSync(mDisplayCanvas, 1);
      currentFrameCount = mFrameCount;
    }
    else {
      usleep(1000);
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
void MatrixSegment::Describe() {
  printf("MatrixStrip %p\n", this);

  printf("\tmTotalPixels = %lu\n", mTotalPixels);
  printf("\tmCanvasWidth = %i\n", mCanvasWidth);
  printf("\tmCanvasHeight = %i\n", mCanvasHeight);
}