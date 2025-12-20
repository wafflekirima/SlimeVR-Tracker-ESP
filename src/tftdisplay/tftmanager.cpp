#include "tftmanager.h"
#include "logging/Logger.h"
#include "GlobalVars.h"
#include <math.h>

SlimeVR::Logging::Logger tftLog{"TFTManager"};

namespace SlimeVR {

void TFTManager::drawWelcome() {
    #ifndef useTFTDisplay
        return;
    #endif
    tft.fillScreen(TFT_BLACK);
    tft.drawString("v." FIRMWARE_VERSION, 0, 0, 1);
    tft.setTextSize(4.0f);
    tft.setTextColor(TFT_PINK);
	tft.setFreeFont(FF26);
    //tft.drawString("WG", 30, 10);
	tft.drawCentreString("SVR", displayW / 2, 6, 1);
    tft.setTextSize(2.0f);
	tft.setFreeFont(FSBI12);
    tft.setTextColor(TFT_PURPLE);
    tft.drawCentreString("SlimeVR", displayW / 2, 93, 1);
}

void TFTManager::setup() {
    #ifndef useTFTDisplay
        return;
    #endif
    tft.init();
    tftLog.info("TFT initial PIN CS %d , SCLK %d, MOSI %d, MISO %d , RST %d", TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO, TFT_RST);
    tft.setRotation(1);
    displayW = tft.width(); displayH = tft.height();
    tftLog.info("TFT Display %d x %d", displayW, displayH);

    pinMode(TFT_BUTTON_PIN1, INPUT_PULLUP);
    pinMode(TFT_BUTTON_PIN2, INPUT_PULLUP);

    drawWelcome();
}

int calculateLatency(int rssi) {
  // Convert RSSI to distance (in meters)
  double distance = pow(10, (rssi / (-57.55)));
  
  // Estimate latency based on distance and network characteristics
  int latency = distance * 10; // Example: 10ms per meter
  
  return latency;
}

void TFTManager::update() {
    #ifndef useTFTDisplay
        return;
    #endif

    if(cycle < drawCycle) { cycle++; return; }
    cycle = 0;
    bool BTN1 = digitalRead(TFT_BUTTON_PIN1) == LOW;
    bool BTN2 = digitalRead(TFT_BUTTON_PIN2) == LOW;
    if(BTN1) { debugIndex--; if(debugIndex < 0) debugIndex = debugMaxIndex; }
    if(BTN2) { debugIndex++; if(debugIndex > debugMaxIndex) debugIndex = 0; }

    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_PINK);
    tft.drawString("SVR", 15, 5, 1);
    tft.setTextSize(2.5);
    tft.setTextColor(TFT_PURPLE);
    tft.drawString("SlimeVR", 10, 55, 2);

    //connection status
	tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    int wnX = 115;
    int wnY = 50;
	switch (wifiNetwork.getWiFiState())
	{
        case SlimeVR::WiFiNetwork::WiFiReconnectionStatus::Success:
            wifiStatus = "Connected";
            tft.fillCircle(wnX, wnY, 6, TFT_GREEN);
            break;
        case SlimeVR::WiFiNetwork::WiFiReconnectionStatus::ServerCredAttempt:
            wifiStatus = "Connecting";
            tft.fillCircle(wnX, wnY, 6, TFT_YELLOW);
            break;
        case SlimeVR::WiFiNetwork::WiFiReconnectionStatus::Failed:
            wifiStatus = "Failed";
            tft.fillCircle(wnX, wnY, 6, TFT_ORANGE);
            break;
        default:
            wifiStatus = "Not Connected";
            tft.fillCircle(wnX, wnY, 6, TFT_RED);
            break;
	}
	serverStatus = networkConnection.isConnected()?"Connected":"Not Connected";
    tft.drawString("WiFi : " + wifiStatus, 5, 85, 2);
    tft.drawString("Server : " + serverStatus, 5, 100, 2);
    if(wifiNetwork.isConnected()) {//Draw ping time ms
        tft.drawRightString(String(calculateLatency(WiFi.RSSI())) + " ms", 235, 105, 2);
    }

    //Draw LED Status
    int ledX = 115;
    int ledY = 70;
    tft.drawCircle(ledX, ledY, 6, TFT_BLUE);
    bool ledState = digitalRead(uint8_t (LED_PIN)) == HIGH;
    if(ledState) {
        tft.fillCircle(ledX, ledY, 5, TFT_BLUE);
    }

    //DEBUG CANVAS
    tft.setTextSize(0);
    tft.setTextColor(TFT_WHITE);
    switch (debugIndex)
    {
    case 1://Battery info
        tft.drawString("Battery Info", debugCanvasW, debugCanvasH, 2);
        tft.drawString("BATT V: " + String(battery.getVoltage()), debugCanvasW, debugCanvasH + 15, 2);
        tft.drawString("BATT L: " + String(battery.getLevel() * 100.0f), debugCanvasW, debugCanvasH + 30, 2);
        break;
    case 2://IMU info
        tft.drawString("IMU info", debugCanvasW, debugCanvasH, 2);
        if(sensorManager.getSensors().size() > 0) {
            for(auto& sensor : sensorManager.getSensors()) {
                String imuInfo = String(sensor->getSensorId()) + " | " + String(getIMUNameByType(sensor->getSensorType()));
                tft.drawString(imuInfo, debugCanvasW, debugCanvasH + 15 + (sensor->getSensorId() * 15), 2);
                const char* mag = sensor->getAttachedMagnetometer();
                if (mag) {
                    tft.drawString("Mag : " + String(mag), debugCanvasW, debugCanvasH + 15 + (sensor->getSensorId() * 15) + 15, 2);
                }
            }
        } else {
            tft.drawString("No IMU", debugCanvasW, debugCanvasH + 15, 2);
        }
        break;
    case 3://Primary IMU info
        if(sensorManager.getSensors().size() > 0) {
            auto& sensor = sensorManager.getSensors()[0];
            tft.drawString("Primary IMU", debugCanvasW, debugCanvasH, 2);
            String imuInfo = String(sensor->getSensorId()) + " | " + String(getIMUNameByType(sensor->getSensorType()));
            tft.drawString(imuInfo, debugCanvasW, debugCanvasH + 15, 2);
            const char* mag = sensor->getAttachedMagnetometer();
            if (mag) {
                tft.drawString("Mag : " + String(mag), debugCanvasW, debugCanvasH + 30, 2);
            }
            //tft.drawString("ADR : 0x" + String(sensor->m_hwInterface->toString().c_str()), debugCanvasW, debugCanvasH + 45, 2);
        } else {
            tft.drawString("No Primary IMU", debugCanvasW, debugCanvasH + 15, 2);
        }
        break;
    case 4://Secondary IMU info
        if(sensorManager.getSensors().size() > 1) {
            auto& sensor = sensorManager.getSensors()[1];
            tft.drawString("Secondary IMU", debugCanvasW, debugCanvasH, 2);
            String imuInfo = String(sensor->getSensorId()) + " | " + String(getIMUNameByType(sensor->getSensorType()));
            tft.drawString(imuInfo, debugCanvasW, debugCanvasH + 15, 2);
            const char* mag = sensor->getAttachedMagnetometer();
            if (mag) {
                tft.drawString("Mag : " + String(mag), debugCanvasW, debugCanvasH + 30, 2);
            }
            //tft.drawString("ADR : 0x" + String(sensor->m_hwInterface->toString().c_str()), debugCanvasW, debugCanvasH + 45, 2);
        } else {
            tft.drawString("No Secondary IMU", debugCanvasW, debugCanvasH + 15, 2);
        }
        break;
    case 5://Network info
        tft.drawString("Network Info", debugCanvasW, debugCanvasH, 2);
        tft.drawString(String(wifiNetwork.getAddress().toString().c_str()), debugCanvasW, debugCanvasH + 15, 2);
        tft.drawString("RSSI " + String(WiFi.RSSI()) + " dBm", debugCanvasW, debugCanvasH + 30, 2);
        break;

    default:
        break;
    }

    //Battery Level display
    tft.drawRoundRect(0, 120, 240, 8, 5, TFT_GREEN);
    tft.fillRoundRect(1, 121, battery.getLevel() * 239, 8, 5, TFT_GREEN);
    if(battery.getVoltage() > 4.18f) {
        tft.setTextSize(0);
        tft.setTextColor(TFT_BLUE);
        tft.drawCentreString(">>> CHARGING >>>", displayW / 2, 121, 2);
    }
}


void TFTManager::setupState(boolean value) { isSetup = value; }

void TFTManager::drawLog(String message) {// available for setup screen only
    if(!isSetup) return;

	tft.setTextSize(1.0f);
	tft.setTextColor(TFT_WHITE);
	tft.fillRect(0, drawLogPosY, displayW, 16, TFT_BLACK);
	tft.drawCentreString(String(message), displayW / 2, drawLogPosY, 2);
}

}