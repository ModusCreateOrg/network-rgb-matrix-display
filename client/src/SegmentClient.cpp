
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <stdlib.h>

#include "SegmentClient.h"


using boost::asio::ip::tcp;


SegmentClient::SegmentClient(struct SegmentClientConfig config) {
#ifdef __linux__
  pthread_mutex_destroy(&mMutex);
  pthread_mutex_init(&mMutex, NULL);
#endif

  mFrameCount = 0;
  mSegmentId = config.segmentId;
  mSinglePanelWidth = config.singlePanelWidth;
  mSinglePanelHeight = config.singlePanelHeight;
  mPixelsPerPanel = mSinglePanelWidth * mSinglePanelWidth;

  mSegmentWidth = config.singlePanelWidth * config.numPanelsWide;
  mSegmentHeight = config.singlePanelHeight * config.numPanelsTall;

  mPanelsWide = config.numPanelsWide;
  mPanelsTall = config.numPanelsTall;

  mTotalPixels = mPixelsPerPanel * mPanelsWide * mPanelsTall;
  mTotalBytes = mTotalPixels * sizeof(uint16_t);


  mSegmentBuffer1 = (uint16_t *)malloc(mTotalBytes);
  mSegmentBuffer2 = (uint16_t *)malloc(mTotalBytes);

  mInputBuffer = mSegmentBuffer1;

  mDestinationIP = (char *)malloc(sizeof(config.destinationIP));
  strcpy(mDestinationIP, config.destinationIP);

  mDestinationPort = (char *)malloc(sizeof(config.destinationPort));
  strcpy(mDestinationPort, config.destinationPort);

  printf("SegmentClient %s (id = %i)\n", mDestinationIP, mSegmentId);
}

//uint16_t  color = 0;
//float color = 0;
void SegmentClient::SendDataThread(SegmentClient *mySegment) {
//  if (mSegmentId != 1) {
//    return;
//  }

  uint16_t currentFrame = 0;

  uint16_t *data = (uint16_t *)malloc(mySegment->mTotalBytes);



  while (mySegment->GetThreadRunning()) {
//    printf("segment %i :: SendData()\n", mSegmentId);
//    color += .001f;

    if (mySegment->GetFrameCount() == currentFrame) {
      usleep(50);
      continue;
    }

    try {
//      printf("SegmentClient::%s %i %s %s\n", __FUNCTION__, mSegmentId, mDestinationIP, mDestinationPort);
//      printf("mTotalBytes %lu\n", mySegment->mTotalBytes);
//      fflush(stdout);

      boost::asio::io_service io_service;

      tcp::socket s(io_service);
      tcp::resolver resolver(io_service);

      boost::asio::connect(s, resolver.resolve({mySegment->mDestinationIP, mySegment->mDestinationPort}));

      mySegment->LockMutex();
      uint16_t *sBuffPtr = mySegment->GetOutputBuffer();

//      memset(data, (uint16_t)color, mySegment->mTotalBytes);
      memcpy(data, sBuffPtr, mySegment->mTotalBytes);
      mySegment->UnlockMutex();

      size_t numBytesWritten = boost::asio::write(s, boost::asio::buffer(data, mySegment->mTotalBytes));
//      printf("numBytesWritten = %lu, color = %i\n", numBytesWritten, data[0]);fflush(stdout);

      char reply[10];
      size_t reply_length = boost::asio::read(s,boost::asio::buffer(reply, 1));

      if (reply_length != 1) {
        printf("* BAD *\n\n");
      }

      currentFrame = mySegment->GetFrameCount();

    }
    catch (std::exception& e) {
//      std::cerr << mySegment->mDestinationIP << " " <<  __FUNCTION__ << " Exception: " << e.what() << "\n";
    }
  }

  printf("SegmentClient::SendDataThread ended %i\n", mySegment->mSegmentId);
}



void SegmentClient::StartThread() {
//  if (mSegmentId != 0) {
//    return;
//  }
  mThreadRunning = true;
  mThread = std::thread(&SegmentClient::SendDataThread, this, this);
  mThread.detach();
}


void SegmentClient::StopThread() {
  mThreadRunning = false;
  if (mThread.joinable()) {
    mThread.join();
  }
  usleep(100);
}

void SegmentClient::LockMutex() {
//  printf("%i SegmentClient::%s %p\n", mSegmentId, __FUNCTION__, &mMutex);
  pthread_mutex_lock(&mMutex);
}

void SegmentClient::UnlockMutex() {
//  printf("SegmentClient::%s\n", __FUNCTION__);

  pthread_mutex_unlock(&mMutex);
}

void SegmentClient::Describe() {
  printf("SegmentClient %p\n", this);
  printf("\tmSegmentId = %i\n", mSegmentId);
  printf("\tmSinglePanelWidth = %i\n", mSinglePanelWidth);
  printf("\tmSinglePanelHeight = %i\n", mSinglePanelHeight);
  printf("\tmPixelsPerPanel = %i\n", mPixelsPerPanel);
  printf("\tmSegmentWidth = %i\n", mSegmentWidth);
  printf("\tmSegmentHeight = %i\n", mSegmentHeight);
  printf("\tmMatricesWide = %i\n", mPanelsWide);
  printf("\tmMatricesTall = %i\n", mPanelsTall);
  printf("\tmTotalPixels = %i\n", mTotalPixels);
  printf("\tmTotalBytes = %lu\n", mTotalBytes);
  printf("\tmDestinationIP = %s\n", mDestinationIP);
  printf("\tmDestinationPort = %s\n", mDestinationPort);
  fflush(stdout);
}

SegmentClient::~SegmentClient() {
  StopThread();


  delete mSegmentBuffer1;
  delete mSegmentBuffer2;
}

