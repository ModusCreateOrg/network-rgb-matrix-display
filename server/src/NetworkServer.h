#ifndef MATRIX_NETWORKSERVER_H
#define MATRIX_NETWORKSERVER_H

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include <cstring>
#include <pthread.h>
#include <time.h>

#include <thread>
using boost::asio::ip::tcp;


#include "MatrixSegment.h"
#include "NetworkServerConfig.h"
#include "ini.h"

class NetworkServer {

public:
  int  mSegmentId;
  int mSinglePanelWidth;
  int mSinglePanelHeight;
  int mPixelsPerPanel;

  int mPanelsWide;
  int mPanelsTall;

  int mSegmentWidth;
  int mSegmentHeight;

  int mTotalPixels;

  size_t mTotalBytes;

  unsigned short mIncomingPort;

  volatile uint32_t mNumberSamples;
  volatile double mTotalDelta;
  volatile double mAverage;
  volatile double mPriorAverage;

public:
  explicit NetworkServer(NetworkServerConfig *config);

  ~NetworkServer();

  void ReceiveDataThread(tcp::socket sock);
  void ServerStartingThread();

  void LockMutex();
  void UnlockMutex();

  void ClearBuffers() {
    printf("%s\n", __FUNCTION__);
    fflush(stdout);
    LockMutex();
    mMatrixStrip->mRenderCanvas->Fill(0, 0, 0);
    mMatrixStrip->mDisplayCanvas->Fill(0, 0, 0);
    mFrameCount++;
    UnlockMutex();
  }

  bool GetThreadRunning() {
    return mThreadRunning;
  }

  void StartThread();
  void StopThread();

  void Describe();

  uint16_t GetFrameCount() {
    return mFrameCount;
  }

  void IncrementFrameCount() {
    mFrameCount++;
  }
private:
  volatile uint16_t mFrameCount;
  volatile bool mThreadRunning;

  pthread_mutex_t mMutex;
  std::thread mThread;
  unsigned short mPort;
  const char *mIP;

  MatrixSegment *mMatrixStrip;
};




#endif //MATRIX_NETWORKSERVER_H
