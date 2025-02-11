#ifndef CAN_LIB
#define CAN_LIB
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h> 

class CAN {
	public:
	static CAN* instance;  // object pointer
	
	void init(CAN& can_obj, uint8_t RX, uint8_t TX, uint32_t baud_rate);
	uint8_t sendPacket(uint16_t id, _Bool rtr, uint8_t data);
	// uint8_t beginExtendedPacket(long id, int dlc = -1, _Bool rtr = false);
	// void endPacket(); 
	// uint8_t requestPacket(uint8_t id, _Bool rtr = true);
	uint8_t readPacket();
	
	uint16_t get_id();
	_Bool get_rtr();
	_Bool get_ide();
	uint8_t get_dlc();
	uint8_t get_data();
	uint16_t get_crc_transmit();
	uint16_t get_crc_receive();
	_Bool get_ack();
	
	_Bool Complete_RX();
	
	void TIMER_A_vect();
	void TIMER_B_vect();
	void PCINT_vect();
	
	private:
	
	_Bool ISR_dir;

	uint8_t TX_pin;
	uint8_t RX_pin;
	uint8_t pin_def_rx;
	uint8_t pin_def_tx;
	
	volatile uint8_t* _PORT;
	volatile uint8_t* _PIN;
	
	// transmit frame
	uint64_t can_frame;
	uint16_t _identifier;
	_Bool _rtr;
	_Bool _ide;
	uint8_t _dlc;
	uint8_t _data;
	uint16_t _crc;
	_Bool _crc_del;
	_Bool _ack;
	_Bool _ack_del;
	uint8_t _eof;
	uint8_t _ifs;
	
	// received frame
	uint16_t _identifier_r;
	_Bool _rtr_r;
	_Bool _ide_r;
	uint8_t _dlc_r;
	uint8_t _data_r;
	uint16_t _crc_t;
	uint16_t _crc_r;
	_Bool _crc_del_r;
	_Bool _ack_r;
	_Bool _ack_del_r;
	uint8_t _eof_r;
	
	void ISR_Receive();
	void ISR_Transmit();
	
	//Error checking
	uint16_t CRC(uint16_t data); 
	_Bool crc_error_flag = false; 
	_Bool ack_error_flag = false; 
	_Bool bit_stuffing_error_flag = false; 
	_Bool form_error_flag = false;
	_Bool error_frame_flag = false;
	_Bool bit_error_flag = false;
	_Bool global_error_flag = false;
	_Bool tx_buss_off_flag = false;
	_Bool rx_buss_off_flag = false;
	
	// ACK check
	_Bool ack_check = false;
	_Bool ack_check_bit = 1;
	
	// RX Complete
	_Bool rx_complete = false;
	
	uint16_t tx_error_counter = 0;
	uint16_t rx_error_counter = 0;
	
	void error_frame();
		
}; 

void UART_transmit(uint8_t data);
void UART_send_string(const char* str);

#endif
