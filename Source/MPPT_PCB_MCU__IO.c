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
	   Input Output functions C-Code FILE
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

// AI channels
volatile float AI_Volt;
volatile float AI_Curr;
volatile float AI_Curr_Corr; // leakage current corrected current measurement value
volatile float AI_Offset;
volatile float AI_Bias;
volatile float AI_NTC_Temp_1;
volatile float AI_NTC_Temp_2;
volatile float AI_RTD_Temp;

// AI Meas settings
volatile uint16_t ADC_Num_Avr_VoltCurr;
volatile uint8_t ADC_Cycl_Avr_VoltCurr;
volatile uint8_t ADC_Num_Avr_Other;
volatile float ADC_Avr_Mult_VoltCurr;
volatile float ADC_Avr_Mult_Other;
volatile uint32_t ADC_Cycl_Avr_Buff_Volt[ADC_Cycl_Avr_Buff_MAX];
volatile uint32_t ADC_Cycl_Avr_Buff_Curr[ADC_Cycl_Avr_Buff_MAX];
volatile uint8_t ADC_Cycl_Avr_Counter;

// AI input feedback
volatile uint8_t AI_Over_Load;
volatile uint8_t AI_Under_Load;

// AI calibration factors (for PV Curr & Volt see Range)
volatile float CAL_Bias_A0;
volatile float CAL_Bias_A1;
volatile float CAL_RTD_A0;
volatile float CAL_RTD_A1;
volatile float CAL_RTD_A2;
volatile float CAL_NTC_1_InvGain;
volatile float CAL_NTC_1_RSer;
volatile float CAL_NTC_1_InvR25;
volatile float CAL_NTC_1_InvBeta;
volatile float CAL_NTC_2_InvGain;
volatile float CAL_NTC_2_RSer;
volatile float CAL_NTC_2_InvR25;
volatile float CAL_NTC_2_InvBeta;

// AO calibration factors
volatile float CAL_DAC_A0;
volatile float CAL_DAC_A1;

// IO EXpander Variables (see Main.h for bit/pin details)
volatile uint8_t DIOE_PORT_A_STATE;
volatile uint8_t DIOE_PORT_B_STATE;

// Convert 16 to 8 bit integer
typedef union {
	uint16_t i16;
	struct {
		uint8_t i8l;
		uint8_t i8h;
	};
} convert16to8;

// Convert 32 to 8 bit integer
typedef union {
	uint32_t i32;
	struct {
		uint8_t i8B1;
		uint8_t i8B2;
		uint8_t i8B3;
		uint8_t i8B4;

	};
} convert32to8;

//===========================================================================================
// EEPROM VARIABLES
EEMEM uint16_t EROM_ADC_Num_Avr_VoltCurr = 50;
EEMEM uint8_t EROM_ADC_Cycl_Avr_VoltCurr = 4;
EEMEM uint8_t EROM_ADC_Num_Avr_Other = 10;
EEMEM float EROM_CAL_Bias_A0 = 0.0;
EEMEM float EROM_CAL_Bias_A1 = 0.000125;
EEMEM float EROM_CAL_RTD_A0 = -2.45681E+02;
EEMEM float EROM_CAL_RTD_A1 = 3.09122E-02;
EEMEM float EROM_CAL_RTD_A2 = 1.74219E-07;
EEMEM float EROM_CAL_NTC_1_InvGain = 0.465116;
EEMEM float EROM_CAL_NTC_1_RSer= 5e4;
EEMEM float EROM_CAL_NTC_1_InvR25 = 1.0E-04;
EEMEM float EROM_CAL_NTC_1_InvBeta = 2.91121E-04; // 3435 type
EEMEM float EROM_CAL_NTC_2_InvGain = 0.465116;
EEMEM float EROM_CAL_NTC_2_RSer= 5e4;
EEMEM float EROM_CAL_NTC_2_InvR25 = 1e-4;
EEMEM float EROM_CAL_NTC_2_InvBeta = 2.50752E-04; // 3988 type
EEMEM float EROM_CAL_DAC_A0 = 648.9;
EEMEM float EROM_CAL_DAC_A1 = 1.0;

//===========================================================================================
// FUNCTIONS
//===========================================================================================
//-------------------------------------------------------------------------------------------
// Basic function to measure ADC inputs
void Meas_Analog_Inputs(){
	/* says it on the tin, it measures all analog inputs to have the data ready for processing
	- uses ~100us for Analog switch activation, here many lower, as switching is done before data scaling*/
	uint32_t Temp_uint32;
	float Temp_float;
	uint8_t i;
		
	//Set_ADC_Mux_Voltage; // set Voltage channel input MUX address resting,  done at end now to save time
	//_delay_us(ADC_MUX_Set_Delay_us); 
	// meas voltage
	Temp_uint32 = ADC_Convert_Avr_OLUL_MAX11163(ADC_Num_Avr_VoltCurr);	// measure Voltage channel
	Set_ADC_Mux_Current; // set current input MUX address
	// write to the voltage buffer - buffer is used to average over multiple cycles (mainly for mains noise rejection)
	if (ADC_Cycl_Avr_Counter >= ADC_Cycl_Avr_VoltCurr) {
		ADC_Cycl_Avr_Counter = 0;
	}
	if (is_R_Stat_Volt_Clear_Buffer_Set) {		// clear buffer with new data if requested
		CLR__R_Stat_Volt_Clear_Buffer;
		i = ADC_Cycl_Avr_VoltCurr;
		while (i){
			i--;
			ADC_Cycl_Avr_Buff_Volt[i] = Temp_uint32;
		}
	}
	else ADC_Cycl_Avr_Buff_Volt[ADC_Cycl_Avr_Counter] = Temp_uint32;
	// sum up the voltage buffer
	Temp_uint32 = 0;
	i = ADC_Cycl_Avr_VoltCurr;
	while (i){
		i--; // counting down
		Temp_uint32 = Temp_uint32 + ADC_Cycl_Avr_Buff_Volt[i];
	}
	// calc new voltage
	Temp_float = (float) Temp_uint32 * ADC_Avr_Mult_VoltCurr;
	AI_Volt = (Temp_float + AI_Volt_Offset) * AI_Volt_Scale;
	// set OL and UL status bits for voltage channel
	if (AI_Under_Load) {
		SET__Status_Voltage_Error;
		SET__R_Stat_Volt_Under_Load;
		CLR__R_Stat_Volt_Over_Load;
	}
	else if (AI_Over_Load) {
		SET__Status_Voltage_Error;
		SET__R_Stat_Volt_Over_Load;
		CLR__R_Stat_Volt_Under_Load;
	}
	else {
		CLR__Status_Voltage_Error;
		CLR__R_Stat_Volt_Under_Load;
		CLR__R_Stat_Volt_Over_Load;
	}
	_delay_us(62); // remainder to get to the 100us MUX channel switching
		
		
	// meas current (mux set before voltage calcs)
	Temp_uint32 = ADC_Convert_Avr_OLUL_MAX11163(ADC_Num_Avr_VoltCurr);	// measure Current channel
	Set_ADC_Mux_Offset; // set offset voltage input MUX address
	// write to the current buffer - buffer is used to average over multiple cycles (mainly for mains noise rejection)
	if (is_R_Stat_Curr_Clear_Buffer_Set) {		// clear buffer with new data if requested
		CLR__R_Stat_Curr_Clear_Buffer;
		i = ADC_Cycl_Avr_VoltCurr;
		while (i){
			i--;
			ADC_Cycl_Avr_Buff_Curr[i] = Temp_uint32;
		}
	}
	else ADC_Cycl_Avr_Buff_Curr[ADC_Cycl_Avr_Counter] = Temp_uint32;
	ADC_Cycl_Avr_Counter ++; // increase buffer counter
	// sum up the current buffer
	Temp_uint32 = 0;
	i = ADC_Cycl_Avr_VoltCurr;
	while (i){
		i--; // counting down
		Temp_uint32 = Temp_uint32 + ADC_Cycl_Avr_Buff_Curr[i];
	}
	// calc new current
	Temp_float = (float) Temp_uint32 * ADC_Avr_Mult_VoltCurr;
	AI_Curr = (Temp_float + AI_Curr_Offset) * AI_Curr_Scale;
	AI_Curr_Corr = AI_Curr + AI_Volt * AI_Volt_R_Leak;
	// set OL and UL status bits for current channel
	if (AI_Under_Load) {
		SET__Status_Current_Error;
		SET__R_Stat_Curr_Under_Load;
		CLR__R_Stat_Curr_Over_Load;
	}
	else if (AI_Over_Load) {
		SET__Status_Current_Error;
		SET__R_Stat_Curr_Over_Load;
		CLR__R_Stat_Curr_Under_Load;
	}
	else {
		CLR__Status_Current_Error;
		CLR__R_Stat_Curr_Under_Load;
		CLR__R_Stat_Curr_Over_Load;
	}
	_delay_us(40); // remainder to get to the 100us MUX channel switching
	
	
	// meas offset (mux set before current calcs)
	Temp_uint32 = ADC_Convert_Avr_MAX11163(ADC_Num_Avr_Other);	// measure Offset channel
	Set_ADC_Mux_Bias; // set bias volt input MUX address
	Temp_float = (float) Temp_uint32 * ADC_Avr_Mult_Other;
	AI_Offset = Temp_float;
	_delay_us(88); // remainder to get to the 100us MUX channel switching
	
	// meas bias (mux set before current calcs)
	Temp_uint32 = ADC_Convert_Avr_MAX11163(ADC_Num_Avr_Other);	// measure bias channel
	Set_ADC_Mux_NTC_1; // set NTC Temp 1 input MUX address
	Temp_float = (float) Temp_uint32 * ADC_Avr_Mult_Other;
	AI_Bias = (Temp_float * CAL_Bias_A1) + CAL_Bias_A0;
	_delay_us(73); // remainder to get to the 100us MUX channel switching
	
	// meas NTC Temp Ch 1
	Temp_uint32 = ADC_Convert_Avr_MAX11163(ADC_Num_Avr_Other);	// measure NTC Temp 1 channel
	Set_ADC_Mux_NTC_2; // set NTC Temp 2 input MUX address
	Temp_float = (float) Temp_uint32 * ADC_Avr_Mult_Other;
	Temp_float = Temp_float * CAL_NTC_1_InvGain; // counts without gain
	Temp_float = (Temp_float / (65536 - Temp_float)) * CAL_NTC_1_RSer; // Resistance NTC
	AI_NTC_Temp_1 = 1/ (log(Temp_float * CAL_NTC_1_InvR25) * CAL_NTC_1_InvBeta + 3.35402E-03) - 273.15;
	//_delay_us(80); // calcs take lots of time already... ~270us
	
	// meas NTC Temp Ch 2
	Temp_uint32 = ADC_Convert_Avr_MAX11163(ADC_Num_Avr_Other);	// measure NTC Temp 2 channel
	Set_ADC_Mux_Voltage; // go back to voltage measurements to safe time next round
	//	Set_ADC_Mux_GND; // set GND input MUX address, resting
	Temp_float = (float) Temp_uint32 * ADC_Avr_Mult_Other;
	Temp_float = Temp_float * CAL_NTC_2_InvGain; // counts without gain
	Temp_float = (Temp_float / (65536 - Temp_float)) * CAL_NTC_2_RSer; // Resistance NTC
	AI_NTC_Temp_2 = 1/ (log(Temp_float * CAL_NTC_2_InvR25) * CAL_NTC_2_InvBeta + 3.35402E-03) - 273.15;
	
		
}

//-------------------------------------------------------------------------------------------
// Measure Voltage Only
void Meas_AI_Volt_Only() {
	/* function measures voltage channel at the MUX */
	uint32_t Temp_uint32;
	float Temp_float;
	
	// set voltage input MUX address
	Set_ADC_Mux_Voltage;
	_delay_us(ADC_MUX_Set_Delay_us); // time it takes to set the channel and settle the signal at the ADC
	
	Temp_uint32 = ADC_Convert_Avr_OLUL_MAX11163((ADC_Num_Avr_VoltCurr * ADC_Cycl_Avr_VoltCurr));	// measure Voltage channel
	
	// Calculate results and set OL & UL indicators
	Temp_float = (float) Temp_uint32 * ADC_Avr_Mult_VoltCurr;
	AI_Volt = (Temp_float + AI_Volt_Offset) * AI_Volt_Scale;
	if (AI_Under_Load) {
		SET__R_Stat_Volt_Under_Load;
		CLR__R_Stat_Volt_Over_Load;
	}
	else if (AI_Over_Load) {
		SET__R_Stat_Volt_Over_Load;
		CLR__R_Stat_Volt_Under_Load;
	}
	else {
		CLR__R_Stat_Volt_Under_Load;
		CLR__R_Stat_Volt_Over_Load;
	}
	
	Set_ADC_Mux_GND; // set GND input MUX address, resting
	
}

//-------------------------------------------------------------------------------------------
// Measure Current Only
void Meas_AI_Curr_Only() {
	/* function measures voltage channel at the MUX */
	uint32_t Temp_uint32;
	float Temp_float;
	
	// Set Current channel on MUX
	Set_ADC_Mux_Current;
	_delay_us(ADC_MUX_Set_Delay_us); // time it takes to set the channel and settle the signal at the ADC
	
	Temp_uint32 = ADC_Convert_Avr_OLUL_MAX11163((ADC_Num_Avr_VoltCurr * ADC_Cycl_Avr_VoltCurr));	// measure Voltage channel
	
	// Calculate results and set OL & UL indicators
	Temp_float = (float) Temp_uint32 * ADC_Avr_Mult_VoltCurr;
	AI_Curr = (Temp_float + AI_Curr_Offset) * AI_Curr_Scale;
	if (AI_Under_Load) {
		SET__R_Stat_Curr_Under_Load;
		CLR__R_Stat_Curr_Over_Load;
	}
	else if (AI_Over_Load) {
		SET__R_Stat_Curr_Over_Load;
		CLR__R_Stat_Curr_Under_Load;
	}
	else {
		CLR__R_Stat_Curr_Under_Load;
		CLR__R_Stat_Curr_Over_Load;
	}
	
	Set_ADC_Mux_GND; // set GND input MUX address, resting
	
}

//-------------------------------------------------------------------------------------------
// DAC output control function in Volts
void Set_DAC_Output_Volts(float Voltage){
	/* converts the voltage to RAW DAC counts, dependent of measurement/controller range 
	- the actual range is controlled by the analog input & the output updates with changes by new function calls*/
	uint16_t DAC_Counts;
	float Tmp_Float;
	
	// calculate digital counts at the DAC
	Tmp_Float = ((Voltage / AI_Volt_Scale) * CAL_DAC_A1) + CAL_DAC_A0;
	//check if within boundaries
	if (Tmp_Float >= DAC_MAX_COUNTS) {
		Tmp_Float = DAC_MAX_COUNTS;
	}
	else if (Tmp_Float < CAL_DAC_A0) {
		Tmp_Float = CAL_DAC_A0;
	}
	// set & send
	DAC_Counts = (uint16_t) (Tmp_Float);
	PV_Volt_DAC_ACT = Voltage;
	Set_DAC_Output_RAW(DAC_Counts);

}

//-------------------------------------------------------------------------------------------
// ADC Convert (works for MAX11163 and similar positive only full 16bit ADC)
int16_t ADC_Convert_MAX11163(){
	/* function used to directly measure the ADC without averaging */
	convert16to8 ADC_Return;
	
	_delay_us(1); // Acquisition time
	SETBIT(CS_AI_CNV_port, CS_AI_CNV_pin);	 // send conversion start signal
	//CLRBIT(CS_AI1_port, CS_AI1_pin);	// disable chip select (not needed must be set)
	_delay_us(3); // conversion time
	CLRBIT(CS_AI_CNV_port, CS_AI_CNV_pin);	 // enable SDO
	//CLRBIT(CS_AI1_port, CS_AI1_pin);	// enable chip select (not needed must be set)
		
	SPDR = 0b00000000;
	while(!(SPSR & (1<<SPIF))){
	}
	ADC_Return.i8h = SPDR;
	SPDR = 0b00000000;
	while(!(SPSR & (1<<SPIF))){
	}
	ADC_Return.i8l = SPDR;
	
	//SETBIT(CS_AI1_port, CS_AI1_pin);	// disable chip select (not needed must be set)
	return ADC_Return.i16; //Convert to 16bit value

}

//-------------------------------------------------------------------------------------------
// ADC Convert with Averaging (works for MAX11163 and similar positive only full 16bit ADC)
uint32_t ADC_Convert_Avr_MAX11163(uint16_t Nu_Avr){
	/* function used to  measure the ADC with averaging
	- does not return average but sum of measurements!!! */
	convert32to8 ADC_Return;
	uint32_t	ADC_AVR;
	uint16_t i;
	ADC_Return.i32 = 0;
	ADC_AVR = 0;
	i = Nu_Avr;
	
	while(i) {
		i --; // counting down to  zero
		_delay_us(1); // Acquisition time
		SETBIT(CS_AI_CNV_port, CS_AI_CNV_pin);	 // send conversion start signal
		//CLRBIT(CS_AI1_port, CS_AI1_pin);	// disable chip select (not needed must be set)
		_delay_us(3); // conversion time
		CLRBIT(CS_AI_CNV_port, CS_AI_CNV_pin);	 // enable SDO
		//CLRBIT(CS_AI1_port, CS_AI1_pin);	// enable chip select (not needed must be set)
	
		SPDR = 0b00000000;
		while(!(SPSR & (1<<SPIF))){
		}
		ADC_Return.i8B2 = SPDR;
		SPDR = 0b00000000;
		while(!(SPSR & (1<<SPIF))){
		}
		ADC_Return.i8B1 = SPDR;
	
		//SETBIT(CS_AI1_port, CS_AI1_pin);	// disable chip select (not needed must be set)
		ADC_AVR = ADC_AVR + ADC_Return.i32;
	}
	
	return ADC_AVR; // return 32bit  sum of all measurements

}

//-------------------------------------------------------------------------------------------
// ADC Convert with Averaging & over-under load check (works for MAX11163 and similar positive only full 16bit ADC)
uint32_t ADC_Convert_Avr_OLUL_MAX11163(uint16_t Nu_Avr){
	/* function used to  measure the ADC with averaging
	- does not return average but sum of measurements!!! */
	convert32to8 ADC_Return;
	uint32_t	ADC_AVR;
	uint16_t i;
	ADC_Return.i32 = 0;
	ADC_AVR = 0;
	AI_Over_Load = 0;
	AI_Under_Load = 0;
	i = Nu_Avr;
	
	while(i) {
		i --; // counting down to  zero
		_delay_us(1); // Acquisition time
		SETBIT(CS_AI_CNV_port, CS_AI_CNV_pin);	 // send conversion start signal
		//CLRBIT(CS_AI1_port, CS_AI1_pin);	// disable chip select (not needed must be set)
		_delay_us(3); // conversion time
		CLRBIT(CS_AI_CNV_port, CS_AI_CNV_pin);	 // enable SDO
		//CLRBIT(CS_AI1_port, CS_AI1_pin);	// enable chip select (not needed must be set)
	
		SPDR = 0b00000000;
		while(!(SPSR & (1<<SPIF))){
		}
		ADC_Return.i8B2 = SPDR;
		SPDR = 0b00000000;
		while(!(SPSR & (1<<SPIF))){
		}
		ADC_Return.i8B1 = SPDR;
	
		//SETBIT(CS_AI1_port, CS_AI1_pin);	// disable chip select (not needed must be set)
		if (ADC_Return.i8B2 == 0 && ADC_Return.i8B1 == 0) {
			AI_Under_Load = 1;
		}
		else if (ADC_Return.i8B2 == 255 && ADC_Return.i8B1 == 255) {
			AI_Over_Load = 1;
		}
		ADC_AVR = ADC_AVR + ADC_Return.i32;
	}
	
	return ADC_AVR; // return 32bit sum of all measurements

}


//-------------------------------------------------------------------------------------------
// DAC Output function - raw data in counts (works for MAX5216 and DAQ8550 16bit DAC)
void Set_DAC_Output_RAW(uint16_t DAC_val){
	/* low level function to set the DAC output value in raw counts*/
		
	#ifdef PCBconfig_DAQ_is_MAX5216 // DAQ control for MAX5216
		convert32to8 ADC_OUT;
		SETBIT(SPCR, CPOL); //normal high clock for DAQ
		//CLRBIT(SPCR0, CPHA0);			// falling edge out
		ADC_OUT.i8B4 = 0;
		ADC_OUT.i8B3 = (uint8_t) (DAC_val >> 10);
		SETBIT(ADC_OUT.i8B3, 6); // control bit D23 low, D22 hight
		ADC_OUT.i8B2 = (uint8_t) (DAC_val >> 2);
		ADC_OUT.i8B1 = (uint8_t) (DAC_val << 6);
		
		CLRBIT(CS_AO1_port,CS_AO1_pin);	// enable MPPT DAQ chip select
		
		SPDR = ADC_OUT.i8B3;
		while(!(SPSR & BIT(SPIF))){
		}
		SPDR = ADC_OUT.i8B2;
		while(!(SPSR & BIT(SPIF))){
		}
		SPDR = ADC_OUT.i8B1;
		while(!(SPSR & BIT(SPIF))){
		}
		SETBIT(CS_AO1_port,CS_AO1_pin);	// disable MPPT DAQ chip select
		//SETBIT(SPCR0, CPHA0);			// rising edge out
		CLRBIT(SPCR, CPOL); //normal low clock for main ADC
	#endif // PCBconfig_DAQ_is_MAX5216
	
	#ifdef PCBconfig_DAQ_is_DAQ8550 // DAQ control for DAQ8550
		SETBIT(SPCR, CPOL); //normal high clock for DAQ
		DAC_val = DAC_val - 32767;
			
		CLRBIT(CS_AO1_port,CS_AO1_pin);	// enable MPPT DAQ chip select
	
		SPDR = 0;
		while(!(SPSR & BIT(SPIF))){
		}
		SPDR = (uint8_t) (DAC_val >> 8);
		while(!(SPSR & BIT(SPIF))){
		}
		SPDR = (uint8_t) (DAC_val);
		while(!(SPSR & BIT(SPIF))){
		}
		SETBIT(CS_AO1_port,CS_AO1_pin);	// disable MPPT DAQ chip select
		CLRBIT(SPCR, CPOL); //normal low clock for main ADC
	#endif // PCBconfig_DAQ_is_DAQ8550
}


//-------------------------------------------------------------------------------------------
// RTD ADC Set-up (works for MAX31865 15bit ADC)
void TEMP_MAX31865_Setup(){
	/* setup function to get the MAX 31865 RTD ADC running */
	uint8_t ADC_CFG;
	uint8_t ADC_SET;
	
	// init communication (set SPI interface)
	SETBIT(SPCR,SPR0);	
	SETBIT(SPCR,SPR1);	// clock is now fck/128

	SETBIT(SPCR, CPHA);  // rising edge output
	//SETBIT(SPCR, CPOL); // normal high clock for PT100 ADC
	CLR__EXP_DIO_1;  // enable chip select
	
	// write configuration register to set the measurement mode
 	ADC_CFG = 0b10000000; // write, test register
	ADC_SET = 0b11100000; // all clear
	#ifdef Line_Freq_50 // adjust config byte if 50Hz line frequency
		ADC_SET = ADC_SET + 1;
	#endif /* Line_Freq_50 */
 	SPDR = ADC_CFG;
 	while(!(SPSR & BIT(SPIF))){
 	}
 	SPDR = ADC_SET;
 	while(!(SPSR & BIT(SPIF))){
 	}

	// finish communication to device (reset SPI settings)
	SET__EXP_DIO_1;  // disable chip select
	CLRBIT(SPCR, CPHA);  // falling edge output main ADC
	//CLRBIT(SPCR, CPOL); // normal low clock for main ADC
	CLRBIT(SPCR,SPR0);	// clock fck/16
}

//-------------------------------------------------------------------------------------------
// RTD ADC Conversion (works for MAX31865 15bit ADC)
int32_t TEMP_MAX31865_Measure(){
	/* collect measurement data from MAX31865 RTD ADC, done after receiving DATA_READY signal*/
	uint8_t ADC_CFG;
	convert32to8 ADC_IN;
	
	// start up the SPI, set PT100 ADC mode
	SETBIT(SPCR,SPR0);
	SETBIT(SPCR,SPR1);	// clock is now fck/128

	SETBIT(SPCR, CPHA);  // rising edge output

	CLR__EXP_DIO_1;	// enable chip select
	
	ADC_CFG = 0b00000001; // read, MSB
	SPDR = ADC_CFG;
	while(!(SPSR & BIT(SPIF))){
	}
	SPDR = 0b11111111;
	while(!(SPSR & BIT(SPIF))){
	}
	ADC_IN.i8B2 = SPDR;
	// read LSB consecutively
	SPDR = 0b11111111;
	while(!(SPSR & BIT(SPIF))){
	}
	ADC_IN.i8B1 = SPDR;
	
	ADC_IN.i32 = (ADC_IN.i32 >> 1);
	
	// finish up SPI, put back to normal
	SET__EXP_DIO_1;	// disable chip select
	CLRBIT(SPCR, CPHA);  // falling edge output main ADC
	//CLRBIT(SPCR, CPOL); // normal low clock for main ADC
	CLRBIT(SPCR,SPR0);	// clock fck/16
		
	return ADC_IN.i32; //Convert to 16bit value

}


//-------------------------------------------------------------------------------------------
// Precision Thermocouple to digital converter Set-up (works for MAX31856)
//SPCR = Control register
//SPSR = Status register
//SPDR = Data register
void TEMP_MAX31856_Setup(){
	uint8_t ADC_CFG; //Address
	uint8_t ADC_SET; //Data
	
	// init communication (set SPI interface)
	SETBIT(SPCR,SPR0);	
	SETBIT(SPCR,SPR1);	// clock is now fck/128
	SETBIT(SPCR, CPHA);  // rising edge output
	
	
	// write configuration0
 	ADC_CFG = 0b10000000; // write, register config0
	ADC_SET = 0b00000010; // no fault detection, clear faults
	#ifdef Line_Freq_50 // adjust config byte if 50Hz line frequency
		ADC_SET = ADC_SET + 1;
	#endif /* Line_Freq_50 */

	//Start communication
	CLR__EXP_DIO_1;  // enable chip select
 	SPDR = ADC_CFG;
 	while(!(SPSR & BIT(SPIF))){ //Wait till register is sent
 	}
 	SPDR = ADC_SET;
 	while(!(SPSR & BIT(SPIF))){//Wait till data is sent
 	}
	SET__EXP_DIO_1;  // disable chip select

	
	// write configuration1
 	ADC_CFG = 0b10000001; // write, register config1
	ADC_SET = 0b01110111; // Average over 16 samples, T-type ADJUST FOR OWN TYPE TERMALCOUPLE
	//Start communication
	CLR__EXP_DIO_1;  // enable chip select
 	SPDR = ADC_CFG;
 	while(!(SPSR & BIT(SPIF))){ //Wait till register is sent
 	}
 	SPDR = ADC_SET;
 	while(!(SPSR & BIT(SPIF))){//Wait till data is sent
 	}
	SET__EXP_DIO_1;  // disable chip select


	// Enable auto conversion
 	ADC_CFG = 0b10000000; // write, register config0
	ADC_SET = 0b10000000; // start autoconversion
	#ifdef Line_Freq_50 // adjust config byte if 50Hz line frequency
		ADC_SET = ADC_SET + 1;
	#endif /* Line_Freq_50 */

	//Start communication
	CLR__EXP_DIO_1;  // enable chip select
 	SPDR = ADC_CFG;
 	while(!(SPSR & BIT(SPIF))){ //Wait till register is sent
 	}
 	SPDR = ADC_SET;
 	while(!(SPSR & BIT(SPIF))){//Wait till data is sent
 	}
	SET__EXP_DIO_1;  // disable chip select


	// finish communication to device (reset SPI settings)
	CLRBIT(SPCR, CPHA);  // falling edge output main ADC
	CLRBIT(SPCR,SPR1);	// clock fck/16
}

//Measurement for MAX31856, returns temperature in Celcius as float
float TEMP_MAX31856_Measure(){
	float Temperature;
	uint8_t ADC_CFG;
    uint8_t LTCBH;
    uint8_t LTCBM;
    uint8_t LTCBL;
    int32_t raw;

	// start up the SPI, 
	SETBIT(SPCR,SPR0);
	CLRBIT(SPCR,SPR1);	// clock is now fck/16
	SETBIT(SPCR, CPHA);  // rising edge output

	
	//Start reading bytes
	ADC_CFG = 0x0C; // read, LTC_BH

	CLR__EXP_DIO_1;	// enable chip select
	SPDR = ADC_CFG;
	while(!(SPSR & BIT(SPIF))){
	}
	SPDR = 0b11111111; //Send dummy byte
	while(!(SPSR & BIT(SPIF))){
	}
	LTCBH = SPDR;
	// read next byte 
	SPDR = 0b11111111;
	while(!(SPSR & BIT(SPIF))){
	}
	LTCBM = SPDR; //LTC_BM
	// read next byte 
	SPDR = 0b11111111;
	while(!(SPSR & BIT(SPIF))){
	}
	LTCBL = SPDR; //LTC_BL	

	// finish up SPI, put back to normal
	SET__EXP_DIO_1;	// disable chip select
	CLRBIT(SPCR, CPHA);  // falling edge output main ADC
	//CLRBIT(SPCR, CPOL); // normal low clock for main ADC
	CLRBIT(SPCR,SPR1);	// clock fck/16

	//Convert to temperature
	raw = ((int32_t)LTCBH << 16 ) | ((int32_t)LTCBM << 8 ) | LTCBL;
	raw >>=5; //Remove 5 lowest bits, temperature is only 19 bits including 1 sign bit

	//If signed
	if (raw & 0x40000) {
		raw |= 0xFFF80000; 
	}

	Temperature = raw * 0.0078125;
	return Temperature;
}




//-------------------------------------------------------------------------------------------
// TC ADC Set-up (works for MCP9600 TC ADC)
void TEMP_MCP9600_Setup(){
	/* This function configures the MCP9600 thermocouple ADC via IC2 bus */
	// TODO IC2 error status might need implementation, timeout by watchdog is active
	//send start
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != START) goto TEMP_MCP9600_Setup_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	//send address
	TWDR = MCP9600_ADR_Write;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto TEMP_MCP9600_Setup_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	//send command
	TWDR = MCP9600_REG_TC_Config;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto TEMP_MCP9600_Setup_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	// send data
	TWDR = MCP9600_DATA_TC_Config;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto TEMP_MCP9600_Setup_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	// send stop
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);
	//send start
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != START) goto TEMP_MCP9600_Setup_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	//send address
	TWDR = MCP9600_ADR_Write;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto TEMP_MCP9600_Setup_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	//send command
	TWDR = MCP9600_REG_DEV_Config;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto TEMP_MCP9600_Setup_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	// send data
	TWDR = MCP9600_DATA_DEV_Config;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto TEMP_MCP9600_Setup_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	// send stop
	TEMP_MCP9600_Setup_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);
	
}

//-------------------------------------------------------------------------------------------
// TC ADC Measurement (works for MCP9600 TC ADC)
float TEMP_MCP9600_Measure(){
	/* This function reads the compensated TC temperature from the MCP96000 and converts the value */
	convert16to8 Data_In;
	Data_In.i16 = 65000;
	float Temperature;
	//send start - for sending the register to read from
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != START) goto TEMP_MCP9600_Measure_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	//send address
	TWDR = MCP9600_ADR_Write;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto TEMP_MCP9600_Measure_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	//send command
	TWDR = MCP9600_REG_TC_Temp;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto TEMP_MCP9600_Measure_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	// send stop
	//TEMP_MCP9600_Measure_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(20);
	//send start - for reading the data at the register
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != START) goto TEMP_MCP9600_Measure_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);
	//send address
	TWDR = MCP9600_ADR_Read;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_SLA_ACK) goto TEMP_MCP9600_Measure_Sent_Stop;
	// read data byte high - send acknowledge
	TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_DATA_ACK) goto TEMP_MCP9600_Measure_Sent_Stop;
	Data_In.i8h = TWDR;
	// read data byte low - send not acknowledge (end data receive)
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_DATA_NAK) goto TEMP_MCP9600_Measure_Sent_Stop;
	Data_In.i8l = TWDR;
	// send stop
	TEMP_MCP9600_Measure_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	// _delay_us(IC2_COM_DELAY_us); no need to wait here
		
	// convert data
	if ((Data_In.i8h & 0x80) == 0x80) Temperature = ( (float) Data_In.i16 * 0.0625) - 4096.0;
	else Temperature = ( (float) Data_In.i16 * 0.0625);
		
	return Temperature;
}

//-------------------------------------------------------------------------------------------
// IOExpander configuration & initialization for start-up (works for PCA9555 16bit DIOs)
void DIOExp_config_init(){
	/*
	This function initializes the Digital IO expander on the board via IC2 bus.
	*/
	
	// Set-up IC2 data transfer frequency (~132kHz at 20Mhz) or 111kHz at 16Mhz
	TWBR = 64; // frequency scaler
	CLRBIT(TWSR, TWPS1); // pre-scaler
	CLRBIT(TWSR, TWPS0); // pre-scaler
	
	// write default output registers before setting data direction registers
	DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
	
	// Set GPIO data direction registers
	DIOExp_Write_Outputs(DIO_REG_DDR, DIO_DATA_DDRA, DIO_DATA_DDRB);
}

//-------------------------------------------------------------------------------------------
// IOExpander Read inputs - this is in 16bit mode (works for PCA9555 16bit DIOs)
void DIOExp_Write_Outputs(uint8_t REG_byte, uint8_t DATA_Port_A, uint8_t DATA_Port_B){
	/*
	This function writes to the Digital IO expander on the board via IC2 bus.
	*/
	// IC2 PCA9555 ....
	// TODO IC2 error status might need implementation, timeout by watchdog is active
	//send start
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != START) goto DIOE_IC2_Write_Sent_Stop;
	//send address
	TWDR = DIO_ADR_Write;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto DIOE_IC2_Write_Sent_Stop;
	//send command
	TWDR = REG_byte;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto DIOE_IC2_Write_Sent_Stop;
	// send data 0 - A
	TWDR = DATA_Port_A;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto DIOE_IC2_Write_Sent_Stop;
	// send data 1 - B
	TWDR = DATA_Port_B;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto DIOE_IC2_Write_Sent_Stop;
	// send stop
	DIOE_IC2_Write_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(20);
	
}


//-------------------------------------------------------------------------------------------
// IOExpander Read inputs - this is in 16bit mode 
// (works for MCP23S17 BUT NOT yet for PCA9555 16bit DIOs [needs filling in the code when needed])
uint16_t DIOExp_Read_Ports(uint8_t REG_byte){
	/* 
	This function reads from the Digital IO expander on the board via IC2 bus.
	*/
		
	// IC2 PCA9555 ....
	convert16to8 Data_In;
	Data_In.i16 = 0;
	// code the reading function, but not needed here....
	_delay_us(20);
		
	return Data_In.i16;
	
}