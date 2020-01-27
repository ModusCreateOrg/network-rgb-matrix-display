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


  mInputScreenWidth = config.inputStreamWidth;
  mInputScreenHeight = config.inputStreamHeight;
  mTotalInputPixels = mInputScreenWidth * mInputScreenHeight;

  mInputBufferSize =  mTotalInputPixels * sizeof(uint16_t);

  mInputBuffer1 = (uint16_t *)malloc(mInputBufferSize);
  mInputBuffer2 = (uint16_t *)malloc(mInputBufferSize);
  mCurrInBuffer = mInputBuffer1;


  mOutputBufferSize = mTotalOutputPixels * sizeof(uint16_t);
  mOutputBuffer1 = (uint16_t *)malloc(mOutputBufferSize);
  mOutputBuffer2 = (uint16_t *)malloc(mOutputBufferSize);
  mCurrOutBuffer = mOutputBuffer1;

  mSinglePanelWidth = config.singlePanelWidth;
  mSinglePanelHeight = config.singlePanelHeight;



  mOutputScreenWidth  = config.totalPanelsWide * config.singlePanelWidth;
  mOutputScreenHeight = config.totalPanelsTall * config.singlePanelHeight;

  mSNow  = Milliseconds();
  mSNext = mSNow + 1000 / config.frameRate;

  StartThread();

#ifdef __USE_SDL2_VIDEO__
//  mSDL2Display = new SDL2Display(mInputScreenWidth, mInputScreenHeight);
  mSDL2Display = new SDL2Display(mOutputScreenWidth, mOutputScreenHeight);
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


    segmentConfig.destinationPort = mConfig.segments[i].destinationPort;

    segmentConfig.destinationIP = mConfig.segments[i].destinationIp;

    mTotalOutputPixels += (segmentConfig.singlePanelWidth * segmentConfig.singlePanelHeight) * segmentConfig.numPanelsWide * segmentConfig.numPanelsTall;

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
      usleep(100);
      continue;
    }


    for (int segmentIdx = 0; segmentIdx < remoteDisplay->mSegments.size() - 1; segmentIdx++) {
      SegmentClient *segment = remoteDisplay->mSegments[segmentIdx];

      segment->LockMutex();
//      bzero(segment->GetInputBuffer(), segment->mTotalBytes);
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

    currentFrame = remoteDisplay->GetFrameCount();

  }

  printf("NetworkDisplay::ThreadFunction ended\n");
}
//uint3232_t  color = 0;
void NetworkDisplay::Update() {
//  SwapBuffers();
//  printf("frame  %i\n", mFrameCount);
  LockMutex();

  bzero(mCurrOutBuffer, mOutputBufferSize);
  size_t smallerBuffer = (mInputBufferSize < mOutputBufferSize) ? mInputBufferSize : mOutputBufferSize;
  memcpy(mCurrOutBuffer, mCurrInBuffer, smallerBuffer);

#ifdef __USE_SDL2_VIDEO__
  mSDL2Display->Update(mCurrInBuffer, mTotalInputPixels);
#endif

  UnlockMutex();

  mFrameCount++;
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