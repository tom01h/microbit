#include "MicroBit.h"
#include "ff.h"
#include "diskio.h"

MicroBit uBit;
SPI spi(MOSI, MISO, SCK);
MicroBitPin cs(MICROBIT_ID_IO_P16, MICROBIT_PIN_P16, PIN_CAPABILITY_DIGITAL);
DSTATUS sd_init=STA_NOINIT;

int main() {
  FIL fil;
  FATFS fatfs;
  FRESULT Res;
  TCHAR *Path = "0:/";
  unsigned char buff[512];
  UINT NumBytesRead;
  UINT FileSize = 512;

  uBit.init();

  uBit.serial.printf("Start SD Test!!\r\n");

  const char* buf="def.h";

  Res = f_mount(&fatfs, Path, 0);

  if(!Res){
    Res = f_open(&fil, buf, FA_READ);
  }else{
    uBit.serial.printf("f_mount error %02x\r\n",Res);
  }
  if(!Res){
    Res = f_read(&fil, buff, FileSize, &NumBytesRead);
  }else{
    uBit.serial.printf("f_open error %02x\r\n",Res);
  }

  if(!Res){
    for(int i=0;i<(NumBytesRead-1)/16+1;i++){
      for(int j=0;j<16;j++){
        uBit.serial.printf("%02x ",buff[i*16+j]);
        if(i*16+j==NumBytesRead-1){break;}
      }
      uBit.serial.printf("\r\n");
    }
    uBit.serial.printf("\r\n");
  }else{
        uBit.serial.printf("f_read error %02x\r\n",Res);
  }

  uBit.serial.printf("Fin!!\r\n");

  release_fiber();
}
