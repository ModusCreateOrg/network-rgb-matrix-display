#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

#include <stdlib.h>
#include <vector>

#include "NetworkDisplay.h"

NetworkDisplay::NetworkDisplay(NetworkDisplayConfig config) {
  mConfig = config;
  mTotalOutputPixels = 0;

#ifdef __linux__
  pthread_mutex_destroy(&mMutex);
  pthread_mutex_init(&mMutex, NULL);
#endif

  InitNetworkSegments();

  mFrameCount = 0;
  mFrameRate = config.frameRate;


  mInputScreenWidth = config.inputScreenWidth;
  mInputScreenHeight = config.inputScreenHeight;

  mTotalInputPixels = mInputScreenWidth * mInputScreenHeight;
  mInputBufferSize =  mTotalInputPixels * sizeof(uint16_t);

  mInputBuffer1 = (uint16_t *)malloc(mInputBufferSize);
  mInputBuffer2 = (uint16_t *)malloc(mInputBufferSize);
  mCurrInBuffer = mInputBuffer1;



  mOutputScreenWidth  = config.outputScreenWidth;
  mOutputScreenHeight = config.outputScreenHeight;

  mTotalOutputPixels = mOutputScreenWidth * mOutputScreenHeight;
  mOutputBufferSize = mTotalOutputPixels * sizeof(uint16_t);
  mOutputBuffer1 = (uint16_t *)malloc(mOutputBufferSize);
  mOutputBuffer2 = (uint16_t *)malloc(mOutputBufferSize);
  mCurrOutBuffer = mOutputBuffer1;

  mSinglePanelWidth = config.singlePanelWidth;
  mSinglePanelHeight = config.singlePanelHeight;



  mSNow  = Milliseconds();
  mSNext = mSNow + 1000 / config.frameRate;

  StartThread();

#ifdef __USE_SDL2_VIDEO__
  //  SDL2Display is buggy, so please do not depend on it just yet.
  mSDL2Display = new SDL2Display(mOutputScreenWidth, mOutputScreenHeight);
#endif


}


void NetworkDisplay::InitNetworkSegments() {

  for (uint8_t i = 0; i < mConfig.numberSegments ; i++) {
    SegmentClientConfig segmentConfig;

    segmentConfig.segmentId = i;
    segmentConfig.singlePanelHeight = mConfig.singlePanelHeight;
    segmentConfig.singlePanelWidth = mConfig.singlePanelWidth;

    segmentConfig.segmentWidth =  mConfig.segmentWidth;
    segmentConfig.segmentHeight = mConfig.segmentHeight;


    segmentConfig.destinationPort = strdup(mConfig.segments[i].destinationPort);

    segmentConfig.destinationIP = strdup(mConfig.segments[i].destinationIp);

    mTotalOutputPixels += segmentConfig.segmentWidth * segmentConfig.segmentWidth;

    auto *segment = new SegmentClient(segmentConfig);
    mSegments.push_back(segment);

    segment->StartThread();
  }

  DescribeSegments();
}



void NetworkDisplay::ThreadFunction(NetworkDisplay *remoteDisplay) {
  uint16_t currentFrame = 0;
  uint16_t smallerSceen = mInputScreenWidth < mOutputScreenWidth ? mInputScreenWidth : mOutputScreenWidth;
  printf("Smaller screen is %s\n", (mInputScreenWidth < mOutputScreenWidth) ? "INPUT" : "OUTPUT");

  while (remoteDisplay->GetThreadRunnning()) {

    if (remoteDisplay->GetFrameCount() == currentFrame) {
      usleep(50);
      continue;
    }


    // Chunk up the entire screen buffer into separate display segments.
    for (int segmentIdx = 0; segmentIdx < remoteDisplay->mSegments.size(); segmentIdx++) {
      SegmentClient *segment = remoteDisplay->mSegments[segmentIdx];

      segment->LockMutex();
        uint16_t startX = segmentIdx * segment->mSegmentWidth;

        const size_t numBytes = segment->mSegmentWidth * sizeof(uint16_t);

        for (uint16_t y = 0; y < segment->mSegmentHeight; y++) {
          uint16_t *screenBuffer = &mCurrOutBuffer[(y * smallerSceen) + (startX)];
          uint16_t *segmentBuffer = &segment->GetInputBuffer()[y * segment->mSegmentWidth];

          memcpy(segmentBuffer, screenBuffer, numBytes);
        }

      segment->UnlockMutex();
      segment->SwapBuffers();
      segment->IncrementFrameCount();
    }

    currentFrame = remoteDisplay->GetFrameCount();

  }

  printf("NetworkDisplay::ThreadFunction ended\n");
}

void NetworkDisplay::Update() {
  LockMutex();

  bzero(mCurrOutBuffer, mOutputBufferSize);
  size_t smallerBuffer = (mInputBufferSize < mOutputBufferSize) ? mInputBufferSize : mOutputBufferSize;
  memcpy(mCurrOutBuffer, mCurrInBuffer, smallerBuffer);

#ifdef __USE_SDL2_VIDEO__
  mSDL2Display->Update(mCurrInBuffer, mTotalInputPixels);
#endif

  UnlockMutex();

  mFrameCount++;
//  SwapBuffers();
  NextFrameDelay();
}




void NetworkDisplay::DescribeSegments() {
  printf("I have %lu segments!\n", mSegments.size());
  for (int i = 0; i < mSegments.size(); i++) {
    mSegments[i]->Describe();
  }
}

uint16_t *NetworkDisplay::GetInputBuffer() {
  return mCurrInBuffer;
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