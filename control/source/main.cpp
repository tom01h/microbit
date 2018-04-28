#include "MicroBit.h"
#include "defines.h"

MicroBit uBit;

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
    i=(int)(p[1]|(p[2]<<8)|(p[3]<<16) |(p[4]<<24));
    itos(i, Lx);
    i=(int)(p[5]|(p[6]<<8)|(p[7]<<16) |(p[8]<<24));
    itos(i, Ly);
    i=(int)(p[9]|(p[10]<<8)|(p[11]<<16) |(p[12]<<24));
    itos(i, Lz);
  }

  if(p[0]==ID*4+2){
    int i;
    i=(int)(p[1]|(p[2]<<8)|(p[3]<<16) |(p[4]<<24));
    itos(i, Rx);
    i=(int)(p[5]|(p[6]<<8)|(p[7]<<16) |(p[8]<<24));
    itos(i, Ry);
    i=(int)(p[9]|(p[10]<<8)|(p[11]<<16) |(p[12]<<24));
    itos(i, Rz);
  }

}

void onButton(MicroBitEvent e)
{
  if(e.source == MICROBIT_ID_BUTTON_A){
    run=true;
  }
  if(e.source == MICROBIT_ID_BUTTON_AB){
    run=false;
  }
}

int main()
{
  uBit.init();
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_EVT_ANY, onButton);
  uBit.radio.enable();

  PacketBuffer b(1);
  b[0] = ID*4;
  while(1){
    if(run){
      uBit.radio.datagram.send(b);
      uBit.display.print(BS);
      uBit.sleep(500);
      uBit.serial.printf("LX=%s LY=%s LZ=%s\tRX=%s RY=%s RZ=%s\r\n",Lx,Ly,Lz,Rx,Ry,Rz);
      uBit.display.print(SS);
      uBit.sleep(500);
    }else{
      uBit.display.print(BB);
      uBit.sleep(100);
    }
  }
}
