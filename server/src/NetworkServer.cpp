/**
 *
 * This class is designed to only run on the RaspberryPi and setup an unencrypted TCP/IP server
 * on port 9890.  It recieves screen buffer data and then copies it to the next available
 * buffer on the MatrixStrip instance after shifting the inbound 16bit color to the desired
 * 32bit color scheme.
 *
 * This thread is also only to run on CPU 3, with a 99 priority.
 */
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include <cstring>
#include <pthread.h>
#include <time.h>

#include "NetworkServer.h"
#include "NetworkServerConfig.h"

using boost::asio::ip::tcp;
using std::min;
using std::max;

NetworkServer::NetworkServer(NetworkServerConfig *config) {
//#ifdef __linux__
  pthread_mutex_destroy(&mMutex);
  pthread_mutex_init(&mMutex, nullptr);
//#endif

  mNumberSamples = 0;
  mTotalDelta = 0;
  mAverage = 0;
  mPriorAverage = 0;
  mThreadRunning = false;

  mFrameCount = 0;
  mSegmentId = config->segmentId;
  mIncomingPort = config->incomingPort;

  mSinglePanelWidth = config->singlePanelWidth;
  mSinglePanelHeight = config->singlePanelHeight;

  mPixelsPerPanel = mSinglePanelWidth * mSinglePanelWidth;

  mSegmentWidth = config->singlePanelWidth * config->numPanelsWide;
  mSegmentHeight = config->singlePanelHeight * config->numPanelsTall;

  mPanelsWide = config->numPanelsWide;
  mPanelsTall = config->numPanelsTall;

  mTotalPixels = mPixelsPerPanel * mPanelsWide * mPanelsTall;
  mTotalBytes = mTotalPixels * sizeof(uint16_t);

  mMatrixStrip = config->matrixStripInstance;

  mPort = config->port;
  printf("IP is %s\n", config->ip);
  fflush(stdout);
  mIP = strdup(config->ip);


#ifdef __MATRIX_SHOW_DEBUG_MESSAGES__
  Describe();
#endif
}

uint16_t nColor = 0;
int nX = 0;
int nY = 0;

void NetworkServer::ReceiveDataThread(tcp::socket sock) {
  int numBytesReceived = 0;
  uint16_t sbIndex = 0;

  volatile clock_t start = 0;
  volatile clock_t end = 0;

  // 32768 = ((NumPixelsWide * NmPixelsTall) * NumPixelsInSegment) * SizeOf(uint16_t)
  //       = ((64 * 64) * 4) * 2
//  static uint16_t data[32768];
//  memset(data,0,mTotalBytes);
  auto *data = (uint16_t *)malloc(mTotalBytes);
  bzero(data, mTotalBytes);
  const char *returnData = "K";

  while (GetThreadRunning()) {
    nColor = random() & UINT16_MAX;

    try {
      nColor++;
      start = clock();

      boost::system::error_code error;
      size_t length = boost::asio::read(sock, boost::asio::buffer(data, mTotalBytes), boost::asio::transfer_exactly(mTotalBytes), error);

      numBytesReceived += length;
//      printf("numBytesReceived %i\n", numBytesReceived);
      // Ended early! No bueno!
      if (error == boost::asio::error::eof){
        printf("Eof\n"); fflush(stdout);
        break;
      }
      else if (error) {
        throw boost::system::system_error(error); // Some other error.
      }


      boost::asio::write(sock, boost::asio::buffer(returnData, 1));




      // this thing re-orients the data so that the pixels are mapped vertically
      if (numBytesReceived == mTotalBytes) {
        end = clock();
        mNumberSamples++;
        double delta = (end - start);
        mTotalDelta += delta;
        mAverage = (mTotalDelta / mNumberSamples) ;

          // This is some debugging crap.
//        uint8_t r = (nColor & 0xF800) >> 8;       // rrrrr... ........ -> rrrrr000
//        uint8_t g = (nColor & 0x07E0) >> 3;       // .....ggg ggg..... -> gggggg00
//        uint8_t b = (nColor & 0x1F) << 3;         // ............bbbbb -> bbbbb000
//        printf("Here\n");
//        mMatrixStrip->GetRenderCanvas()->Fill(r,g,b);
//        mMatrixStrip->mFrameCount++;
//
//        break;




//
//        int ptrIndex = mTotalPixels - 1;
//        uint16_t *outputBuff = data;
//
//        int y = 0;
//        int x = mMatrixStrip->mCanvasWidth - 1;
//        for (; x > -1 ; x--) {
////          printf("x %i\n", x);
//          y = 0;
//          for (; y < mMatrixStrip->mCanvasHeight; y++) {
//
//            uint16_t pixel = outputBuff[ptrIndex--];
//            // Color separation based off : https://stackoverflow.com/questions/38557734/how-to-convert-16-bit-hex-color-to-rgb888-values-in-c
//            uint8_t r = (pixel & 0xF800) >> 8;       // rrrrr... ........ -> rrrrr000
//            uint8_t g = (pixel & 0x07E0) >> 3;       // .....ggg ggg..... -> gggggg00
//            uint8_t b = (pixel & 0x1F) << 3;         // ............bbbbb -> bbbbb000
//
//            mMatrixStrip->GetRenderCanvas()->SetPixel(x, y, r, g, b);
//          }
//
//        }

        const int width = mMatrixStrip->GetRenderCanvas()->width();
        const int height =  mMatrixStrip->GetRenderCanvas()->height();
        uint16_t *outputBuff = data;
        int ptrIndex = 0;
        int y = 0;
        for (int x = 0; x < width ; x++) {
          y = 0;
          for (; y < mMatrixStrip->mCanvasHeight; y++) {

            uint16_t pixel = outputBuff[ptrIndex++];
            // Color separation based off : https://stackoverflow.com/questions/38557734/how-to-convert-16-bit-hex-color-to-rgb888-values-in-c
            uint8_t r = (pixel & 0xF800) >> 8;       // rrrrr... ........ -> rrrrr000
            uint8_t g = (pixel & 0x07E0) >> 3;       // .....ggg ggg..... -> gggggg00
            uint8_t b = (pixel & 0x1F) << 3;         // ............bbbbb -> bbbbb000

            mMatrixStrip->GetRenderCanvas()->SetPixel(x, y, r, g, b);
          }

        }

//        const int width = mMatrixStrip->GetRenderCanvas()->width();
//        const int height =  mMatrixStrip->GetRenderCanvas()->height();
//
//        mMatrixStrip->GetRenderCanvas()->Fill(0,0,0);
//
//        nX++;
//
//        if (nX > width) {
//          nX = 0;
//          nY++;
//          if (nY > height) {
//            nY = 0;
//          }
//        }

        mMatrixStrip->GetRenderCanvas()->SetPixel(nX, nY, random() & 0xFF, random() & 0xFF, random() & 0xFF);
//        if ((mFrameCount % 10) == 0) {
//          printf("width %i, height %i\n", width, height);
//          printf("%i\n",mFrameCount);
//        }
        mMatrixStrip->mFrameCount++;
        break;
      }

    }
    catch (std::exception& e) {
      std::cerr <<  __FUNCTION__ << " Exception: " << e.what() << "\n";
    }
  }

  delete data;
  mFrameCount++;
}


void NetworkServer::ServerStartingThread() {
  mThreadRunning = true;

  boost::asio::io_context io_context;
  unsigned short port = mPort;
  tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));

  sched_param sch_params;
  sch_params.sched_priority = 99;

  int affinity_mask = (1<<2);

  cpu_set_t cpu_mask;
  CPU_ZERO(&cpu_mask);

  for (int i = 0; i < 32; ++i) {
    if ((affinity_mask & (1<<i)) != 0) {
      CPU_SET(i, &cpu_mask);
    }
  }

  while (mThreadRunning) {
    mThread = std::thread(&NetworkServer::ReceiveDataThread, this, a.accept());
    pthread_setschedparam(mThread.native_handle(), SCHED_FIFO, &sch_params);

    int err;

    if ((err=pthread_setaffinity_np(mThread.native_handle(), sizeof(cpu_mask), &cpu_mask))) {
      fprintf(stderr, "FYI: Couldn't set affinity 0x%x: %s\n",
              affinity_mask, strerror(err));
    }

    mThread.detach();
  }
}

void NetworkServer::StartThread() {
  sched_param sch_params;
  sch_params.sched_priority = 99;

  int affinity_mask = (1<<1);
  cpu_set_t cpu_mask;
  CPU_ZERO(&cpu_mask);

  for (int i = 0; i < 32; ++i) {
    if ((affinity_mask & (1<<i)) != 0) {
      CPU_SET(i, &cpu_mask);
    }
  }

  int err;

  std::thread starterThread = std::thread(&NetworkServer::ServerStartingThread, this);

  pthread_setschedparam(starterThread.native_handle(), SCHED_FIFO, &sch_params);

  if ((err=pthread_setaffinity_np(starterThread.native_handle(), sizeof(cpu_mask), &cpu_mask))) {
    fprintf(stderr, "FYI: Couldn't set affinity 0x%x: %s\n",
            affinity_mask, strerror(err));
  }


  starterThread.detach();

}


void NetworkServer::StopThread() {

  mThreadRunning = false;
  if (mThread.joinable()) {
    mThread.join();
  }
  usleep(100);

}

void NetworkServer::LockMutex() {
  printf("%i NetworkServer::%s %p\n", mSegmentId, __FUNCTION__, &mMutex);
  pthread_mutex_lock(&mMutex);
}

void NetworkServer::UnlockMutex() {
  printf("NetworkServer::%s\n", __FUNCTION__); fflush(stdout);

  pthread_mutex_unlock(&mMutex);
}

void NetworkServer::Describe() {
  printf("---------\n");
  printf("NetworkServer %p\n", this);
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
  printf("\tmIncomingPort = %i\n", mIncomingPort);
  printf("\n");
  fflush(stdout);
}

NetworkServer::~NetworkServer() {
  StopThread();
}
