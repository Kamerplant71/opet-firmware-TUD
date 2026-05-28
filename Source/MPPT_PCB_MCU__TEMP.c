/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Justen van Koppen
	Version:	1.15
	Date:		20.05.2026
	-----------------------------------------
	ATMega 1284 Micro controller support
	-----------------------------------------
	  Communication function C-Code FILE
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
// VARIABLES and STRUCTURES

volatile float AI_HEAT_NTC_Temp[8];
volatile uint8_t AI_HEAT_ADC_MASK[8];



//===========================================================================================
// EEPROM VARIABLES

//===========================================================================================
// FUNCTIONS
//===========================================================================================
//-------------------------------------------------------------------------------------------

//Try to setup all adc's
void Temp_monitoring_Setup(){
	for (uint8_t i =0; i<8; i++){
		ADC121C021_Setup(BASE_ADDRESS_ADC121C021+i);
	}
}


//Read temperature of all ADC's
void Read_Temp_All_ADC121C021(){

	uint16_t raw;
	for (uint8_t i =0; i<8; i++){
		raw = ADC121C021_ReadRaw(i);
		AI_HEAT_NTC_Temp[i] = NTC_RawToTemp(raw);
	}
}





//Setup ADC with correct values in registers
//I2C clock already configured in DIOExp_config_init as 111kHz
void ADC121C021_Setup(uint8_t address){

	ADC121C021_SetConfig(address);
	ADC121C021_Set_Vhigh(address);
	ADC121C021_Set_Vhyst(address);

}


//Reads the 12 bit result of ADC
uint16_t ADC121C021_ReadRaw(uint8_t i){
	//Input is the i th ADC to read (from 0 to 7)
    
	uint8_t address = BASE_ADDRESS_ADC121C021 + i;
	
	uint8_t msb= 0;
    uint8_t lsb= 0;
    uint16_t raw = 0;

	//Send start
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT))); //Wait for start to be sent
	if ((TWSR & 0xF8) != START) goto ADC121C021_ReadRaw_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);


	//send address
	TWDR = (address<<1); //Write address
	TWCR = (1<<TWINT)|(1<<TWEN); //Start transmission address
	while (!(TWCR & (1<<TWINT))); //Wait for interrupt
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto ADC121C021_ReadRaw_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send register
	TWDR = ADC121C021_REG_CONV_RESULT;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_ReadRaw_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	//send repeated start
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT))); //Wait for start to be sent
	if ((TWSR & 0xF8) != REPEATED_START) goto ADC121C021_ReadRaw_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send address
	TWDR = (address<<1) | (1<<0); //read address
	TWCR = (1<<TWINT)|(1<<TWEN); //Start transmission address
	while (!(TWCR & (1<<TWINT))); //Wait for interrupt
	if ((TWSR & 0xF8) != MR_SLA_ACK) goto ADC121C021_ReadRaw_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);	

	// read data byte high - send acknowledge
	TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_DATA_ACK) goto ADC121C021_ReadRaw_Sent_Stop;
	msb = TWDR;

	// read data byte low - send not acknowledge (end data receive)
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MR_DATA_NAK) goto ADC121C021_ReadRaw_Sent_Stop;
	lsb = TWDR;

	//send stop
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);	

	//Succesfull data transfer update mask and return data
	AI_HEAT_ADC_MASK[i] = 1;
	raw = (((uint16_t)msb <<8) | lsb) & 0x0FFF; //Only 12 bits of ADC information
	return raw;
	
	// send stop unexpected I2C event
	ADC121C021_ReadRaw_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);	

	//Unsuccesfull data transfer
	AI_HEAT_ADC_MASK[i] = 0;

	return raw;
}

void ADC121C021_SetConfig(uint8_t address){
	//Set configuration register
	//Send start condition
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT))); //Wait for start to be sent
	if ((TWSR & 0xF8) != START) goto ADC121C021_Setup_Config_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);


	//send address
	TWDR = (address<<1); //Write address
	TWCR = (1<<TWINT)|(1<<TWEN); //Start transmission address
	while (!(TWCR & (1<<TWINT))); //Wait for interrupt
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto ADC121C021_Setup_Config_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send register
	TWDR = ADC121C021_REG_CONFIG;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Config_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send data
	TWDR = 0b11100010; // 0.4ksps auto conversion, Alert self clear,Enable alert ,Alert active low
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Config_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send stop
	ADC121C021_Setup_Config_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);
}

void ADC121C021_Set_Vhigh(uint8_t address){
	//Set V_high
	//Send start condition
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT))); //Wait for start to be sent
	if ((TWSR & 0xF8) != START) goto ADC121C021_Setup_Vhigh_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send address
	TWDR = (address<<1); //Write address
	TWCR = (1<<TWINT)|(1<<TWEN); //Start transmission address
	while (!(TWCR & (1<<TWINT))); //Wait for interrupt
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto ADC121C021_Setup_Vhigh_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send register
	TWDR = ADC121C021_REG_V_HIGH;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vhigh_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send high byte
	TWDR = 0x0F; //TODO: set correct high limit 
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vhigh_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send low byte
	TWDR = 0xFF; //TODO: set correct high limit 
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vhigh_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send stop
	ADC121C021_Setup_Vhigh_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);
}

void ADC121C021_Set_Vhyst(uint8_t address){
	//Set V_hyst
	//Send start condition
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT))); //Wait for start to be sent
	if ((TWSR & 0xF8) != START) goto ADC121C021_Setup_Vhyst_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send address
	TWDR = (address<<1) ; //Write address
	TWCR = (1<<TWINT)|(1<<TWEN); //Start transmission address
	while (!(TWCR & (1<<TWINT))); //Wait for interrupt
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto ADC121C021_Setup_Vhyst_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send register
	TWDR = ADC121C021_REG_V_HYST;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vhyst_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send high byte
	TWDR = 0x00; //TODO: set correct hyst level
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vhyst_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send low byte
	TWDR = 0x00; //TODO: set correct hyst level
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vhyst_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send stop
	ADC121C021_Setup_Vhyst_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);
}

float NTC_RawToTemp(uint16_t raw){
    float ratio;
    float r_ntc;
    float inv_t;
    float temp_k;
    float temp_c;

    if (raw == 0 || raw >= 4095) {
        return 0.0f;   //Invalid return 0
    }

    ratio = (float)raw / 4095.0;

    r_ntc = R_FIXED_OHM * ratio / (1.0 - ratio);

    inv_t = (1.0 / 298.15) + (log(r_ntc / NTC_R0_OHM) / NTC_BETA_K);
    temp_k = 1.0 / inv_t;
    temp_c = temp_k - 273.15;

    return temp_c; 
}