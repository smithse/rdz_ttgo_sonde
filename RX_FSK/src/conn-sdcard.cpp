#include "../features.h"

#if FEATURE_SDCARD

#include "conn-sdcard.h"
#include <dirent.h>
#include <time.h>

static SPIClass sdspi(HSPI);

char *cardTypeStr(uint8_t cardtype) {
  switch(cardtype) {
     case CARD_NONE: return "No SD card attached";
     case CARD_MMC: return "MMC";
     case CARD_SD: return "SDSC";
     case CARD_SDHC: return "SDHC";
     default: return "unknown";
  }
}

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
  Serial.printf("SD Card Type: %s\n", cardTypeStr(cardType));
  if (cardType == CARD_NONE) { return; }

  uint32_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %luMB\n", cardSize);
  uint32_t usedSize = SD.usedBytes() / (1024 * 1024);
  uint32_t totalSize = SD.totalBytes() / (1024 * 1024);
  Serial.printf("SD Card used/total: %lu/%lu MB\n", usedSize, totalSize);

  file = SD.open("/data.csv", FILE_APPEND);
  if (!file) {
    Serial.println("Cannot open file");
    return;
  }
  file.printf("Hello word... test\n");
  file.close();

  //sdf = SD.open("/data.csv", FILE_READ);

#if 0
  // Just testcode
  DIR *dir = opendir("/sd/");
  struct dirent *dent;
  struct stat attr;
  char fname[1024];
  strcpy(fname,"/sd/");
  if(dir) {
    while((dent=readdir(dir))!=NULL) {
      strcpy(fname+4, dent->d_name);
      stat(fname, &attr);
      char ftim[50];
      strftime(ftim, 50, "%Y-%m-%d %H:%M:%S", gmtime(&attr.st_mtime)); 
      printf("%s %s %d\n", dent->d_name, ftim, attr.st_size);
      
    }
    closedir(dir);
  }
#endif
}

void ConnSDCard::netsetup() {
  /* empty function, we don't use any network here */
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

String ConnSDCard::getStatus() {
  if(sonde.config.sd.cs == -1) { return String("Disabled"); }
  if(!initok) { return String("SD card init failed"); }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) { return String(cardTypeStr(cardType)); }

  uint32_t cardSize = SD.cardSize() / (1024 * 1024);
  uint32_t usedSize = SD.usedBytes() / (1024 * 1024);
  uint32_t totalSize = SD.totalBytes() / (1024 * 1024);
  char buf[256];
  snprintf(buf, 256, "SD card type: %s [size: %lu MB]. File system: %lu / %lu MB free", cardTypeStr(cardType),
    cardSize, usedSize, totalSize);
  return String(buf);
}

String ConnSDCard::getName() {
  return String("SD Card");
}


ConnSDCard connSDCard;

#endif
