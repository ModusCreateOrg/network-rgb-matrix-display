#ifndef MATRIX_MATRIXSTRIP_H
#define MATRIX_MATRIXSTRIP_H

#include <thread>
#include <pthread.h>
#include <string.h>

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "pixel-mapper.h"
#include "graphics.h"

using namespace rgb_matrix;


class MatrixSegment : public ThreadedCanvasManipulator  {

public:
  explicit MatrixSegment(RGBMatrix *m);

  void Run() override;

  size_t mTotalPixels;
  uint16_t mCanvasWidth;
  uint16_t mCanvasHeight;
  volatile uint16_t mFrameCount;

  FrameCanvas *mRenderCanvas;
  FrameCanvas *mDisplayCanvas;
  volatile bool mShouldRun;

  bool mShouldClearBuffers;
  unsigned long mClearBuffersDelay;


public:
  void SwapBuffers() {
    LockMutex();

    if (mRenderCanvas == mCanvas1) {
      mRenderCanvas = mCanvas2;
      mDisplayCanvas = mCanvas1;
    }
    else {
      mRenderCanvas = mCanvas1;
      mDisplayCanvas = mCanvas2;
    }

    UnlockMutex();
  }

  void LockMutex() {
//    printf("MatrixStrip::%s\n", __FUNCTION__); fflush(stdout);
    pthread_mutex_lock(&mMutex);
  }

  void UnlockMutex() {
//    printf("MatrixStrip::%s\n", __FUNCTION__); fflush(stdout);
    pthread_mutex_unlock(&mMutex);
  }


  void ClearBuffers() {
//    printf("\n%s %i\n", __FUNCTION__, mFrameCount); fflush(stdout);

    LockMutex();
    mCanvas1->Fill(0,0,0);
    mCanvas2->Fill(0,0,0);
    UnlockMutex();
    mFrameCount++;
  }

  FrameCanvas *GetRenderCanvas() {
    return mRenderCanvas;
  }

  void Describe();

private:
  FrameCanvas *mCanvas1;
  FrameCanvas *mCanvas2;


  RGBMatrix *const mMatrix;
  pthread_mutex_t mMutex;
};





#endif //MATRIX_MATRIXSTRIP_H
