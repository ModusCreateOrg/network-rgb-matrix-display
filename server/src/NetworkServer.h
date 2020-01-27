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
  static NetworkServerConfig GenerateConfigFromFile(const char *aFile) {
    NetworkServerConfig svrConfig;
    int error = ini_parse(aFile, ini_file_handler, &svrConfig);
    if (error != 0) {
      fprintf(stderr, "Fatal Error: Can't parse %s. Error code %i.\n", aFile, error);
      fflush(stderr);
      exit(1);
    }

    svrConfig.totalSinglePanelSize = svrConfig.singlePanelHeight * svrConfig.singlePanelWidth;
    svrConfig.totalPixels = svrConfig.totalSinglePanelSize * svrConfig.numPanelsWide * svrConfig.numPanelsTall;

    return svrConfig;
  }
public:
  uint8_t  mSegmentId;
  uint16_t mSinglePanelWidth;
  uint16_t mSinglePanelHeight;
  uint16_t mPixelsPerPanel;

  uint16_t mPanelsWide;
  uint16_t mPanelsTall;

  uint16_t mSegmentWidth;
  uint16_t mSegmentHeight;

  uint16_t mTotalPixels;

  size_t mTotalBytes;


  uint16_t mIncomingPort;

  volatile uint32_t mNumberSamples;
  volatile double mTotalDelta;
  volatile double mAverage;
  volatile double mPriorAverage;

public:
  explicit NetworkServer(struct NetworkServerConfig config);

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
