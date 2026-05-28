/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Justen van Koppen
	Version:	1.00
	Date:		20.05.2026
	-----------------------------------------
	ATMega 1284 Micro controller support
	-----------------------------------------
	      Communication Header FILE
	=========================================
*/

#ifndef MPPT_PCB_MCU__TEMP_H_
#define MPPT_PCB_MCU__TEMP_H_

//===========================================================================================
// Definitions and constants

#define BASE_ADDRESS_ADC121C021 0x50  // 101 0000


//Registers
#define ADC121C021_REG_CONV_RESULT 0x00
#define ADC121C021_REG_CONFIG 0x02
#define ADC121C021_REG_V_HIGH 0x04
#define ADC121C021_REG_V_HYST 0x05

//Vishay NTCALUG01A103J
#define R_FIXED_OHM 10000.0   // fixed divider resistor
#define NTC_R0_OHM  10000.0   // NTC resistance at 25 C
#define NTC_BETA_K  3984.0    // beta value from NTC datasheet

//===========================================================================================
// VARIABLES and STRUCTURES

extern volatile float AI_HEAT_NTC_Temp[8];
extern volatile uint8_t AI_HEAT_ADC_MASK[8];

//===========================================================================================
// EXTERN EEPROM VARIABLES


//===========================================================================================
// FUNCTION Prototypes

void Temp_monitoring_Setup();
void Read_Temp_All_ADC121C021();

void ADC121C021_Setup(uint8_t address);
uint16_t ADC121C021_ReadRaw(uint8_t address);
void ADC121C021_SetConfig(uint8_t address);
void ADC121C021_Set_Vhigh(uint8_t address);
void ADC121C021_Set_Vhyst(uint8_t address);
float NTC_RawToTemp(uint16_t raw);

//end
#endif /* MPPT_PCB_MCU__Com_H_ */