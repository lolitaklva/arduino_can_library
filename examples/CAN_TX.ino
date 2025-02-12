#include <IncFile1.h>

CAN can_tx;

#define RX 4
#define TX 5

uint16_t id = 0b00001000110;
uint8_t data = 0b00110101;

uint16_t id2 = 0b01101000110;
uint8_t data2 = 0b00100100;

bool rtr = false;

void setup() {
  can_tx.init(can_tx, RX, TX, 100);
}

void loop() {
  if(can_tx.Complete_TX()) {
    can_tx.sendPacket(id, rtr, data);
  }

  if(can_tx.Complete_TX()) {
    can_tx.sendPacket(id2, rtr, data2);
  }
}