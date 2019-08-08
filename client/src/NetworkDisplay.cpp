#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

#include <stdlib.h>
#include <vector>

#include "NetworkDisplay.h"




NetworkDisplay::NetworkDisplay(NetworkDisplayConfig aConfig) {
  mConfig = aConfig;


#ifdef __linux__
  pthread_mutex_destroy(&mMutex);
  pthread_mutex_init(&mMutex, NULL);
#endif

  InitNetworkSegments();

  mFrameCount = 0;
  mFrameRate = aConfig.frameRate;

  mInputBufferSize =  aConfig.inputScreenWidth * aConfig.inputScreenHeight * sizeof(uint16_t);
  mInputBuffer1 = (uint16_t *)malloc(mInputBufferSize);
  mInputBuffer2 = (uint16_t *)malloc(mInputBufferSize);
  mCurrInBuffer = mInputBuffer1;

  mOutputBufferSize = mTotalOutputPixels * sizeof(uint16_t);
  mOutputBuffer1 = (uint16_t *)malloc(mOutputBufferSize);
  mOutputBuffer2 = (uint16_t *)malloc(mOutputBufferSize);
  mCurrOutBuffer = mOutputBuffer1;

  mSinglePanelWidth = aConfig.singlePanelWidth;
  mSinglePanelHeight = aConfig.singlePanelHeight;

  mInputScreenWidth = aConfig.inputScreenWidth;
  mInputScreenHeight = aConfig.inputScreenHeight;
  mTotalInputPixels = mInputScreenWidth * mInputScreenHeight;

  mOutputScreenWidth  = aConfig.totalPanelsWide * aConfig.singlePanelWidth;
  mOutputScreenHeight = aConfig.totalPanelsTall * aConfig.singlePanelHeight;

  mSNow  = Milliseconds();
  mSNext = mSNow + 1000 / aConfig.frameRate;

  StartThread();

#ifdef __USE_SDL2_VIDEO__
  mSDL2Display = new SDL2Display(mInputScreenWidth, mInputScreenHeight);
#endif


}


void NetworkDisplay::InitNetworkSegments() {

  int ipFinalDigit = mConfig.destinationIpStartDigit;

  for (uint8_t i = 0; i < mConfig.totalSegments ; i++) {
    SegmentClientConfig segmentConfig;

    segmentConfig.segmentId = i;
    segmentConfig.singlePanelHeight = mConfig.singlePanelHeight;
    segmentConfig.singlePanelWidth = mConfig.singlePanelWidth;

    segmentConfig.numPanelsWide =  mConfig.segmentPanelsWide;
    segmentConfig.numPanelsTall = mConfig.segmentPanelsTall;

    segmentConfig.destinationPort = mConfig.destinationPort;

    char *destinationIp = (char *)malloc(strlen(mConfig.destinationIP));
    sprintf(destinationIp, mConfig.destinationIP, ipFinalDigit++);
    segmentConfig.destinationIP = destinationIp;

    mTotalOutputPixels += (segmentConfig.singlePanelWidth * segmentConfig.singlePanelHeight)
            * segmentConfig.numPanelsWide * segmentConfig.numPanelsTall;

    auto *segment = new SegmentClient(segmentConfig);
    mSegments.push_back(segment);

    segment->StartThread();
  }
}



void NetworkDisplay::SegmentBufferRunLoop(NetworkDisplay *aRemoteDisplay) {
  uint16_t currentFrame = 0;
  uint16_t smallerSceen = mInputScreenWidth < mOutputScreenWidth ? mInputScreenWidth : mOutputScreenWidth;

  while (aRemoteDisplay->GetThreadRunnning()) {

    // Nothing has changed. Sleep a little.
    if (aRemoteDisplay->GetFrameCount() == currentFrame) {
      usleep(100);
      continue;
    }


    for (int segmentIdx = 0; segmentIdx < aRemoteDisplay->mSegments.size(); segmentIdx++) {
      SegmentClient *segment = aRemoteDisplay->mSegments[segmentIdx];

      segment->LockMutex();

      uint16_t startX = segmentIdx * segment->mSegmentWidth;

      for (uint16_t y = 0; y < segment->mSegmentHeight; y++) {
        uint16_t *screenBuffer = &mCurrOutBuffer[(y * smallerSceen) + (startX)];
        uint16_t *segmentBuffer = &segment->GetInputBuffer()[y * segment->mSegmentWidth];

        memcpy(segmentBuffer, screenBuffer, segment->mSegmentWidth * sizeof(uint16_t));
      }

      segment->UnlockMutex();
      segment->SwapBuffers();
      segment->IncrementFrameCount();
    }

  }

  printf("NetworkDisplay::SegmentBufferRunLoop ended\n");
}


void NetworkDisplay::Update() {
  LockMutex();

  size_t smallerBuffer = (mInputBufferSize < mOutputBufferSize) ? mInputBufferSize : mOutputBufferSize;
  memcpy(mCurrOutBuffer, mCurrInBuffer, smallerBuffer);

#ifdef __USE_SDL2_VIDEO__
  mSDL2Display->Update(mCurrInBuffer, mTotalInputPixels);
#endif

  UnlockMutex();

  mFrameCount++;
  NextFrameDelay();
}






NetworkDisplay::~NetworkDisplay() {
  mThreadRunning = false;
  usleep(100);

  if (mThread.joinable()) {
    mThread.join();
  }

  for (int segmentIdx = 0; segmentIdx < mSegments.size(); segmentIdx++) {
    mSegments[segmentIdx]->StopThread();
  }

  delete mInputBuffer1;
  delete mInputBuffer2;
  delete mOutputBuffer1;
  delete mOutputBuffer2;
}