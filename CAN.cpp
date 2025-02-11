/*
 * CAN_lib.cpp
 *
 * Created: 21.11.2024 20:08:06
 */
  
 #include "IncFile1.h"

 CAN* CAN::instance = NULL;
 
 void CAN::init(CAN& can_obj, uint8_t RX, uint8_t TX, uint32_t baud_rate) {
		 
	 // UART for debugging purposes 
	 UBRR0H = 0;
	 UBRR0L = 103;
		 
	 UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
		 
	 UCSR0A &= ~((1 << FE0) | (1 << DOR0) | (1 << UPE0)); // Frame error, Data overrun and Parity error
	 UCSR0B &= ~(1 << RXEN0);
	 
	 instance = &can_obj;
	 TX_pin = TX;
	 RX_pin = RX;
	 
	 // define the right port
	 //int PORTB[] = {13, 12, 11, 10, 9, 8};
	 //int PORTC[] = {A0, A1, A2, A3, A4, A5};
	 //int PORTC[] = {14, 15, 16, 17, 18, 19};
	 //int PORTD[] = {7, 6, 5, 4, 3, 2, 1, 0};
	 
	 if (RX_pin > 19 || TX_pin > 19) {
		 return;
	 }
		 
	 if (RX_pin >= 8 && RX_pin <= 13) {
		 DDRB &= ~(1 << (RX_pin - 8)); // 0 - input
		 _PIN = &PINB;
		 pin_def_rx = (RX_pin - 8);
		 PCICR |= 1 << PCIE0; // Pin Change Interrupt Enable
		 PCMSK0 |= 1 << (RX_pin - 8); //  Pin Change Enable Mask
	 }
	 else if (RX_pin >= 14 && RX_pin <= 19) {
		 DDRC &= ~(1 << (RX_pin - 14)); // 0 - input
		 _PIN = &PINC;
		 pin_def_rx = (RX_pin - 14);
		 PCICR |= 1 << PCIE1; // Pin Change Interrupt Enable
		 PCMSK1 |= 1 << (RX_pin - 14); //  Pin Change Enable Mask
	 }
	 else if (RX_pin >= 0 && RX_pin <= 7) {
		 DDRD &= ~(1 << RX_pin); // 0 - input
		 _PIN = &PIND;
		 pin_def_rx = RX_pin;
		 PCICR |= 1 << PCIE2; // Pin Change Interrupt Enable
		 PCMSK2 |= 1 << (RX_pin); //  Pin Change Enable Mask
	 }
	 if (TX_pin >= 8 && TX_pin <= 13) {
		 DDRB |= 1 << (TX_pin - 8); // 1 - output
		 PORTB |= 1 << (TX_pin - 8);  // recessive state
		 _PORT = &PORTB;
		 pin_def_tx = (TX_pin - 8);
	 }
	 else if (TX_pin >= 14 && TX_pin <= 19) {
		 DDRC |= 1 << (TX_pin - 14); // 1 - output
		 PORTC |= 1 << (TX_pin - 14);  // recessive state
		 _PORT = &PORTC;
		 pin_def_tx = (TX_pin - 14);
	 }
	 else if (TX_pin >= 0 && TX_pin <= 7) {
		 DDRD |= 1 << TX_pin; // 1 - output
		 PORTD |= 1 << TX_pin;  // recessive state
		 _PORT = &PORTD;
		 pin_def_tx = TX_pin;
	 }
	 
	 // init timer 
	 TCCR1B |= (1 << WGM12);  // CTC mode timer1 with OCR1A used as TOP value
	 TCCR1B |= (1 << CS11) | (1 << CS10);   // 64 prescaler divider 
	 
	 if (baud_rate == 0) {
		 return;
	 }
	 
	 // set OCR1A value
	 uint16_t ocr_value = 16000000/(64 * baud_rate)-1;
	 OCR1A = ocr_value;
	 
	 //	set OCR1B value
	 OCR1B = (float)(OCR1A) * 0.75;  // 75 % for bit sampling
	 
	 TIFR1 |= (1 << OCF1A); // flag reset 
	 
	 sei(); // enable global interrupt
	 
 }
 
 void UART_transmit(uint8_t data) {
	 UCSR0B |= (1 << TXEN0);
	 int8_t iter = 0;
	 if(data > 99) iter = 2;
	 else if(data > 9) iter = 1;
	 
	 for(; iter >=0; iter--) {
		 while (!(UCSR0A & (1 << UDRE0))); 
		 if(iter == 2) {
			 UDR0 = data/100 + 48;
			 data %= 100;
		 }
		 else if(iter == 1) {
			 UDR0 = data/10 + 48;
			 data %= 10;
		 }
		 else UDR0 = data + 48;
	 }           
	 UCSR0B &= ~(1 << TXEN0);           
 }
 
 void UART_send_string(const char* str) {
	 UCSR0B |= (1 << TXEN0);
	 
	 while (*str) {
		 while (!(UCSR0A & (1 << UDRE0))); 
		 UDR0 = *str++; 
	 }
	 
	 while (!(UCSR0A & (1 << TXC0))); 
	 UCSR0B &= ~(1 << TXEN0); //
 }
 
 uint8_t CAN::sendPacket(uint16_t id, _Bool rtr, uint8_t data) {
	 _identifier = id; // identifier
	 _rtr = rtr; // remote transmission request
	 _ide = 0; // identifier extension
	 _dlc = 1; // data length code
	 _data = data; // data
	 _crc = CRC(data); // cyclic redundancy check
	 _crc_del = 1; // crc delimiter 
	 _ack = 1; // acknowledgment bit
	 _ack_del = 1; // acknowledgment delimiter
	 _eof = 0b01111111; // end of frame 
	 _ifs = 0b01111111; // interframe space
	 
	 can_frame = ((uint64_t)_identifier << 0);
	 can_frame |= ((uint64_t)_rtr << 11);
	 can_frame |= ((uint64_t)_ide << 12);
	 can_frame |= ((uint64_t)_dlc << 13);
	 can_frame |= ((uint64_t)_data << 17);
	 can_frame |= ((uint64_t)_crc << 25);
	 can_frame |= ((uint64_t)_crc_del << 40);
	 can_frame |= ((uint64_t)_ack << 41);
	 can_frame |= ((uint64_t)_ack_del << 42);
	 can_frame |= ((uint64_t)_eof << 43);
	 can_frame |= ((uint64_t)_ifs << 50);
 
	 TIMSK1 |= 1 << OCIE1A;  // enable interrupt on compare match
	 TIMSK1 &= ~(1 << OCIE1B);  // disable receiver interrupt 
	 ISR_dir = true; // transmitting
	 
	 return 0;
 }
 
 uint8_t CAN::readPacket() {
	 TIMSK1 &= ~(1 << OCIE1A);  // disable transmitter interrupt 
	 TIMSK1 |= 1 << OCIE1B;  // enable interrupt on compare match
	 ISR_dir = false; // receiving 
	 return 0;
 }
 
 // Interrupts vectors
 ISR(TIMER1_COMPA_vect)  { CAN::instance->TIMER_A_vect(); }
 ISR(TIMER1_COMPB_vect)  { CAN::instance->TIMER_B_vect(); }
 ISR(PCINT0_vect)        { CAN::instance->PCINT_vect(); }
 ISR(PCINT1_vect)        { CAN::instance->PCINT_vect(); }
 ISR(PCINT2_vect)        { CAN::instance->PCINT_vect(); }
 
 void CAN::TIMER_A_vect() {
	 if (!global_error_flag) {
		 if (ISR_dir && !tx_buss_off_flag) {
			 ISR_Transmit();
		 }
	 }
	 error_frame();
 }
 
 void CAN::TIMER_B_vect() {
	 if (!ack_check) {
		 if (!global_error_flag) {
			 if (!ISR_dir && !rx_buss_off_flag) {
				 ISR_Receive();	
			 }
		 }
		 error_frame();
	 }
	 else {
		 ack_check_bit = (*_PIN >> pin_def_rx) & 1;
		 UART_transmit(ack_check_bit);
		 ack_check = false;
		 TIMSK1 &= ~(1 << OCIE1B); 
	 }
	 
 }
 
 void CAN::PCINT_vect() {
	 if(PCMSK0 & (1 << pin_def_rx)) {
		 TCNT1 = 0; // synchronize receiver clock with transmitter clock 		
	 }
	 if(PCMSK1 & (1 << pin_def_rx)) {
		 TCNT1 = 0; // synchronize receiver clock with transmitter clock
	 }
	 if(PCMSK2 & (1 << pin_def_rx)) {
		 TCNT1 = 0; // synchronize receiver clock with transmitter clock
	 }
 }
 
 void CAN::ISR_Transmit() { 
	 static uint8_t shift = 0;
	 static uint16_t count_recessive = 0;
	 static _Bool transmitting = false;
	 static uint8_t stuff_counter = 0;
	 static uint8_t error_counter = 0;
	 uint8_t stuff_bit;
	 _Bool stuff_flag = false;
	 uint8_t tx_bit;
	 static uint8_t rx_bit;
 
	 // Check for recessive state
	 if (!transmitting) {
		 if (*_PIN & (1 << pin_def_rx)) {  // RX_pin recessive
			 count_recessive++;
		 }
		 else count_recessive = 0;
		 
		 // Check for 14 sequential recessive bits
		 if (count_recessive >= 14) {
			 transmitting = true;  // begin transmission
			 stuff_counter = 0;   
			 error_counter = 0;  
			 shift = 0;
			 *_PORT &= ~(1 << pin_def_tx); // SOF
		 }
	 }
	 else {			
		 // frame transmitting
		 if (shift < 57) {  // 57-bit frame
			 tx_bit = (can_frame >> shift) & 1;
			 
			 if ((tx_bit == ((can_frame >> (shift - 1)) & 1)) && shift < 43) { // check for two bits of the same logic level excluding eof and ifs
				 stuff_counter ++;
			 }
			 else stuff_counter = 0;
			 
			 if (stuff_counter == 5) { // check for five bits of the same logic level
				 stuff_bit = tx_bit ? 0 : 1;  // Opposite logic level
				 stuff_flag = true;
			 }
			 if (stuff_flag) { // stuff the bit of the opposite logic level
				 if (stuff_bit) {
					 *_PORT |= (1 << pin_def_tx); 
					 } else {
					 *_PORT &= ~(1 << pin_def_tx);
				 }
				 rx_bit = (*_PIN >> pin_def_rx) & 1;
				 stuff_flag = false;
				 stuff_counter = 0;
				 return;
			 }
			 
			 if (rx_bit == 0) {  // Dominant bit detected
				 error_counter++;
			 }
			 else error_counter = 0;  // Reset if recessive bit detected
			 
			 if (error_counter >= 6) {  // Error frame detected (6 dominant bits)
				 transmitting = false;
				 CAN::bit_stuffing_error_flag = true;  // Set error flag
				 count_recessive = 0;
				 return;
			 } 
			 
			 if (shift < 11) { // 11 bit identifier
				 //Arbitrage
				 if (tx_bit) {
					 *_PORT |= (1 << pin_def_tx);  // first id bit
					 } else {
					 *_PORT &= ~(1 << pin_def_tx); // first id bit
				 }
				 rx_bit = (*_PIN >> pin_def_rx) & 1;
				 if (tx_bit != rx_bit) {
					 transmitting = false; // Arbitrage lost
					 count_recessive = 0;
						 
				 }
			 }
			 else { // transmitting the rest of the frame
				 if (tx_bit) {
					 *_PORT |= (1 << pin_def_tx);  // Recessive state
				 }
				 else {
					 *_PORT &= ~(1 << pin_def_tx); // Dominant state
				 }
				 rx_bit = (*_PIN >> pin_def_rx) & 1;
				 if (tx_bit != rx_bit && shift != 41) { // bit error check
					 transmitting = false;
					 bit_error_flag = true; // raise bit error
					 count_recessive = 0;
					 return;
				 }
				 if (shift == 41) {
					 ack_check = true;
					 TIMSK1 |= 1 << OCIE1B; 
				 }
				 if (ack_check_bit == 1 && shift == 42) { // check for the ACK bit on the next interrupt
					 transmitting = false;
					 ack_error_flag = true; // raise ACK error
					 count_recessive = 0;
					 return;
				 }
			 }
			 shift++;				
		 }
		 else {
			 if (tx_error_counter >= 8) tx_error_counter -= 8;
			 transmitting = false;  // end transmission
			 count_recessive = 0;
			 readPacket(); // if not transmitting, then receiving
		 }
	 }	
 }
 
 uint16_t CAN::CRC(uint16_t data) {
	 const uint16_t poly = 0x4599; // CRC-15-CAN polynomial
	 uint16_t crc = 0x0000;        // Init CRC value (0)
 
	 // Proceed data (16 bit)
	 for (uint8_t i = 0; i < 16; i++) {
		 bool bit = (data >> (15 - i)) & 1; // Current data bit
		 bool crc_msb = (crc >> 14) & 1;    // Most significant CRC bit
		 crc <<= 1;                        // CRC left shifting
 
		 if (bit ^ crc_msb) {              // If XOR data bit, CRC msb == 1
			 crc ^= poly;                  // add polynomial to CRC
		 }
	 }
 
	 crc &= 0x7FFF; // Masking to get 15 bit result
	 
	 return crc;
 }
 
 void CAN::ISR_Receive() {
	 static _Bool receiving = false;
	 static uint64_t can_frame_r = 0; 
	 static uint8_t shift = 0;
	 static uint8_t count_dominant = 0; 
	 static uint8_t stuff_counter = 0;
	 uint8_t stuff_bit = 0;
	 _Bool stuff_flag = false;
	 //_Bool start_bit = false;
	 
	 //check for the bus dominant state
	 if (!(*_PIN & (1 << pin_def_rx))) {
		 count_dominant++;
		 if (!receiving) {
			 shift = 0;
			 // start_bit = true;
		 }
		 receiving = true;
		 
	 }
	 else count_dominant = 0;
	 
	 if (count_dominant == 6) {
		 error_frame_flag = true; // error frame detected
		 receiving = false;
		 return;
	 }
	 
	 //read a frame
	 if (receiving) {
		 rx_complete = false;
		 if (((((*_PIN) >> pin_def_rx) & 1) == ((can_frame_r >> (shift - 1)) & 1)) && (shift != 0) && (shift < 43)) {// check for two bits of the same logic level
			 stuff_counter ++;
		 }
		 else stuff_counter = 0;
		 
		 if ( stuff_counter > 5) { // 6 bits of the same logic level
			 bit_stuffing_error_flag = true; // raise bit stuffing error
			 receiving = false;
			 return;
		 }
		 
		 if (stuff_counter == 4) { // check for five bits of the same logic level
			 stuff_flag = true;
		 }
		 
		 if (stuff_flag) { // unstuff the bit of the opposite logic level
			 stuff_bit ++;
			 stuff_flag = false;
		 }
		 else {
			 if (shift < 50) {
				 *_PORT |= (1 << pin_def_tx);
				 if (*_PIN & (1 << pin_def_rx)) {
					 can_frame_r |= (1 << shift);  // Recessive state
				 }
				 else {
					 can_frame_r &= ~(1 << shift); // Dominant state
				 }
				 // ACK check
				 if (shift == 41 && (*_PIN & (1 << pin_def_rx))) { 
					 *_PORT &= ~(1 << pin_def_tx);
				 }
			 }
			 else {
				 receiving = false;
				 // parse received frame
				 _identifier_r = (uint16_t)(can_frame_r & 0x7FF);
				 _rtr_r = (_Bool)((can_frame_r >> 11) & 1);
				 _ide_r = (_Bool) ((can_frame_r >> 12) & 1);
				 _dlc_r = (uint8_t)((can_frame_r >> 13) & 0x0F);
				 _data_r = (uint8_t)((can_frame_r >> 17) & 0xFF);
				 _crc_t = (uint16_t)((can_frame_r >> 25) & 0x7FFF);
				 _crc_del_r = (_Bool) ((can_frame_r >> 40) & 1);
				 _ack_r = (_Bool) ((can_frame_r >> 41) & 1);
				 _ack_del_r = (_Bool) ((can_frame_r >> 42) & 1);
				 _eof_r = (uint8_t)((can_frame_r >> 43) & 0x7F);
				 can_frame_r = 0;
				 // CRC check
				 _crc_r = CRC(_data_r);
				 
				 if (_crc_r != _crc_t) {
					 crc_error_flag = true; // raise CRC error
					 return;
				 }
				 
				 // form check
				 if (!((_eof_r & 0b01111111) == 0b01111111) || !(_ack_del_r) || !(_crc_del_r)) {
					 form_error_flag = true; // raise form error
					 return;
				 }
				 
				 rx_complete = true;
				 if (rx_error_counter >= 1) rx_error_counter -= 1;
			 }
			 
			 shift ++;
		 }
		 
	 }	
 }
 
 uint16_t CAN::get_id() {
	 return _identifier_r;
 }
 
 _Bool CAN::get_rtr() {
	 return _rtr_r;
 }
 
 _Bool CAN::get_ide() {
	 return _ide_r;
 }
 
 uint8_t CAN::get_dlc() {
	 return _dlc_r;
 }
 
 uint8_t CAN::get_data() {
	 return _data_r;
 }
 
 uint16_t CAN::get_crc_transmit() {
	 return _crc_t;
 }
 
 uint16_t CAN::get_crc_receive() {
	 return _crc_r;
 }
 
 _Bool CAN::get_ack() {
	 return _ack_r;
 }
 
 _Bool CAN::Complete_RX() {
	 return rx_complete;
 }
 
 void CAN::error_frame() {
	 static uint8_t i = 0;
	 static _Bool stat_error = 0;
	 uint8_t shift_error = 8;
	 static _Bool counter_increment = true;
	 
	 // check for any possible error flag
	 if (crc_error_flag || ack_error_flag || bit_stuffing_error_flag || form_error_flag || bit_error_flag || error_frame_flag) {
		 
		 global_error_flag = true; // global flag on
		 if (counter_increment) {
			 if (ISR_dir) { // transmitter error handling logic
				 if (!ack_error_flag) {
					 tx_error_counter += 8;
				 }
				 if (tx_error_counter > 127) { // transmitter becomes error-passive
					 stat_error = 1;
					 shift_error = 6;
				 }
				 if (tx_error_counter > 255) {
					 tx_buss_off_flag = true; // get transmitter off the bus
				 }
			 }
			 else { // receiver error handling logic
				 rx_error_counter += 1;
			 
				 if (rx_error_counter > 127) { // receiver becomes error-passive
					 stat_error = 1;
					 shift_error = 6;
				 }
				 if (rx_error_counter > 255) {
					 rx_buss_off_flag = true; // get receiver off the bus
				 }
			 }
			 counter_increment = false;
		 }
		 
		 if (stat_error == 0){ // for error-active nodes
			 if (i < 5) {
				 *_PORT &= ~(1 << pin_def_tx); // 6 dominant bits
				 i ++;
			 } else {
				 i = 0;
				 stat_error = 1;
			 }
		 } else {
			 if (i < shift_error - 1) { // for error-passive nodes
				 *_PORT |= (1 << pin_def_tx); // 8 or 6 recessive bits 
				 i ++;
			 } else {
				 i = 0;
				 // reset all the flags
				 stat_error = 0;
				 crc_error_flag = false;
				 ack_error_flag = false;
				 bit_stuffing_error_flag = false;
				 form_error_flag = false;
				 bit_error_flag = false;
				 error_frame_flag = false;
				 global_error_flag = false;
				 counter_increment = true;
			 }
		 }
	 }
 }
 