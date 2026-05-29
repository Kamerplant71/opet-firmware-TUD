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
volatile uint8_t AI_HEAT_ADC_PRESENT_MASK;
volatile float TEMP_MAX_Heat_dissipation;


//===========================================================================================
// EEPROM VARIABLES

//===========================================================================================
// FUNCTIONS
//===========================================================================================
//-------------------------------------------------------------------------------------------

// Try to setup all adc's
void Temp_monitoring_Setup(){
    AI_HEAT_ADC_PRESENT_MASK = 0;

    for (uint8_t i = 0; i < 8; i++) {

        if (ADC121C021_IsPresent(BASE_ADDRESS_ADC121C021 + i)) {

            AI_HEAT_ADC_PRESENT_MASK |= (1 << i);

            ADC121C021_Setup(BASE_ADDRESS_ADC121C021 + i);
        }
    }
}


// Read temperature of all ADC's
void Read_Temp_All_ADC121C021(){

	uint16_t raw;
    for (uint8_t i = 0; i < 8; i++) {

        if (!(AI_HEAT_ADC_PRESENT_MASK & (1 << i))) {
            continue;
        }

        if (ADC121C021_ReadRaw(i, &raw)) {
            AI_HEAT_NTC_Temp[i] = NTC_RawToTemp(raw);
        }
        else {
            AI_HEAT_ADC_PRESENT_MASK &= ~(1 << i);
        }
    }
}





// Setup ADC with correct values in registers
// I2C clock already configured in DIOExp_config_init as 111kHz
void ADC121C021_Setup(uint8_t address){

	ADC121C021_SetConfig(address);
	ADC121C021_Set_Vlow(address);
	ADC121C021_Set_Vhyst(address);

}


// Reads the 12 bit result of ADC and gives an success rate as output
uint8_t ADC121C021_ReadRaw(uint8_t i, uint16_t *raw_out){
    uint8_t address = BASE_ADDRESS_ADC121C021 + i;

    uint8_t msb = 0;
    uint8_t lsb = 0;

    // Send start
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_ReadRaw_Error;
    if ((TWSR & 0xF8) != START) goto ADC121C021_ReadRaw_Error;
    _delay_us(IC2_COM_DELAY_us);

    // Send address write
    TWDR = (address << 1);
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_ReadRaw_Error;
    if ((TWSR & 0xF8) != MT_SLA_ACK) goto ADC121C021_ReadRaw_Error;
    _delay_us(IC2_COM_DELAY_us);

    // Send register
    TWDR = ADC121C021_REG_CONV_RESULT;
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_ReadRaw_Error;
    if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_ReadRaw_Error;
    _delay_us(IC2_COM_DELAY_us);

    // Repeated start
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_ReadRaw_Error;
    if ((TWSR & 0xF8) != REPEATED_START) goto ADC121C021_ReadRaw_Error;
    _delay_us(IC2_COM_DELAY_us);

    // Send address read
    TWDR = (address << 1) | 1;
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_ReadRaw_Error;
    if ((TWSR & 0xF8) != MR_SLA_ACK) goto ADC121C021_ReadRaw_Error;
    _delay_us(IC2_COM_DELAY_us);

    // Read MSB, ACK
    TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_ReadRaw_Error;
    if ((TWSR & 0xF8) != MR_DATA_ACK) goto ADC121C021_ReadRaw_Error;
    msb = TWDR;

    // Read LSB, NACK
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_ReadRaw_Error;
    if ((TWSR & 0xF8) != MR_DATA_NAK) goto ADC121C021_ReadRaw_Error;
    lsb = TWDR;

    // Stop
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    _delay_us(IC2_COM_DELAY_us);

    *raw_out = (((uint16_t)msb << 8) | lsb) & 0x0FFF;
    return 1;

ADC121C021_ReadRaw_Error:
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    _delay_us(IC2_COM_DELAY_us);
    return 0;
}

void ADC121C021_SetConfig(uint8_t address){
	// Set configuration register
	// Send start condition
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Config_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != START) goto ADC121C021_Setup_Config_Sent_Stop; // Check correct status
	_delay_us(IC2_COM_DELAY_us);


	// send address
	TWDR = (address<<1); // Write address
	TWCR = (1<<TWINT)|(1<<TWEN); // Start transmission address
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Config_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto ADC121C021_Setup_Config_Sent_Stop; // Check correct status
	_delay_us(IC2_COM_DELAY_us);

	// send register
	TWDR = ADC121C021_REG_CONFIG;
	TWCR = (1<<TWINT)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Config_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Config_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send data
	TWDR = 0b11100010; // 0.4ksps auto conversion, Alert self clear,Enable alert ,Alert active low
	TWCR = (1<<TWINT)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Config_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Config_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send stop
	ADC121C021_Setup_Config_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);
}

void ADC121C021_Set_Vlow(uint8_t address){

	uint8_t msb;
	uint8_t lsb;
	uint16_t Vlow;
	// Calculate Vhigh threshold
	Vlow = NTC_TempToRaw(TEMP_MAX_Heat_dissipation);
	Vlow &= 0x0FFF;

	lsb = (uint8_t) (Vlow & 0xFF);
	msb = (uint8_t) (Vlow>>8);


	// Set V_low
	// Send start condition
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vlow_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != START) goto ADC121C021_Setup_Vlow_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send address
	TWDR = (address<<1); //Write address
	TWCR = (1<<TWINT)|(1<<TWEN); //Start transmission address
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vlow_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto ADC121C021_Setup_Vlow_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send register
	TWDR = ADC121C021_REG_V_LOW;
	TWCR = (1<<TWINT)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vlow_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vlow_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send high byte
	TWDR = msb;  
	TWCR = (1<<TWINT)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vlow_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vlow_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send low byte
	TWDR = lsb; 
	TWCR = (1<<TWINT)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vlow_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vlow_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send stop
	ADC121C021_Setup_Vlow_Sent_Stop:
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	_delay_us(IC2_COM_DELAY_us);
}

//ADC will deasert the alert pin if the voltage is X amount higher than Vlow or (X higher than Vhigh)
//Where X is the value stored at the hysteresis register
void ADC121C021_Set_Vhyst(uint8_t address){
	// Calculate V Hysteresis
	// Hysteresis is calculated based on 5% below the upper temperature limit
	float temp_hyst; // Temperature where the alert should be turned off
	uint8_t msb;
	uint8_t lsb;
	uint16_t Vhyst;	
	
	
	temp_hyst = TEMP_MAX_Heat_dissipation - TEMP_MAX_Heat_dissipation * 0.05;
	Vhyst = NTC_TempToRaw(temp_hyst);
	Vhyst &= 0x0FFF;

	lsb = (uint8_t) (Vhyst & 0xFF);
	msb = (uint8_t) (Vhyst>>8);


	//Set V_hyst
	//Send start condition
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vhyst_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != START) goto ADC121C021_Setup_Vhyst_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send address
	TWDR = (address<<1) ; //Write address
	TWCR = (1<<TWINT)|(1<<TWEN); //Start transmission address
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vhyst_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_SLA_ACK) goto ADC121C021_Setup_Vhyst_Sent_Stop; //Check correct status
	_delay_us(IC2_COM_DELAY_us);

	//send register
	TWDR = ADC121C021_REG_V_HYST;
	TWCR = (1<<TWINT)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vhyst_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vhyst_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send high byte
	TWDR = msb; 
	TWCR = (1<<TWINT)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vhyst_Sent_Stop; // Wait for start to be sent
	if ((TWSR & 0xF8) != MT_DATA_ACK) goto ADC121C021_Setup_Vhyst_Sent_Stop;
	_delay_us(IC2_COM_DELAY_us);

	// send low byte
	TWDR = lsb; 
	TWCR = (1<<TWINT)|(1<<TWEN);
	if (!I2C_Wait_TWINT()) goto ADC121C021_Setup_Vhyst_Sent_Stop; // Wait for start to be sent
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
        return 99.9;   //Invalid return 99.9
    }

    ratio = (float)raw / 4095.0;

    r_ntc = R_FIXED * ratio / (1.0 - ratio);

    inv_t = 1.0 / 298.15 + log(r_ntc / R_NTC_T0) / NTCALUG0_BETA_K;
    temp_k = 1.0 / inv_t;
    temp_c = temp_k - 273.15;

    return temp_c; 
}


uint16_t NTC_TempToRaw(float temp_c){
	float temp_k;
    float r_ntc;
	float ratio;
	float raw;

	temp_k = temp_c + 273.15;

	r_ntc =  R_NTC_T0 * exp(NTCALUG0_BETA_K * (1/temp_k - 1.0/ 298.15));

	ratio = r_ntc / (r_ntc + R_FIXED);

	raw = 4095 * ratio;
	
	//Clamp result
	if (raw < 0.0) {
        raw = 0.0;
    }
    if (raw > 4095.0) {
        raw = 4095.0;
    }
	return (uint16_t) raw;
} 


//Helper to handle I2C communication and give a timeout if it takes too long
uint8_t I2C_Wait_TWINT(){
    uint16_t timeout = I2C_TIMEOUT_COUNT;

    while (!(TWCR & (1 << TWINT))) {
        if (--timeout == 0) {
            return 0;
        }
    }

    return 1;
}

uint8_t ADC121C021_IsPresent(uint8_t address){
    // Send start
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_IsPresent_Error;
    if ((TWSR & 0xF8) != START) goto ADC121C021_IsPresent_Error;

    // Send address write
    TWDR = (address << 1);
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!I2C_Wait_TWINT()) goto ADC121C021_IsPresent_Error;

    if ((TWSR & 0xF8) != MT_SLA_ACK) {
        goto ADC121C021_IsPresent_Error;
    }

    // Stop
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    _delay_us(IC2_COM_DELAY_us);

    return 1;

	ADC121C021_IsPresent_Error:
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    _delay_us(IC2_COM_DELAY_us);

    return 0;
}