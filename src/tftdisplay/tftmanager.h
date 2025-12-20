#ifndef SLIMEVR_TFTMANAGER_H
#define SLIMEVR_TFTMANAGER_H

#include <SPI.h>
#include "tftdefines.h"
#include <TFT_eSPI.h>
#include "fffont.h"
//#include "logging/Logger.h"

namespace SlimeVR {
    class TFTManager {
        public:
            void setup();
            void update();
            void setupState(boolean value);
            void drawLog(String message);
        private:
            TFT_eSPI tft = TFT_eSPI();
            int cycle = 0;
            int drawCycle = 200; //ms
            boolean isSetup = false;
            int drawLogPosY = 80;
            int16_t displayW = 0;
            int16_t displayH = 0;
            String serverStatus = "";
            String wifiStatus = "";
            int debugIndex = 0;
            int debugCanvasW = 135;
            int debugCanvasH = 5;
            int debugMaxIndex = 5;

            void drawWelcome();
    };
}

#endif  // SLIMEVR_TFTMANAGER_H