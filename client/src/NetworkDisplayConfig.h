#ifndef NETWORK_DISPLAY_CLIENT_NETWORKDISPLAYCONFIG_H
#define NETWORK_DISPLAY_CLIENT_NETWORKDISPLAYCONFIG_H


#include "ini.h"


#define MATCH_GROUP(s) strcmp(aSection, s) == 0
#define MATCH_CONFIG(s) strcmp(aName, s) == 0

static int ini_file_handler(void* aConfig, const char* aSection, const char* aName, const char* aValue);


typedef struct {
  char *destinationPort;
  char *destinationIp;
} NetworkDisplaySegment;

typedef struct NetworkDisplayConfig {

  size_t   inputBufferSize;

  uint16_t inputStreamWidth;
  uint16_t inputStreamHeight;

  uint16_t outputScreenWidth;
  uint16_t outputScreenHeight;

  uint16_t singlePanelWidth;
  uint16_t singlePanelHeight;

  uint16_t totalPanelsWide;
  uint16_t totalPanelsTall;

  uint16_t totalSegments;
  int frameRate;

  uint16_t segmentPanelsWide;
  uint16_t segmentPanelsTall;

  char *destinationPort;
  char *destinationIP;
  uint16_t destinationIpStartDigit;

  NetworkDisplaySegment *segments;

  void Dump() {
    printf("NetworkDisplayConfig: \n");
    printf("\tframeRate %i\n", frameRate);

    printf("\tinputBufferSize %lu\n", inputBufferSize);
    printf("\tinputStreamWidth %i\n", inputStreamWidth);
    printf("\tinputStreamHeight %i\n", inputStreamHeight);
    printf("\toutputScreenWidth %i\n", outputScreenWidth);
    printf("\toutputScreenHeight %i\n", outputScreenHeight);
    printf("\tsinglePanelWidth %i\n", singlePanelWidth);
    printf("\tsinglePanelHeight %i\n", singlePanelHeight);
    printf("\ttotalPanelsWide %i\n", totalPanelsWide);
    printf("\ttotalPanelsTall %i\n", totalPanelsTall);
    printf("\tsegmentPanelsWide %i\n", segmentPanelsWide);
    printf("\tsegmentPanelsTall %i\n", segmentPanelsTall);
    printf("\tSegments:\n");

    for (int i = 0; i < totalSegments; i++) {
      printf("\t\tSegment %i IP: %s\n", i, segments[i].destinationIp);
      printf("\t\tSegment %i Port: %s\n", i, segments[i].destinationPort);
//      fflush(stdout);
    }

  }
} NetworkDisplayConfig;



static int ini_file_handler(void* aConfig, const char* aSection, const char* aName, const char* aValue) {

  auto *displayConfig = (NetworkDisplayConfig*)aConfig;

  if (MATCH_GROUP("input_stream")) {
    if (MATCH_CONFIG("width")) {
      displayConfig->inputStreamWidth = atoi(aValue);
      return 1;
    }
    if (MATCH_CONFIG("height")) {
      displayConfig->inputStreamHeight = atoi(aValue);
      return 1;
    }
    return 0;
  }

  if (MATCH_GROUP("matrix_dimensions")) {
    if (MATCH_CONFIG("width")) {
      displayConfig->singlePanelWidth = atoi(aValue);
      return 1;
    }
    if (MATCH_CONFIG("height")) {
      displayConfig->singlePanelHeight = atoi(aValue);
      return 1;
    }
    return 0;
  }

  if (MATCH_GROUP("segment_info")) {
    if (MATCH_CONFIG("panels_wide")) {
      displayConfig->segmentPanelsWide = atoi(aValue);
      return 1;
    }
    if (MATCH_CONFIG("panels_tall")) {
      displayConfig->segmentPanelsTall = atoi(aValue);
      return 1;
    }

    return 0;
  }

  if (MATCH_GROUP("output_stream")) {
    if (MATCH_CONFIG("frame_rate")) {
      displayConfig->frameRate = atoi(aValue);
      return 1;
    }
    if (MATCH_CONFIG("total_panels_wide")) {
      displayConfig->totalPanelsWide = atoi(aValue);
      return 1;
    }
    if (MATCH_CONFIG("total_panels_tall")) {
      displayConfig->totalPanelsTall = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("total_segments")) {
      displayConfig->totalSegments = atoi(aValue);
      displayConfig->segments = (NetworkDisplaySegment *)malloc(displayConfig->totalSegments * sizeof(NetworkDisplaySegment));
      return 1;
    }

    return 0;
  }


//   Found a segment
  if (strstr(aSection, "segment_num_") != NULL) {
    int segmentNum = -1;

    char *buff = strdup(aSection);
    char *token;

    int i = 0;
    while ((token = strsep(&buff, "_")) != NULL) {
      if (i == 2) {
        segmentNum = atoi(token);
        break;
      }

      i++;
    }

    if (segmentNum != -1) {
      // Too many segments!
      if (segmentNum > displayConfig->totalSegments) {
        fprintf(
        stderr,
        "Fatal error! Segment %i found in ini file, but greater than total segments of %i.\n",
        segmentNum,
        displayConfig->totalSegments
        );

        return 1;
      }

      if (MATCH_CONFIG("ip")) {
        displayConfig->segments[segmentNum - 1].destinationIp = strdup(aValue);
        return 1;
      }
      if (MATCH_CONFIG("port")) {
        displayConfig->segments[segmentNum - 1].destinationPort = strdup(aValue);
        return 1;
      }
    }
    else {
      fprintf(stderr, "Fatal Error! Could not find segment number in group [%s]\n", aSection);
      return 1;
    }

  }
  return 0;
}




#endif //NETWORK_DISPLAY_CLIENT_NETWORKDISPLAYCONFIG_H
