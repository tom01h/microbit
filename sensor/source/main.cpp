#include "MicroBit.h"
#include "defines.h"

MicroBit uBit;

void onData(MicroBitEvent)
{
  PacketBuffer p = uBit.radio.datagram.recv();
  PacketBuffer b(13);

  if(p[0]==ID*4){
    int v;
    b[0] = ID*4+NO;
    v=uBit.accelerometer.getX();
    b[1] = v&0x000000ff;
    b[2] = (v>>8)&0x0000ff;
    b[3] = (v>>16)&0x0000ff;
    b[4] = (v>>24)&0x0000ff;
    v=uBit.accelerometer.getY();
    b[5] = v&0x000000ff;
    b[6] = (v>>8)&0x0000ff;
    b[7] = (v>>16)&0x0000ff;
    b[8] = (v>>24)&0x0000ff;
    v=uBit.accelerometer.getZ();
    b[9] = v&0x000000ff;
    b[10] = (v>>8)&0x0000ff;
    b[11]= (v>>16)&0x0000ff;
    b[12]= (v>>24)&0x0000ff;

    uBit.display.clear();
    uBit.sleep(10*NO);
    uBit.radio.datagram.send(b);
#if NO==1
    uBit.display.printAsync('L');
#elif NO==2
    uBit.display.printAsync('R');
#endif
  }
}


int main()
{
  uBit.init();
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.enable();

#if NO==1
  uBit.display.printAsync('L');
#elif NO==2
  uBit.display.printAsync('R');
#endif

  while(1)
    uBit.sleep(1000);
}
