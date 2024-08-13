#include "../features.h"

#if FEATURE_SDCARD

#include "conn-sdcard.h"

static SPIClass sdspi(HSPI);

void ConnSDCard::init() {
  if(sonde.config.sd.clk==-1)
    return;
  /* Initialize SD card */
  // SPI (==VSPI) is used by SX127x. 
  // On LoRa32, SD-Card is on different pins, so cannot share VSPI
  // Use HSPI (if using a TFT with SPI, you have to make sure that the same pins are used for both (MISO/MOSI/CLK) 
  sdspi.begin(sonde.config.sd.clk, sonde.config.sd.miso, sonde.config.sd.mosi, sonde.config.sd.cs);
  initok = SD.begin(sonde.config.sd.cs, sdspi);
  Serial.printf("SD card init: %s\n", initok ? "OK" : "Failed");
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %luMB\n", cardSize);
  uint64_t usedSize = SD.usedBytes() / (1024 * 1024);
  //uint64_t totalSize = SD.totalBytes() / (1024 * 1024);
  uint64_t totalSize = SD.totalBytes();
  Serial.printf("SD Card used/total: %lu/%lu MB\n", usedSize, totalSize);

  file = SD.open("/data.csv", FILE_APPEND);
  if (!file) {
    Serial.println("Cannot open file");
    return;
  }
  file.printf("Hello word... test\n");
  file.close();

  //sdf = SD.open("/data.csv", FILE_READ);
}

void ConnSDCard::netsetup() {
  /* empty function, we don't use any network here */
}

String ConnSDCard::getStatus() {
  return String("");
}

// Rotation by time or by id.

void ConnSDCard::updateSonde( SondeInfo *si ) {
  if (!initok) return;
  if (!file) {
    file = SD.open("/data.csv", FILE_APPEND);
  }
  if (!file) {
    Serial.println("Error opening file");
    return;
  }
  SondeData *sd = &si->d;
  file.printf("%d,%s,%s,%d,"
              "%f,%f,%f,%f,%f,%f,%d,%d,"
              "%d,%d,%d,%d\n",
              sd->validID, sd->ser, sd->typestr, sd->subtype,
              sd->lat, sd->lon, sd->alt, sd->vs, sd->hs, sd->dir, sd->sats, sd->validPos,
              sd->time, sd->frame, sd->vframe, sd->validTime);
  wcount++;
  if (wcount >= sonde.config.sd.sync) {
    file.flush();
    wcount = 0;
  }
}


void ConnSDCard::updateStation( PosInfo *pi ) {
}


ConnSDCard connSDCard;

#endif
