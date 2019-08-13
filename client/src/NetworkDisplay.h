#ifndef GENUS_NETWORKED_MATRIX_DISPLAY_MATRIXDISPLAY_H
#define GENUS_NETWORKED_MATRIX_DISPLAY_MATRIXDISPLAY_H
#include <stdint.h>
#include <vector>
#include <thread>
#include <time.h>
#include <cmath>
#include <unistd.h>

#include "SegmentClient.h"
#include "SDL2Display.h"

struct NetworkDisplayConfig {
  size_t   inputBufferSize;

  uint16_t inputScreenWidth;
  uint16_t inputScreenHeight;

  uint16_t outputScreenWidth;
  uint16_t outputScreenHeight;

  uint16_t singlePanelHeight;
  uint16_t singlePanelWidth;

  uint16_t totalPanelsWide;
  uint16_t totalPanelsTall;

  uint8_t totalSegments;
  int frameRate;

  uint8_t segmentPanelsTall;
  uint8_t segmentPanelsWide;

  char *destinationPort;
  char *destinationIP;
  uint8_t destinationIpStartDigit;
};

class NetworkDisplay {

public:

  explicit NetworkDisplay(NetworkDisplayConfig config);
  ~NetworkDisplay();

  void ThreadFunction(NetworkDisplay *remoteDisplay);

  void DescribeSegments();
  void Update();

  uint16_t *GetInputBuffer();

  void WritePixel(uint16_t index, uint16_t color);

  void SwapBuffers() {
    mCurrInBuffer = (mCurrInBuffer == mInputBuffer1) ? mInputBuffer2 : mInputBuffer1;
    mCurrOutBuffer = (mCurrOutBuffer == mOutputBuffer1) ? mOutputBuffer2 : mOutputBuffer1; // Goes to matrix
  }

  uint16_t GetFrameCount() {
    return mFrameCount;
  }

public:

  size_t GetTotalInputPixels() {
    return mInputBufferSize;
  }

  size_t GetTotalOutputPixels() {
    return mOutputBufferSize;
  }

  bool GetThreadRunnning() {
    return mThreadRunning;
  }

  void StartThread() {
    mThreadRunning = true;
    mThread = std::thread(&NetworkDisplay::ThreadFunction, this, this);
    mThread.detach();
  }

  void LockMutex() {
    pthread_mutex_lock(&mMutex);
  }

  void UnlockMutex() {
    pthread_mutex_unlock(&mMutex);
  }

  size_t GetInputBufferSize() {
    return mInputBufferSize;
  }

public:
  uint32_t mSNow;
  uint32_t mSNext;

  void NextFrameDelay() {
    if (mFrameRate < 0) {
      return;
    }

    if (mSNow < mSNext) {
      usleep((mSNext - mSNow) * 1000);
      mSNow = Milliseconds();
    }

    mSNext = (mSNext + 1000 / mFrameRate);
  }

  uint32_t Milliseconds() {
    uint32_t ms;
    time_t s;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = lround(spec.tv_nsec / 1.0e6);
    if (ms > 999) {
      s++;
      ms = 0;
    }
    ms += s * 1000;
    return ms;
  }


private:
  NetworkDisplayConfig mConfig;

  int mFrameRate;
  uint16_t mScreenWidth;
  uint16_t mScreenHeight;


  void InitNetworkSegments();

  std::thread mThread;

  size_t mInputBufferSize;
  uint16_t mTotalInputPixels;
  uint16_t *mCurrInBuffer;
  uint16_t *mInputBuffer1;
  uint16_t *mInputBuffer2;

  size_t mOutputBufferSize;
  uint32_t mTotalOutputPixels;
  uint16_t *mCurrOutBuffer;
  uint16_t *mOutputBuffer1;
  uint16_t *mOutputBuffer2;

  uint16_t mOutputScreenWidth;
  uint16_t mOutputScreenHeight;
  uint16_t mInputScreenWidth;
  uint16_t mInputScreenHeight;

  uint16_t mSinglePanelWidth;
  uint16_t mSinglePanelHeight;

  std::vector<SegmentClient *> mSegments;
  uint16_t mFrameCount;

  bool mThreadRunning;
  pthread_mutex_t mMutex;

#ifdef __USE_SDL2_VIDEO__
  SDL2Display *mSDL2Display;
#endif
};


#endif //GENUS_MATRIX_DISPLAY_MATRIXDISPLAY_H

