#include <Arduino.h>
#include <CAN.h>
//#include 
unsigned char Can_Rcv_Flags;
unsigned char Rx_Data_Len;
char Msg_Rcvd;
const long ID_1st = 12111, ID_2nd = 3;
long Rx_ID;
char RX_Data[8];

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received ");

  if (CAN.packetExtended()) {
    Serial.print("extended ");
  }

  if (CAN.packetRtr()) {
    // Remote transmission request, packet contains no data
    Serial.print("RTR ");
  }

  Serial.print("packet with id 0x");
  Serial.print(CAN.packetId(), HEX);

  if (CAN.packetRtr()) {
    Serial.print(" and requested length ");
    Serial.println(CAN.packetDlc());
  } else {
    Serial.print(" and length ");
    Serial.println(packetSize);
    Serial.print((char)CAN.read());
    // only print packet data for non-RTR packets
    while (CAN.available()) {
      Serial.print((char)CAN.read());
    }
    Serial.println();
  }

  Serial.println();
}
void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Receiver Callback");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(125E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

  // register the receive callback
  CAN.onReceive(onReceive);
}

void loop() {
}


