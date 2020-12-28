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


//  Describe();


#ifdef __MATRIX_SHOW_DEBUG_MESSAGES__
  Describe();
#endif
}
static uint16_t msColor = 0;

void MatrixSegment::Run() {

  volatile uint32_t currentFrameCount = 0;

  while (running() && mShouldRun) {
    if (mFrameCount != currentFrameCount) {

//      msColor++;
//      uint8_t r = (msColor & 0xF800) >> 8;       // rrrrr... ........ -> rrrrr000
//      uint8_t g = (msColor & 0x07E0) >> 3;       // .....ggg ggg..... -> gggggg00
//      uint8_t b = (msColor & 0x1F) << 3;         // ............bbbbb -> bbbbb000
//      mRenderCanvas->Fill(r,g,b);

#ifdef __MATRIX_SHOW_DEBUG_MESSAGES__
      printf("mDisplayCanvas height %i :: width %i\n", mRenderCanvas->height(), mRenderCanvas->width());

      if (mRenderCanvas == mCanvas1) {
        printf("mRenderCanvas = mCanvas 1 :: %p !\n", mCanvas1);fflush(stdout);
      }
      else {
        printf("mRenderCanvas = mCanvas 2 :: %p !\n", mCanvas2);fflush(stdout);
      }
#endif
      SwapBuffers();
      mDisplayCanvas = mMatrix->SwapOnVSync(mDisplayCanvas, 1);
      currentFrameCount = mFrameCount;
    }
//    else {
//      usleep(10);
//    }

  }

  printf("MatrixStrip::Run() ended!!\n");
}

// Used for debug purposes only.
void MatrixSegment::Describe() {
  printf("MatrixStrip %p\n", this);

//  printf("\tmTotalPixels = %lu\n", mTotalPixels);
  printf("\tmCanvasWidth = %i\n", mCanvasWidth);
  printf("\tmCanvasHeight = %i\n", mCanvasHeight);
  printf("\tmShouldClearBuffers = %i\n", mShouldClearBuffers);
  printf("\tmClearBuffersDelay = %i\n", mClearBuffersDelay);
}