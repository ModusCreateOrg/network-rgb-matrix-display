#ifndef MATRIX_SERVER_NETWORKSERVERCONFIG_H
#define MATRIX_SERVER_NETWORKSERVERCONFIG_H



#include "MatrixSegment.h"
#include "ini.h"
#include "RGBM

#define MATCH_GROUP(s) strcmp(aSection, s) == 0
#define MATCH_CONFIG(s) strcmp(aName, s) == 0
static int ini_file_handler(void* aConfig, const char* aSection, const char* aName, const char* aValue);


typedef struct NetworkServerConfig {
  uint16_t singlePanelWidth;
  uint16_t singlePanelHeight;
  uint16_t numPanelsWide;
  uint16_t numPanelsTall;
  uint16_t segmentId;
  uint16_t incomingPort;
  uint16_t totalSinglePanelSize;
  uint16_t totalPixels;
  const char *ip;
  unsigned char port;
  MatrixSegment *matrixStripInstance;

  RGBMatrix::Options *matrix_options;

  // TODO @jgarcia get the rest of the optoins in here
} NetworkServerConfig;


static int ini_file_handler(void* aConfig, const char* aSection, const char* aName, const char* aValue) {

  auto *serverConfig = (NetworkServerConfig*)aConfig;

  if (MATCH_GROUP("matrix_dimensions")) {
    if (MATCH_CONFIG("width")) {
      serverConfig->singlePanelWidth = atoi(aValue);
      return 1;
    }
    if (MATCH_CONFIG("height")) {
      serverConfig->singlePanelHeight = atoi(aValue);
      return 1;
    }
    return 0;
  }

  if (MATCH_GROUP("segment_info")) {
    if (MATCH_CONFIG("panels_wide")) {
      serverConfig->numPanelsWide = atoi(aValue);
      return 1;
    }
    if (MATCH_CONFIG("panels_tall")) {
      serverConfig->numPanelsTall = atoi(aValue);
      return 1;
    }

    return 0;
  }

  if (MATCH_GROUP("network")) {
    if (MATCH_CONFIG("ip")) {
      serverConfig->ip = strdup(aValue);
      return 1;
    }
    if (MATCH_CONFIG("port")) {
      serverConfig->port = atoi(aValue);
      return 1;
    }
  }



  return 0;
}


#endif //MATRIX_SERVER_NETWORKSERVERCONFIG_H
