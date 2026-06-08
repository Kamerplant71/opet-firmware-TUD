/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Martin Bliss
	Version:	1.03
	Date:		03.11.2023
	-----------------------------------------
	ATMega 324 Micro controller support 
	-----------------------------------------
	MAIN Function C-Code FILE
	=========================================
*/

//===========================================================================================
// INCLUDE Header

#include "MPPT_PCB_MCU__Main.h"
#include "MPPT_PCB_MCU__Com.h"
#include "MPPT_PCB_MCU__IO.h"
#include "MPPT_PCB_MCU__EROM.h"
#include "MPPT_PCB_MCU__LOAD_CTR.h"
#include "MPPT_PCB_MCU__Range.h"
#include "MPPT_PCB_MCU__PI_CTR.h"
#include "MPPT_PCB_MCU__IV_Trans.h"
#include "MPPT_PCB_MCU__TEMP.h"

//===========================================================================================
// GLOBAL VARIABLES and STRUCTURES
volatile uint8_t SysStatus_A;
volatile uint8_t SysStatus_B;
volatile uint8_t SysControl;
volatile uint8_t Timer_CTR;
volatile uint8_t UartStatus;
volatile uint8_t PV_Status;
volatile uint8_t SysConfig;
volatile uint8_t Temp_Sensor_Type;
volatile uint8_t EEMEM_Written;
volatile uint8_t TimerMeas_Count;
volatile uint8_t Timer_Control_Counter;
volatile uint16_t Timer_Temp_Meas_Counter;
volatile uint8_t Timer_Control_Match;
volatile uint8_t Timer_Temp_Meas_Match;
volatile float TEMP_MAX_Heat_NTC;  
volatile uint8_t TEMP_Heat_dissipation_devices;

//Convert 16to8 bit integer
typedef union {
	uint16_t i16;
	struct {
		uint8_t i8l;
		uint8_t i8h;
	};
} convert16to8;

//===========================================================================================
// EEPROM VARIABLES
EEMEM uint16_t EROM_Loop_Timer_1_Comp_Match = TIMER_1_COMP_MATCH;
EEMEM uint8_t EROM_Timer_Control_Match = CONTROL_TIMER_MULT;
EEMEM uint8_t EROM_Timer_Temp_Meas_Match = TEMP_MEAS_TIMER_MUILT;
EEMEM uint8_t EROM_SysConfig =
    (PCBconfig_TEMP_is_enabled ? BIT(0) : 0) |
    (PCBconfig_TEMP_Heat_Monitoring_enabled ? BIT(2) : 0); // see board config definitions section in Main.h
EEMEM uint8_t EROM_SysControl = 0; // autostart EROM system control
EEMEM uint8_t EROM_Temp_Sensor_Type = PCBconfig_TEMP_Sensor_Type; // see board config definitions section in Main.h
EEMEM float EROM_TEMP_MAX_Heat_NTC = 70.0; // X degree or higher of the heat dissipation device and output should turn off 
EEMEM uint8_t EROM_TEMP_Heat_dissipation_devices = PCBconfig_TEMP_Heat_Monitoring_devices; // Number of active dissipation devices


//===========================================================================================
// MAIN Function
int main(void)
{
	// disable the watchdog at start-up
	cli();								// Disable Interrupts
	wdt_reset();						// reset watchdog
	MCUSR &= ~(1<<WDRF);				// clear watchdog status flag
	wdt_disable();						// disable watchdog
	//MCUSR &= ~(1<<WDRF);				// make sure WDRF is cleared in MCU status reg or else watchdog cannot be turned off
	//WDTCSR |= (1<<WDCE) | (1<<WDE);	// send special value to command register enabling it to be edited
	//WDTCSR = 0x00;					// turn off watchdog by clearing WDE & WDIE plus all the rest
		
	// Reset Defaults
	//OSCCAL=0x97; // calibrate internal oscillator (when running on internal 8MHz clock source)
	TimerMeas_Count 	= 0;
	Timer_Control_Counter	= 0;
	SysStatus_A 	= 0;
	SysControl	= 0;
	SysConfig	= 0;
	Timer_CTR	= 0;
	UartStatus  = 0;
	PV_Status	= 0;
	PV_Mode_CTR	= 0;
	PV_Mode_ACT = 0;
	PV_Volt_Setpoint = 0;
	PV_Volt_DAQ_New  = 0;
	PV_Curr_Setpoint  = 0;
	DIOE_PORT_A_STATE = 0;
	DIOE_PORT_B_STATE = 0;
	
	//Wait until full power up (just in case not there yet)
	_delay_ms(250);
	
	// Load EEPROM data variables
	EEPROM_LOAD_VAL_CONFIG();
	
	// Set Data direction Registers & control defaults
	Set_DDR_and_Default_State();
	
	// enable watchdog with 2 seconds timer
	wdt_enable(WDTO_2S);
	wdt_reset();
	// initialize IO Expander
	DIOExp_config_init();
	DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
		
	// Read RS485 ADDRESSf
	RS485_get_com_address_external_pins();
	
	// Initialize SPI interface (ADC & DAC conversion) - default state... (set6 for main ADC & DAC, changes for PT100 ADC)
	SPI_DDR |= (1<<MOSI_pin)|(1<<SCK_pin)|(1<<SS_pin);	//set MOSI & SCK as output
	SPCR |= (1<<SPE)|(1<<MSTR); //|(1<<CPOL0);//|(1<<CPHA0)|;	// set Master, enable, falling edge output, normal low clock
	CLRBIT(SPCR,SPR1);	// clear clock fck/64
	CLRBIT(SPCR,SPR0);	// clear clock fck/16
	CLRBIT(SPSR,SPI2X); // clear clock doubling (final speed is Clock/4)
	CLRBIT(SPI_port, SS_pin);	// disable chip select (drive high)
	
	// Initialize USART
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	UCSR0C |= (0<<USBS0)|(3<<UCSZ00);// asynchronous 8N1
	UBRR0H = (uint8_t)( UART_UBRR_CALC( UART_BAUD_RATE, F_CPU ) >> 8 );
	UBRR0L = (uint8_t)UART_UBRR_CALC( UART_BAUD_RATE, F_CPU );
	UCSR0B |= (1<<RXCIE0);	// receive complete interrupt enable
	CLRBIT(RS485_port, RS485_pin);		// RS485 com disable TxD & enable RxD
	RS485_DDR; // enable RS485 com direction output
	UART_In_Buff_L = 0;	//Input Buffer empty
	UART_Comand_L = 0;	//Command line empty
	
	// Initialize Timer 1 for MPPT loop timing
	TCCR1A |= 0b00000000; //Clear counter on compare match value
	TCCR1B |= 0b00001011; // 64 frequency divider
	OCR1B = (uint16_t) 0 ;	// compare match
	OCR1A = EEROM_LOAD_TIMER_1_CM(); //(uint16_t) TIMER_1_COMP_MATCH ;	// Timer counter reset & interupt value
	TIMSK1 |= (1<<OCIE1A);	//enable interrupt
	
	// clear current over-range state at start-up
	Reset_Over_Current_Clamp();
	
	// set & read voltage and current range & calibration
	Range_Volt_ID_Act = Volt_range_max_ID +1;	// +1 makes undefined to make sure latch relays are controlled!
	Range_Curr_ID_Act = Curr_range_max_ID +1;	// +1 makes undefined to make sure latch relays are controlled!
	SET_VOLTAGE_RANGE(Volt_range_max_ID);
	SET_CURRENT_RANGE(Curr_range_max_ID);
	Preare_Auto_Range_Selection();
	
	// initialize / reset fan control
	Reset_Fan_Control();
	
	// set PI controller defaults
	PI_CRT__Process_Controller_Settings();
	
	// initialize DAC & main ADC & PT100 ADC
	if (is_SysConfig_TEMP_On){ // reset and config Temperature sensor ADC & run internal calibration cycle
		if (Temp_Sensor_Type == Temp_Sensor__MAX31865) {
			TEMP_MAX31865_Setup();
		}
		else if (Temp_Sensor_Type == Temp_Sensor__MCP9600) {
			TEMP_MCP9600_Setup();
		}
		else if (Temp_Sensor_Type == Temp_Sensor__MAX31856){
			TEMP_MAX31856_Setup();
		}
	}

	//Setup heat dissipation temperature sensors
		if (is_SysConfig_TEMP_Heat_Monitoring_On){
			Temp_monitoring_Setup();
		}

	Set_DAC_Output_RAW(65535);	// Reset DAC to max voltage (low current VOC)
	Meas_Analog_Inputs(); // measure AIs
		
		
	// Enable Interrupts
	CLR__Timer_CTR_Main_Flag;	//Reset timer flag
	sei();
	
	// ETERNAL LOOP (till death do us apart)
 	while (1) 
	{
		// reset watchdog
		wdt_reset(); 
		
		// Check if Main meas timer start flag
		// All other flags checked in this loop to keep synchronizations of measurements
		if(is_Timer_CTR_Main_Flag_On) {
			// measure analog inputs
			Meas_Analog_Inputs();	// Measure Output
			Check_Input_Range_Selection(); // set range as needed
			//TODO: does PI_CTR selection need to be here - move to coms?
			PI_CRT__Process_Controller_Settings(); // set controller config as needed
			// Check if Control Timer Start Flag
			if(is_Timer_CTR_Control_Flag_On) {
				// process MPPT control functions
				LOAD_CTR_MAIN();			//Call main Laser Control function
				// reset control timer flag
				CLR__Timer_CTR_Control_Flag;	// Clear Timer flag at the end...
			}
			// process UART commands if receive complete - com only if not in control cycle, to keep things within timing
			else if (UART_Comand_L > 0) {
				// if not transferring IV data or measuring IV or transient, process command
				if(!is_COM_Status_IV_Data_Transfer_On  && !is_SysControl_IV_Meas && !is_SysControl_Trans_Meas) { 
					UART_Process_Input();	//Process command
					UART_Comand_L = 0;	//Delete Command
				}
			}
			// Check Temp Meas start Flag - measure only if not in control cycle, to keep things within timing
			else if(is_Timer_RTD_MEAS_Flag_On) {
				// meas PT100 when data is ready (pin is low!!!)
				if (is_SysConfig_TEMP_On){
					uint32_t Temp_uint32;
					float Temp_float;
					if (Temp_Sensor_Type == Temp_Sensor__MAX31865) {
						if (!is_EXP_DIO_3_Set) { // if Data_Ready pin is reset (data is ready to collect), read
							
							Temp_uint32 = TEMP_MAX31865_Measure();
							Temp_float = (float) Temp_uint32;
							AI_RTD_Temp =  (Temp_float * Temp_float * CAL_RTD_A2) + (Temp_float * CAL_RTD_A1) + CAL_RTD_A0;
						}
					}
					else if (Temp_Sensor_Type == Temp_Sensor__MCP9600) {
						// TODO add some timer here as the 18bit ADC needs 320ms to convert
						AI_RTD_Temp = TEMP_MCP9600_Measure();
					}
					else if (Temp_Sensor_Type == Temp_Sensor__MAX31856){
						AI_RTD_Temp = TEMP_MAX31856_Measure();
					}
				}

				// Measure temp heat dissipation device 
				if (is_SysConfig_TEMP_Heat_Monitoring_On){
					
					// Set temp flag if temperature is too high
					uint8_t temp_flag =0;
					
					for (uint8_t i = 0; i < 8; i++) {

						// If not present skip
						if (!(AI_HEAT_ADC_Present_Mask & (1 << i))) {  
							continue;
						}

						// Read temperature
						Read_Temp_ADC121C021(i);

						// If over temp is active check a lower limit to avoid rapid on/off
						if(is_Status_HEAT_NTC_Over_Temp){
							if (AI_HEAT_NTC_Temp[i] >= (TEMP_MAX_Heat_NTC -5.0) ) {
								temp_flag = 1;
							}							
						}
						else{
							if (AI_HEAT_NTC_Temp[i] >= TEMP_MAX_Heat_NTC) {
								temp_flag = 1;
							}
						}

						// If temperature is below -10.0 degrees set offline as probably something is wrong 
						if (AI_HEAT_NTC_Temp[i] <= -10.0) {
							AI_HEAT_ADC_Fault_Mask |= (1 << i);
						}
					}

					if(temp_flag){
						SET__Status_HEAT_NTC_Over_Temp;
					}					
					else{
						CLR__Status_HEAT_NTC_Over_Temp;
					}


					// Check alert pin
					if (is_ADC121C021_ALERT_Active){
						SET__Status_HEAT_NTC_Alert;
					}
					else{
						CLR__Status_HEAT_NTC_Alert;
					}

					// Check offline devices
					uint8_t number_active_devices = 0;

					// Check if number of active devices is equal to the expected amount of devices
					for (uint8_t i =0; i<8; i++){
						if (AI_HEAT_ADC_Present_Mask & (1<<i)){
						number_active_devices += 1; 
						}
					}

					// If less devices then expected or ADC fault, give error
					if(number_active_devices < TEMP_Heat_dissipation_devices || AI_HEAT_ADC_Fault_Mask != 0 ){ 
						SET__Status_HEAT_NTC_offline;
					}
					else{
						CLR__Status_HEAT_NTC_offline;	
					}


				}
				else {
					CLR__Status_HEAT_NTC_Over_Temp;
					CLR__Status_HEAT_NTC_Alert;
					CLR__Status_HEAT_NTC_offline;
					AI_HEAT_ADC_Present_Mask = 0;
					AI_HEAT_ADC_Fault_Mask = 0;
				}

				// reset RTD Cal flag
				CLR__Timer_TEMP_MEAS_Flag;	// Clear Timer flag at the end...
			}
			
			// reset measurement timer
			CLR__Timer_CTR_Main_Flag;	// Clear Timer flag at the end...
		}
		// Check if IV data transfer initiated, process data send buffer
		if (is_COM_Status_IV_Data_Transfer_On) {
			// process & handle data sending buffer
			UART_Transfer_IV();
		}		
		
		// reset watchdog
		wdt_reset();
		
	} //eternal loop return
	
	// if ya got here you did something wrong, eternity is not suppose to just end!!!
}

//===========================================================================================
// SUB Functions
//-------------------------------------------------------------------------------------------
// Data-Direction-Register set-up
void Set_DDR_and_Default_State(){
	/* Sets default digital output values and data direction registers for IO pins
	- set output values before switching to output, to prevent false state 
	*/
	// voltage control DAC
	SETBIT(CS_AO1_port, CS_AO1_pin);	// clear DAC chip select
	CS_AO1_DDR;
	// main input ADC
	SETBIT(CS_AI1_port, CS_AI1_pin);	// clear ADC chip select - must be high (clear) all times
	CS_AI1_DDR;
	CLRBIT(CS_AI_CNV_port, CS_AI_CNV_pin);  // clear ADC Conversion start
	CS_AI_CNV_DDR;
	// Temp Sensor DDR config
	if (is_SysConfig_TEMP_On){
		if (Temp_Sensor_Type == Temp_Sensor__MAX31865) {
			SET__EXP_DIO_1; // clear RT100 ADC chip select
			DDR__DO__EXP_DIO_1;
			DDR__DI__EXP_DIO_3;
		}
	    else if (Temp_Sensor_Type == Temp_Sensor__MAX31856) {
	        SET__EXP_DIO_1;      
	        DDR__DO__EXP_DIO_1;  
	        DDR__DI__EXP_DIO_3;
	    }
		else if (Temp_Sensor_Type == Temp_Sensor__MCP9600) {
			DDR__DI__EXP_DIO_1;
			DDR__DI__EXP_DIO_2;
			DDR__DI__EXP_DIO_3;
			DDR__DI__EXP_DIO_4;
		}
	}

	//Heat monitoring device active
	if (is_SysConfig_TEMP_Heat_Monitoring_On){
		DDR__DI__EXP_DIO_2; //Alert pin as input USE D2 IN HARDWARE 	
		CLR__EXP_DIO_2;		//external pull-up
	}
	
	// FAN latching relay control
	DDR__LRELAY_FAN_Off;
	DDR__LRELAY_FAN_On;
	// Address Pins
	DI_ADR_1_DDR;
	DI_ADR_2_DDR;
	DI_ADR_4_DDR;
	DI_ADR_8_DDR;
	DI_ADR_16_DDR;
	// main ADC channel multiplexer
	CLRBIT(ADC_MUX_A0_port, ADC_MUX_A0_pin);
	ADC_MUX_A0_DDR;
	CLRBIT(ADC_MUX_A1_port, ADC_MUX_A1_pin);
	ADC_MUX_A1_DDR;
	CLRBIT(ADC_MUX_A2_port, ADC_MUX_A2_pin);
	ADC_MUX_A2_DDR;
	// RS485 com
	CLRBIT(RS485_port, RS485_pin);		// RS485 com disable TxD & enable RxD
	RS485_DDR; // enable RS485 com direction output
	// Others
	DI_Curr_Bypass_ON_DDR;	// over current bypass indicator
	DDR__Output_On; // output enable line
	CLR__Output_On;
	CLRBIT(TIMER_port, TIMER_pin);	// clear Timer low output line
	TIMER_DDR;						// Enable line out for Timer
	DDR__LED_Output_On;
	CLR__LED_Output_On;
	DDR__LED_Error;
	CLR__LED_Error;
	
}

//-------------------------------------------------------------------------------------------
// Get RS485 Communication Address
void RS485_get_com_address_external_pins(){
	/* reads the address using the external pins
	- can not be used when JTAG is enabled
	- maybe find a more suitable IO solution later
	*/
	Com_ID = UART_ASCII_ADR_OFFSET; // = ADDRESS 0 = ASCII-@ --> address 1=A, 2=B
	// JTAG off, read address ports
	# ifdef PCBconfig_JTAG_is_disabled
		if (is_ADR_1_Set) {Com_ID = Com_ID + 1;}
		if (is_ADR_2_Set) {Com_ID = Com_ID + 2;}
		if (is_ADR_4_Set) {Com_ID = Com_ID + 4;}
		if (is_ADR_8_Set) {Com_ID = Com_ID + 8;}
		if (is_ADR_16_Set) {Com_ID = Com_ID + 16;}
	#endif
	// JTAG on, set 1 as default address
	# ifdef PCBconfig_JTAG_is_enabled
		Com_ID = Com_ID + 1; //debug default ADR = 1
	#endif
		
}


//===========================================================================================
// INTERUPT Function
//===========================================================================================
//-------------------------------------------------------------------------------------------
// Data read interrupt
ISR (USART0_RX_vect){
	
	// Store Status Register
	uint8_t TempSREG;
	TempSREG = SREG;
	
	// USART Wait and read
	uint8_t InUDR;
	while (!(UCSR0A & (1<<RXC0)));
	InUDR = UDR0;
	//UART_WriteChar(InUDR); // debug: return data to sender
		
	//Buffer handling
	if (InUDR==UART_TERM_CHAR){		//Command termination character received, push buffer to com input
		// check all is ready to go
		if ((UART_In_Buff[1] == UART_ADR_SEP_CHAR) && (UART_In_Buff[0] == Com_ID) && (UART_Comand_L == 0)) {
				// second character [1] address separation
				// if command is for this device, address in first character
				// if command buffer is empty, transfer new data
			// transfer input buffer to com processor
			uint8_t i;
			i=0;
			while (i <= UART_In_Buff_L-1) {	//Copy command from buffer to command process string
				UART_Comand[i] = UART_In_Buff[i];
				i++;
			}
			UART_Comand[i] = '\0';	//add string termination
			UART_Comand_L = i;	//set process string length
			UART_In_Buff_L = 0;	//reset buffer
			// debug: return to sender
			// char Value[UART_Value_LENGH+1];
			// char OutSTR[UART_COM_LENGH];
			// itoa (UART_Comand_L, Value, 10);	//Write int to string
			// Concentrate_String("Buffer	", Value, OutSTR); //Format output
			// AddEnd_String(OutSTR, "	");
			// AddEnd_String(OutSTR, UART_Comand);
			// UART_WriteString (&UART_Comand[0]);	//Output 
		}
		else {	
				// address separation character not found, clear buffer
				// com not for this device, clear buffer
				// last command buffer not empty yet, discard new command
			UART_In_Buff_L = 0;	//reset buffer
		}
	}
	else {		//No termination character received
		if (InUDR=='\0') {	// NULL character
		}					// discard
		else {			
			UART_In_Buff[UART_In_Buff_L] = InUDR;
			UART_In_Buff_L++;
			if(UART_In_Buff_L >= UART_COM_LENGH) {
				// UART buffer overflow, discard command line
				UART_In_Buff_L = 0;	//reset buffer				
			}
			//UART_WriteChar(UART_In_Buff[UART_In_Buff_L]); // debug: return to sender
		}
	}
		
	//Retrieve Status Register
	SREG = TempSREG;

}

//-------------------------------------------------------------------------------------------
// Data send interrupt
ISR (USART0_TX_vect){
	
	// Store Status Register
	uint8_t TempSREG;
	TempSREG = SREG;

	// USART Wait and send
	if (UART_Reply_P >= UART_Reply_L) {
		UCSR0B &= ~(1<<TXCIE0); // disable transmitter interrupt (transfer finished, buffer transmitted)
		CLRBIT(RS485_port, RS485_pin);		// RS485 com disable TxD & enable RxD
		UART_Reply_L = 0;
		UART_Reply_P = 0;
	}
	else {
		//while (!(UCSR0A & (1<<UDRE0))); // waits until ready (in case of debug UART_WriteChar calls)
		//_delay_us(10);
		UDR0 = UART_Reply[UART_Reply_P];
		UART_Reply_P++;
	}
	
	//Retrieve Status Register
	SREG = TempSREG;

}

//-------------------------------------------------------------------------------------------
// Main timing controller
ISR (TIMER1_COMPA_vect){
	
	// Store Status Register
	uint8_t TempSREG;
	TempSREG = SREG;
	
	// set measurement & com start flag
	if(is_Timer_CTR_Main_Flag_On) SET__Status_MainTimerOverRun;
	SET__Timer_CTR_Main_Flag;
	
	//Increase Meas control counter - check CAL counter
	Timer_Control_Counter++;
	if(Timer_Control_Counter == Timer_Control_Match) {
		SET__Timer_CTR_Control_Flag;
		Timer_Control_Counter = 0;
	}
	
	//Increase RTD Measurement counter - check MEAS counter
	Timer_Temp_Meas_Counter++;
	if(Timer_Temp_Meas_Counter == Timer_Temp_Meas_Match) {
		SET__Timer_TEMP_MEAS_Flag;
		Timer_Temp_Meas_Counter = 0;
	}
			
	//Retrieve Status Register
	SREG = TempSREG;

}

