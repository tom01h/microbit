#include "MicroBit.h"
#include "defines.h"
#include "ff.h"
#include "diskio.h"

MicroBit uBit;
SPI spi(MOSI, MISO, SCK);
MicroBitPin cs(MICROBIT_ID_IO_P16, MICROBIT_PIN_P16, PIN_CAPABILITY_DIGITAL);
MicroBitPin miso(MISO, MICROBIT_PIN_P14, PIN_CAPABILITY_DIGITAL);
DSTATUS sd_init=STA_NOINIT;

MicroBitImage BS("\
        1 1 1 1 1\n\
        1 0 0 0 1\n\
        1 0 0 0 1\n\
        1 0 0 0 1\n\
        1 1 1 1 1\n"); 
        
        
MicroBitImage SS("\
        0 0 0 0 0\n\
        0 1 1 1 0\n\
        0 1 0 1 0\n\
        0 1 1 1 0\n\
        0 0 0 0 0\n");
        
MicroBitImage BB("\
        0 0 0 0 0\n\
        0 0 0 0 0\n\
        0 0 1 0 0\n\
        0 0 0 0 0\n\
        0 0 0 0 0\n");
        
        
char Lx[6]="     ", Ly[6]="     ", Lz[6]="     ";
char Rx[6]="     ", Ry[6]="     ", Rz[6]="     ";

bool run=false;

void itos(int i, char buf[]){
  if(i<0){ i*=-1; buf[0]='-';}else{buf[0]=' ';}
  if(i>9999){ buf[1]='9';buf[2]='9';buf[3]='9';buf[4]='9';
  }else{
    buf[4]=i%10+'0';i/=10; buf[3]=i%10+'0';i/=10;
    buf[2]=i%10+'0';i/=10; buf[1]=i%10+'0';i/=10;
  }
}

void onData(MicroBitEvent)
{
  PacketBuffer p = uBit.radio.datagram.recv();

  if(p[0]==ID*4+1){
    int i;
    i=(int)(p[1]|(p[2]<<8)|(p[3]<<16)|(p[4]<<24));
    itos(i, Lx);
    i=(int)(p[5]|(p[6]<<8)|(p[7]<<16)|(p[8]<<24));
    itos(i, Ly);
    i=(int)(p[9]|(p[10]<<8)|(p[11]<<16)|(p[12]<<24));
    itos(i, Lz);
  }

  if(p[0]==ID*4+2){
    int i;
    i=(int)(p[1]|(p[2]<<8)|(p[3]<<16)|(p[4]<<24));
    itos(i, Rx);
    i=(int)(p[5]|(p[6]<<8)|(p[7]<<16)|(p[8]<<24));
    itos(i, Ry);
    i=(int)(p[9]|(p[10]<<8)|(p[11]<<16)|(p[12]<<24));
    itos(i, Rz);
  }

}

TCHAR *Path = "0:/";
FIL fil;
FATFS fatfs;
UINT NumBytes;

void onButtonA(MicroBitEvent)
{
  DIR dir;
  FILINFO fno;
  FRESULT res;
  char buf[16];
  char *fn;

  f_mount(&fatfs, Path, 0);
  res = f_opendir(&dir, "/");
  if(res != FR_OK){uBit.serial.printf("open dir error\r\n");return;}

  int i = 0;
  while(1){
    res = f_readdir(&dir, &fno);
    if(res != FR_OK){uBit.serial.printf("read dir error\r\n");return;}
    if(fno.fname[0] == 0) break;
    if(fno.fname[0] == '.') continue;   // ドットエントリを無視
    fn = fno.fname;
    if(!(fno.fattrib & AM_DIR)){        // ディレクトリを無視
      if(fn[0]!='M') continue;
      if(fn[1]!='B') continue;
      if(fn[2]<'0' || fn[2]>'9') continue;
      if(fn[3]<'0' || fn[3]>'9') continue;
      if(fn[4]<'0' || fn[4]>'9') continue;
      if(fn[5]<'0' || fn[5]>'9') continue;
      if(fn[6]!='.') continue;
      if(fn[7]!='C') continue;
      if(fn[8]!='S') continue;
      if(fn[9]!='V') continue;
      if(fn[10]!=0)  continue;
      int j=(fn[5]-'0')+(fn[4]-'0')*10+(fn[3]-'0')*100+(fn[2]-'0')*1000;
      if(j>=i){i=j+1;}
    }
  }

  buf[0]='M';
  buf[1]='B';
  buf[5]=i%10+'0'; i/=10;
  buf[4]=i%10+'0'; i/=10;
  buf[3]=i%10+'0'; i/=10;
  buf[2]=i%10+'0';
  buf[6]='.';
  buf[7]='C';
  buf[8]='S';
  buf[9]='V';
  buf[10]=0;

  f_open(&fil, buf, FA_WRITE|FA_OPEN_ALWAYS);
  f_write(&fil, "LX,LY,LZ,RX,RY,RZ\n", 18, &NumBytes);

  uBit.serial.printf("create file : %s\r\n",buf);

  run=true;
}

void onButtonAB(MicroBitEvent)
{
  f_close(&fil);

  run=false;
}

int main()
{
  uBit.init();
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_CLICK, onButtonAB);
  uBit.radio.enable();

  miso.getDigitalValue(PullUp);

  PacketBuffer b(1);
  b[0] = ID*4;
  while(1){
    if(run){
      uBit.radio.datagram.send(b);
      uBit.display.print(BS);
      uBit.sleep(500);
      uBit.display.print(SS);
      uBit.sleep(500);
      uBit.serial.printf("LX=%s LY=%s LZ=%s\tRX=%s RY=%s RZ=%s\r\n",Lx,Ly,Lz,Rx,Ry,Rz);
      f_write(&fil, Lx,  5, &NumBytes);
      f_write(&fil, ",", 1, &NumBytes);
      f_write(&fil, Ly,  5, &NumBytes);
      f_write(&fil, ",", 1, &NumBytes);
      f_write(&fil, Lz,  5, &NumBytes);
      f_write(&fil, ",", 1, &NumBytes);
      f_write(&fil, Rx,  5, &NumBytes);
      f_write(&fil, ",", 1, &NumBytes);
      f_write(&fil, Ry,  5, &NumBytes);
      f_write(&fil, ",", 1, &NumBytes);
      f_write(&fil, Rz,  5, &NumBytes);
      f_write(&fil, "\n",1, &NumBytes);
    }else{
      uBit.display.print(BB);
      uBit.sleep(100);
    }
  }
}
