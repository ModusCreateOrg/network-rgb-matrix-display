#ifndef MATRIX_SERVER_NETWORKSERVERCONFIG_H
#define MATRIX_SERVER_NETWORKSERVERCONFIG_H



#include "MatrixSegment.h"
#include "INIReader.h"
#include "led-matrix.h"



class NetworkServerConfig {
public:
    static NetworkServerConfig *FromIniFile(const char *aFile) {
      auto *serverConfig = new NetworkServerConfig();

      auto *reader = new INIReader(aFile);

      serverConfig->autoClearDisplay = (bool)reader->GetBoolean("general", "auto_clear_display", true);
      serverConfig->autoClearDelay = (unsigned long)reader->GetInteger("general", "auto_clear_delay", 50000000);


      serverConfig->singlePanelWidth  = (int)reader->GetInteger("matrix_dimensions", "width", 0);
      serverConfig->singlePanelHeight  = (int)reader->GetInteger("matrix_dimensions", "height", 0);
      serverConfig->numPanelsWide = (int)reader->GetInteger("segment_info", "panels_wide", 0);
      serverConfig->numPanelsTall = (int)reader->GetInteger("segment_info", "panels_tall", 0);
//      printf("ip %s\n", reader->GetString("network", "ip", "").c_str());
//      fflush(stdout);

      serverConfig->ip = strdup(reader->GetString("network", "ip", "").c_str());
      serverConfig->port = (unsigned short)reader->GetInteger("network", "port", 0);
      serverConfig->incomingPort = (int)reader->GetInteger("network", "port", 0);

      const char *libOptions = "rgb_matrix_lib_options";

      serverConfig->matrix_options.hardware_mapping = strdup(reader->GetString(libOptions, "hardware_mapping", "").c_str());
      serverConfig->matrix_options.parallel = (int)reader->GetInteger(libOptions, "parallel", 1);
//      serverConfig->matrix_options.chain_length = (int)reader->GetInteger("matrix_dimensions", "chain_length", 1);

      serverConfig->matrix_options.chain_length = serverConfig->numPanelsWide > serverConfig->numPanelsTall ? serverConfig->numPanelsWide : serverConfig->numPanelsTall;

      serverConfig->matrix_options.pwm_bits = (int)reader->GetInteger(libOptions, "pwm_bits", 0);
      serverConfig->matrix_options.pwm_lsb_nanoseconds = (int)reader->GetInteger(libOptions, "pwm_lsb_nanoseconds", 0);
      serverConfig->matrix_options.pwm_dither_bits = (int)reader->GetInteger(libOptions, "pwm_dither_bits", 0);
      serverConfig->matrix_options.brightness = (int)reader->GetInteger(libOptions, "brightness", 100);
      serverConfig->matrix_options.scan_mode = (int)reader->GetInteger(libOptions, "scan_mode", 0);
      serverConfig->matrix_options.multiplexing = (int)reader->GetInteger(libOptions, "multiplexing", 0);
      serverConfig->matrix_options.disable_hardware_pulsing = (bool)reader->GetBoolean(libOptions, "disable_hardware_pulsing", false);
      serverConfig->matrix_options.show_refresh_rate = (bool)reader->GetInteger(libOptions, "show_refresh_rate", false);
      serverConfig->matrix_options.inverse_colors = (bool)reader->GetInteger(libOptions, "inverse_colors", false);
      serverConfig->matrix_options.led_rgb_sequence = strdup(reader->GetString(libOptions, "led_rgb_sequence", "RGB").c_str());
      serverConfig->matrix_options.pixel_mapper_config = strdup(reader->GetString(libOptions, "pixel_mapper_config", "").c_str());

      const char *runtimeOptions = "rgb_matrix_runtime_options";
      serverConfig->runtime_options.gpio_slowdown = (int)reader->GetInteger(runtimeOptions, "gpio_slowdown", 0);
      serverConfig->runtime_options.daemon = (int)reader->GetInteger(runtimeOptions, "daemon", -1);
      serverConfig->runtime_options.drop_privileges = (int)reader->GetInteger(runtimeOptions, "drop_privileges", -1);
      serverConfig->runtime_options.do_gpio_init = (bool)reader->GetBoolean(runtimeOptions, "do_gpio_init", true);


      return serverConfig;
    }
public:
  NetworkServerConfig() {

    singlePanelWidth = 0;
    singlePanelHeight = 0;

    numPanelsWide = 0;
    numPanelsTall = 0;
    segmentId = 0;
    incomingPort = 0;
//    totalSinglePanelSize = 0;
    totalPixels = 0;
    port = 0;
    ip = nullptr;
    matrixStripInstance = nullptr;

  };
  ~NetworkServerConfig() = default;

  int singlePanelWidth;
  int singlePanelHeight;
  int numPanelsWide;
  int numPanelsTall;
  int segmentId;
  int incomingPort;

  bool autoClearDisplay;
  unsigned long autoClearDelay;

//  int totalSinglePanelSize;
  int totalPixels;
  unsigned short port;
  const char *ip;
  MatrixSegment *matrixStripInstance;

  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_options;

  void Describe() {
    printf("------------\n");
    printf("NetworkServerConfig:\n");
    printf("\tsinglePanelWidth = %i\n", singlePanelWidth);
    printf("\tsinglePanelHeight = %i\n", singlePanelHeight);
    printf("\tnumPanelsWide = %i\n", numPanelsWide);
    printf("\tnumPanelsTall = %i\n", numPanelsTall);
    printf("\tsegmentId = %i\n", segmentId);
    printf("\tincomingPort = %i\n", incomingPort);
//    printf("\ttotalSinglePanelSize = %i\n", totalSinglePanelSize);
    printf("\ttotalPixels = %i\n", totalPixels);
    printf("\tautoClearDisplay = %s\n", autoClearDisplay ? "true" : "false");
    printf("\tautoClearDelay = %lu\n", autoClearDelay);
    printf("\tip = %s\n", ip);
    printf("\tport = %u\n", port);
    printf("--- matrix_options ---\n");
    printf("\tmatrix_options.parallel = %i\n", matrix_options.parallel);
    printf("\tmatrix_options.chain_length = %i\n", matrix_options.chain_length);
    printf("\tmatrix_options.pwm_bits = %i\n", matrix_options.pwm_bits);
    printf("\tmatrix_options.pwm_lsb_nanoseconds = %i\n", matrix_options.pwm_lsb_nanoseconds);
    printf("\tmatrix_options.pwm_dither_bits = %i\n", matrix_options.pwm_dither_bits);
    printf("\tmatrix_options.brightness = %i\n", matrix_options.brightness);
    printf("\tmatrix_options.scan_mode = %i\n", matrix_options.scan_mode);
    printf("\tmatrix_options.row_address_type = %i\n", matrix_options.row_address_type);
    printf("\tmatrix_options.multiplexing = %i\n", matrix_options.multiplexing);
    printf("\tmatrix_options.disable_hardware_pulsing = %i\n", matrix_options.disable_hardware_pulsing);
    printf("\tmatrix_options.show_refresh_rate = %i\n", matrix_options.show_refresh_rate);
    printf("\tmatrix_options.inverse_colors = %i\n", matrix_options.inverse_colors);
    printf("--- runtime_options ---\n");
    printf("\truntime_options.gpio_slowdown = %i\n", runtime_options.gpio_slowdown);
    printf("\truntime_options.daemon = %i\n", runtime_options.daemon);
    printf("\truntime_options.drop_privileges = %i\n", runtime_options.drop_privileges);
    printf("\truntime_options.do_gpio_init = %i\n", runtime_options.do_gpio_init);
    fflush(stdout);

  }

};



#endif //MATRIX_SERVER_NETWORKSERVERCONFIG_H
