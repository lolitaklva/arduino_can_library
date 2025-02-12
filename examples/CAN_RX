#include <IncFile1.h>

CAN can_rx;

#define TX 4
#define RX 5

void setup() {
  Serial.begin(9600);

  can_rx.init(can_rx, RX, TX, 100);
}

void loop() {
  can_rx.readPacket();

  if(can_rx.Complete_RX()) {
    uint8_t r_data = can_rx.get_data();
    uint8_t r_crc = can_rx.get_crc_receive();

    Serial.print("Received data is:");
    Serial.println(r_data);
    Serial.print("CRC on the received side is");
    Serial.println(r_crc);
  }
  
}
