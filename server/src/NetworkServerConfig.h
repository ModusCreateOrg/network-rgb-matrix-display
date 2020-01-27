#ifndef MATRIX_SERVER_NETWORKSERVERCONFIG_H
#define MATRIX_SERVER_NETWORKSERVERCONFIG_H



#include "MatrixSegment.h"
#include "ini.h"
#include "led-matrix.h"


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
  rgb_matrix::RuntimeOptions *runtime_options;

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

  if (MATCH_GROUP("rgb_matrix_lib_options")) {
    if (MATCH_CONFIG("hardware_mapping")) {
      serverConfig->matrix_options->hardware_mapping = strdup(aValue);
      return 1;
    }

    if (MATCH_CONFIG("parallel")) {
      serverConfig->matrix_options->parallel = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("pwm_bits")) {
      serverConfig->matrix_options->parallel = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("pwm_lsb_nanoseconds")) {
      serverConfig->matrix_options->pwm_lsb_nanoseconds = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("brightness")) {
      serverConfig->matrix_options->brightness = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("scan_mode")) {
      serverConfig->matrix_options->scan_mode = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("row_address_type")) {
      serverConfig->matrix_options->row_address_type = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("multiplexing")) {
      serverConfig->matrix_options->multiplexing = atoi(aValue);
      return 1;
    }


    if (MATCH_CONFIG("disable_hardware_pulsing")) {
      serverConfig->matrix_options->disable_hardware_pulsing = (bool)atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("show_refresh_rate")) {
      serverConfig->matrix_options->show_refresh_rate = (bool)atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("inverse_colors")) {
      serverConfig->matrix_options->inverse_colors = (bool)atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("led_rgb_sequence")) {
      serverConfig->matrix_options->led_rgb_sequence =  strdup(aValue);
      return 1;
    }

    if (MATCH_CONFIG("pixel_mapper_config")) {
      if (strlen(aValue) > 0 && strcmp(aValue, "NULL") != 0) {
        serverConfig->matrix_options->pixel_mapper_config =  strdup(aValue);
        return 1;
      }
      // Silently fail for now. Please open github issue and tag @jaygarcia. =)
      return 1;
    }


    if (MATCH_CONFIG("brightness")) {
      serverConfig->matrix_options->brightness = atoi(aValue);
      return 1;
    }

    return 0;
  }

  if (MATCH_GROUP("rgb_matrix_runtime_options")) {
    if (MATCH_CONFIG("gpio_slowdown")) {
      serverConfig->runtime_options->gpio_slowdown = atoi(aValue);
      return 1;
    }
    if (MATCH_CONFIG("daemon")) {
      serverConfig->runtime_options->daemon = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("drop_privileges")) {
      serverConfig->runtime_options->drop_privileges = atoi(aValue);
      return 1;
    }

    if (MATCH_CONFIG("do_gpio_init")) {
      serverConfig->runtime_options->do_gpio_init = (bool)atoi(aValue);
      return 1;
    }
    return 0;
  }

  return 0;
}


#endif //MATRIX_SERVER_NETWORKSERVERCONFIG_H
