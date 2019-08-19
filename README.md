
# Network Matrix Display 

Network Matrix Display is a set of libraries that allows you create a scalable network of matrix displays using Raspberry Pi Single Board Computers.  It does this by using TCIP/IP as a transport method for the display data and is broken into two discrete sections; Client and Server. 

Please refer to our [how it works](./md/How_it_works.md) document for a detailed description of this projects functionality.

This project is choc-full of dependencies and is not just for the faint of heart.  It requires you to put in effort to set things up.

It's worth noting that the scale of [our project](./md/example_project.md) was not the cheapest solution for the desired result.  The purpose of this project was to demonstrate what can be done with a hand full of available parts and open source libraries.  
For the cheapest solution, we recommend display controllers like [these](https://www.aliexpress.com/item/32922416742.html).

## Related documents: 
- [How it works](./md/How_it_works.md)
- [Client Setup Guide](./md/Client_setup_guide.md)
- [Server Setup Guide](./md/Server_setup_guide.md)
- [Display Setup Guide](./md/Display_setup_guide.md)

## General requirements 
- Good knowledge in C++
- Basic understanding of how RGB Matrices work
- Parts for your project

## Client Requirements:
- [LibBoost 1.7.0](https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz)
- [CMake 3.14+](https://github.com/Kitware/CMake/releases/download/v3.14.4/cmake-3.14.4.tar.gz)
- [SDL2 (optional)](https://www.libsdl.org/download-2.0.php)

## Server Requirements
- [LibBoost 1.7.0](https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz)
- [CMake 3.14+](https://github.com/Kitware/CMake/releases/download/v3.14.4/cmake-3.14.4.tar.gz)
- [rpi-rgb-led-matrix library](https://github.com/hzeller/rpi-rgb-led-matrix)
- [DietPi or similar light-weight linux distribution](https://dietpi.com/)
- [Raspberry Pi 2 or greater SBC](https://www.raspberrypi.org/products/raspberry-pi-3-model-b-plus/)
- [Electro Dragon RGB Panel driver board](https://www.electrodragon.com/product/rgb-matrix-panel-drive-board-raspberry-pi/) (*recommended*)
- [1+ RGB Matrices](https://www.adafruit.com/product/420)

## Additional hardware
- For Full Motion video, GigaBit Network switch
- For low-framerate animations or periodic switches to static images, WIFI is fine
- Frame for your matrix
- Power supplies for your SBCs
- Power supply for your RGB Matrix

## Installation
Install DietPi on a Raspberry Pi 2 or greater. Make sure it has Internet connectivity. Then run this script to provision the system and install the Network Matrix Display project:

    curl https://raw.githubusercontent.com/ModusCreateOrg/network-rgb-matrix-display/master/bin/install.sh | bash

## Client example.
The following example will generate the proper network display client configuration and kick off 
```
  NetworkDisplayConfig displayConfig;

  displayConfig.frameRate = 60; // -1 to disable

  // This is your source display dimensions
  displayConfig.inputScreenWidth = 320;
  displayConfig.inputScreenHeight = 240;

  // Dimensions for your panels
  displayConfig.singlePanelWidth = 64;
  displayConfig.singlePanelHeight = 64;

  // How you will lay out your segments
  displayConfig.segmentPanelsTall = 3;
  displayConfig.segmentPanelsWide = 1;

  // How you will lay out your panels per segment
  displayConfig.totalPanelsWide = 5;
  displayConfig.totalPanelsTall = 3;

  displayConfig.totalSegments = 5;

  displayConfig.destinationPort = "9890";
  
  // In this scheme, the last IP address octet rolls up for multiple segments. 
  // Segment 1 is 201, segment 2 is 202, etc..
  displayConfig.destinationIP = "10.0.1.20%i";
  displayConfig.destinationIpStartDigit = 1;

  displayConfig.outputScreenWidth = displayConfig.singlePanelWidth * displayConfig.totalPanelsWide;
  displayConfig.outputScreenHeight = displayConfig.singlePanelHeight * displayConfig.totalPanelsTall;

  NetworkDisplay *networkDisplay = new NetworkDisplay(displayConfig);

  std::thread(interrupterThread).detach();

  uint16_t color = 0;
  while (! interrupt_received) {
    // Your code could easily populate the input buffer (array of uint16_t) with the pixel data
    memset(networkDisplay->GetInputBuffer(), color += 1, networkDisplay->GetInputBufferSize());
    
    // Flush the display buffer to the network
    networkDisplay->Update();
  }
```

## License
This library is licensed under [MIT](./LICENSE).

# Contributing
Interested in contributing? Please see our [contribution](.github/CONTRIBUTING.md) and [code of conduct](.github/CODE_OFCONDUCT.md) guidelines. 


[![Modus Create](./md/img/modus.logo.svg)](https://moduscreate.com)
