Write CAN
To send or read CAN message create CAN instance. Then include the created instance as the first argument using init() function. Also provide TX and RX pin number and baudrate as second, third and fourth arguments respectively.
Example of initialization:
```cpp
CAN can_tx;
#define RX 4
#define TX 5
void setup() {
  can_tx.init(can_tx, RX, TX, 100);
}
```