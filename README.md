# Library usage
## Init CAN  
---
To send or read a CAN message, create a CAN instance. Then, include the created instance as the first argument in the init() function. Additionally, provide the TX and RX pin numbers and the baud rate as the second, third, and fourth arguments, respectively.
### Example of Initialization
```cpp
CAN can_tx;
#define RX 4
#define TX 5
void setup() {
  can_tx.init(can_tx, RX, TX, 100);
}
```
## Write CAN  
---
To write a CAN message, use sendPacket() function. Provide the message id, set rtr to false and pass the data as arguments.
### Example
```cpp
can_tx.sendPacket(0b00001000110, false, 0b00110101);
```