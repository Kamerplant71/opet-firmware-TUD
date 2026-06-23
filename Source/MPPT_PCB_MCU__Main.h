/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Martin Bliss
	Version:	1.12
	Date:		27.02.2024
	-----------------------------------------
	ATMega 324 Micro controller support
	-----------------------------------------
	MAIN Header FILE
	=========================================
*/

#ifndef MPPT_PCB_MCU__Main_H_
#define MPPT_PCB_MCU__Main_H_

// Temp_Sensor type definitions ( 0 is uninitialized)
#define Temp_Sensor__None			0
#define Temp_Sensor__MAX31865		1
#define Temp_Sensor__MCP9600		2
#define Temp_Sensor__MAX31856		3


//===========================================================================================
// BOARD HARDWARE CONFIG DEFINITIONS:
//----------------------------------------
// most of these need to be set before compiling to program the specific board, used for backwards compatibility
//----------------------------------------
#define PCBconfig_Current_is_15A // use <PCBconfig_Current_is_150mA> for up to 150mA OR <PCBconfig_Current_is_15A> for up to 15A
								   // use <PCBconfig_Current_is_340mA> for up to 340mA

#define PCBconfig_DAQ_is_DAQ8550	// use <PCBconfig_DAQ_is_MAX5216> for MAX5216 for PCB rev <=1.3 
									// or <PCBconfig_DAQ_is_DAQ8550> for DAQ8550 for rev 1.4

#define PCBconfig_Vswitch_is_1CL_Relay	// use<PCBconfig_Vswitch_is_MAX4665> for MAX4665 analog range selection switch PCB rev <=1.3 
										// or <PCBconfig_Vswitch_is_1CL_Relay> for 1 coil latch relay for rev 1.4
										
#define PCBconfig_Curr_switch_is_1CL_Relay	// use<PCBconfig_Curr_switch_is_2CL_Relay> for 2 coil latching relay to switch current amp range PCB rev <=1.3
											// or <PCBconfig_Curr_switch_is_1CL_Relay> for 1 coil latch relay for switch current amp range rev 1.4

#define PCBconfig_JTAG_is_disabled // use <PCBconfig_JTAG_is_enabled> JTAG is enabled at controller fuse, else <PCBconfig_JTAG_is_disabled>

#define PCBconfig_TEMP_is_enabled 1 // use <PCBconfig_TEMP_is_enabled 1> to enable temperature sensor readout by default, 
									// or <PCBconfig_TEMP_is_enabled 0> to disable
									// also configured as EEPROM in SysConfig
				
#define PCBconfig_TEMP_Sensor_Type (Temp_Sensor__MAX31856)	// see above table of sensor type definitions to configure connected temperature readout on IOextender header
															// also configured as EEPROM in SysConfig

#define PCBconfig_TEMP_Heat_Monitoring_enabled 1 // use <PCBconfig_TEMP_Heat_Monitoring_enabled> 1 when using the modular power dissipation device otherwise set 0

#define PCBconfig_TEMP_Heat_Monitoring_devices 2 // Number of heat dissipation devices, set to 0 if none   


#define Line_Freq_50		//use <Line_Freq_60> for 60Hz line frequency and <Line_Freq_50> for 50Hz

//===========================================================================================
// Definitions and constants
#define F_CPU 16000000
#ifdef Line_Freq_60
	#define	TIMER_1_COMP_MATCH 1041		//~4.1667ms @16Mhz = 1041.667 // 64 clock divider
	#define CONTROL_TIMER_MULT 6		// 6 = every 25.0ms; 16.67mS/60Hz 4xAVR AI update over a single line cycle, ~ 8ms for settling
	#define TEMP_MEAS_TIMER_MUILT 120	// 96 = ~500ms @ 4.167ms;
#endif /* Line_Freq_60 */
#ifdef Line_Freq_50
	#define	TIMER_1_COMP_MATCH 1249		//~5.0ms:: @ 20Mhz = 1562.5; 16MHz = 1250 (-1); @ 8MHz = (625 + 1?) // 64 clock divider
	#define CONTROL_TIMER_MULT 5		// 5 (6) = every 25.0ms; 20ms/50Hz 4xAVR AI update over a single line cycle, ~ 5ms for settling
	#define TEMP_MEAS_TIMER_MUILT 100	// 100 = ~500ms @ 5ms;
#endif /* Line_Freq_50 */

#define DeviceName "OPET_R1.4C"
#define FirmwareVersion "V1.16A-D08M03Y75"

//===========================================================================================
// INCLUDE Libraries
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <ctype.h>
#include <avr/wdt.h> /*Watchdog timer handling*/

//===========================================================================================
// Programming definitions:
#define BIT(pin) (1 << (pin))
#define SETBIT(port, pin) (port)|=  BIT(pin)
#define CLRBIT(port, pin) (port)&= ~BIT(pin)
#define DO_Nothing asm volatile ("nop" :: )


//===========================================================================================
// MCU Port bit definitions:
//----------------------------------------
//	PortA:
//		Bit0:	Current_Bypass_ON
				#define DI_Curr_Bypass_ON_DDR (CLRBIT(DDRA, 0))
				#define is_DI_Curr_Bypass_ON (!(PINA & BIT(0))) // negated here as high = bypass open
//		Bit1:	ADC MUX A2
				#define ADC_MUX_A2_port PORTA
				#define ADC_MUX_A2_pin 1
				#define ADC_MUX_A2_DDR (SETBIT(DDRA, ADC_MUX_A2_pin))
//		Bit2:	ADC MUX A1
				#define ADC_MUX_A1_port PORTA
				#define ADC_MUX_A1_pin 2
				#define ADC_MUX_A1_DDR (SETBIT(DDRA, ADC_MUX_A1_pin))
//		Bit3:	ADC MUX A0
				#define ADC_MUX_A0_port PORTA
				#define ADC_MUX_A0_pin 3
				#define ADC_MUX_A0_DDR (SETBIT(DDRA, ADC_MUX_A0_pin))
//		Bit4:	EXP_DIO_1
				#define SET__EXP_DIO_1 (SETBIT(PORTA, 4))
				#define CLR__EXP_DIO_1 (CLRBIT(PORTA, 4))
				#define DDR__DO__EXP_DIO_1 (SETBIT(DDRA, 4))
				#define DDR__DI__EXP_DIO_1 (CLRBIT(DDRA, 4))
				#define is_EXP_DIO_1_Set (PINA & BIT(4))
//		Bit5:	EXP_DIO_2
				#define SET__EXP_DIO_2 (SETBIT(PORTA, 5))
				#define CLR__EXP_DIO_2 (CLRBIT(PORTA, 5))
				#define DDR__DO__EXP_DIO_2 (SETBIT(DDRA, 5))
				#define DDR__DI__EXP_DIO_2 (CLRBIT(DDRA, 5))
				#define is_EXP_DIO_2_Set (PINA & BIT(5))
//		Bit6:	DIOP IN2
				#define SET__LRELAY_FAN_Off (SETBIT(PORTA, 6))
				#define CLR__LRELAY_FAN_Off (CLRBIT(PORTA, 6))
				#define DDR__LRELAY_FAN_Off (SETBIT(DDRA, 6))
//		Bit7:	DIOP IN1
				#define SET__LRELAY_FAN_On (SETBIT(PORTA, 7))
				#define CLR__LRELAY_FAN_On (CLRBIT(PORTA, 7))
				#define DDR__LRELAY_FAN_On (SETBIT(DDRA, 7))

//----------------------------------------
//	PortB:
				#define SPI_DDR DDRB
				#define SPI_port PORTB
//		Bit0:	Output Enable
				#define SET__Output_On (SETBIT(PORTB, 0))
				#define CLR__Output_On (CLRBIT(PORTB, 0))
				#define DDR__Output_On (SETBIT(DDRB, 0))
//		Bit1:	CLOCK output, Timer output
				#define TIMER_port PORTB
				#define TIMER_pin 1
				#define TIMER_DDR (SETBIT(DDRA, TIMER_pin))
				#define SET_TIMER_DO (SETBIT(TIMER_port, TIMER_pin))
				#define CLR_TIMER_DO (CLRBIT(TIMER_port, TIMER_pin))
//		Bit2:	ADC conversion start
				#define CS_AI_CNV_port PORTB
				#define CS_AI_CNV_pin 2
				#define CS_AI_CNV_DDR (SETBIT(DDRB, CS_AI_CNV_pin))
//		Bit3:	ADC chip select, current & voltage meas
				#define CS_AI1_port PORTB
				#define CS_AI1_pin 3
				#define CS_AI1_DDR (SETBIT(DDRB, CS_AI1_pin))
//		Bit4:	SPI SS,  PT100 ADC master input clock
				#define CS_AO1_port PORTB
				#define CS_AO1_pin 4
				#define CS_AO1_DDR (SETBIT(DDRB, CS_AO1_pin))
				#define SS_pin 4				
//		Bit5:	SPI DI, Programmer MOSI
				#define MOSI_pin 5
//		Bit6:	SPI DO, Programmer MISO
				#define MISO_pin 6
//		Bit7:	SPI SCK, Programmer SCK
				#define SCK_pin 7
//----------------------------------------
//	PortC:
//		Bit0:	IC2 DIOE-SCL (IC2 REV>1.0), or DRDY_ADC_PT100 (SPI Rev1.0)
				// IC2-SCL		
// 		Bit1:	IC2 DIOE-SDA (IC2 REV>1.0), or CS_ADC_PT100 (SPI Rev1.0)
				// IC2-SDA
//		Bit2:	ADR SET BIT 16
				#define DI_ADR_16_DDR (CLRBIT(DDRC, 2))
				#define is_ADR_16_Set (!(PINC & BIT(2))) // inverted here 
//		Bit3:	ADR SET BIT 4
				#define DI_ADR_4_DDR (CLRBIT(DDRC, 3))
				#define is_ADR_4_Set (!(PINC & BIT(3))) // inverted here 
//		Bit4:	ADR SET BIT 8
				#define DI_ADR_8_DDR (CLRBIT(DDRC, 4))
				#define is_ADR_8_Set (!(PINC & BIT(4))) // inverted here 
//		Bit5:	ADR SET BIT 2
				#define DI_ADR_2_DDR (CLRBIT(DDRC, 5))
				#define is_ADR_2_Set (!(PINC & BIT(5))) // inverted here 
//		Bit6:	ADR SET BIT 1
				#define DI_ADR_1_DDR (CLRBIT(DDRC, 6))
				#define is_ADR_1_Set (!(PINC & BIT(6))) // inverted here 
//		Bit7:	LED_Output_On
				#define SET__LED_Output_On (SETBIT(PORTC, 7))
				#define CLR__LED_Output_On (CLRBIT(PORTC, 7))
				#define DDR__LED_Output_On (SETBIT(DDRC, 7))
				
//----------------------------------------
//	PortD:
//		Bit0:	RXD
//		Bit1:	TXD
//		Bit2:	RS485 device output enabled
				#define RS485_port PORTD
				#define RS485_pin 2
				#define RS485_DDR (SETBIT(DDRD, RS485_pin))
//		Bit3:	none
//		Bit4:	none
//		Bit5:	EXP_DIO_3
				#define SET__EXP_DIO_3 (SETBIT(PORTD, 5))
				#define CLR__EXP_DIO_3 (CLRBIT(PORTD, 5))
				#define DDR__DO__EXP_DIO_3 (SETBIT(DDRD, 5))
				#define DDR__DI__EXP_DIO_3 (CLRBIT(DDRD, 5))
				#define is_EXP_DIO_3_Set (PIND & BIT(5))
//		Bit6:	EXP_DIO_4
				#define SET__EXP_DIO_4 (SETBIT(PORTD, 6))
				#define CLR__EXP_DIO_4 (CLRBIT(PORTD, 6))
				#define DDR__DO__EXP_DIO_4 (SETBIT(DDRD, 6))
				#define DDR__DI__EXP_DIO_4 (CLRBIT(DDRD, 6))
				#define is_EXP_DIO_4_Set (PIND & BIT(6))				
//		Bit7:	LED Error
				#define SET__LED_Error (SETBIT(PORTD, 7))
				#define CLR__LED_Error (CLRBIT(PORTD, 7))
				#define DDR__LED_Error (SETBIT(DDRD, 7))
				
//========================================
// GPIO bit definitions:
//----------------------------------------
//	DIOE_PORT_A_STATE:
//		Bit0:	V_range_Spare
				#define SET__V_range_Spare (SETBIT(DIOE_PORT_A_STATE, 0))
				#define CLR__V_range_Spare (CLRBIT(DIOE_PORT_A_STATE, 0))
//		Bit1:	V_range_10V
				#define SET__V_range_10V (SETBIT(DIOE_PORT_A_STATE, 1))
				#define CLR__V_range_10V (CLRBIT(DIOE_PORT_A_STATE, 1))
//		Bit2:	V_range_100V
				#define SET__V_range_100V (SETBIT(DIOE_PORT_A_STATE, 2))
				#define CLR__V_range_100V (CLRBIT(DIOE_PORT_A_STATE, 2))
//		Bit3:	V_range_30V
				#define SET__V_range_30V (SETBIT(DIOE_PORT_A_STATE, 3))
				#define CLR__V_range_30V (CLRBIT(DIOE_PORT_A_STATE, 3))
//		Bit4:	C_range_mux_A1
				#define SET__C_range_mux_A1 (SETBIT(DIOE_PORT_A_STATE, 4))
				#define CLR__C_range_mux_A1 (CLRBIT(DIOE_PORT_A_STATE, 4))
//		Bit5:	C_range_mux_A0
				#define SET__C_range_mux_A0 (SETBIT(DIOE_PORT_A_STATE, 5))
				#define CLR__C_range_mux_A0 (CLRBIT(DIOE_PORT_A_STATE, 5))
//		Bit6:	C_range_gain_1 (latching relay connected)
				#define SET__C_range_gain_2 (SETBIT(DIOE_PORT_A_STATE, 6))
				#define CLR__C_range_gain_2 (CLRBIT(DIOE_PORT_A_STATE, 6))
//		Bit7:	C_range_gain_2 (latching relay connected)
				#define SET__C_range_gain_1 (SETBIT(DIOE_PORT_A_STATE, 7))
				#define CLR__C_range_gain_1 (CLRBIT(DIOE_PORT_A_STATE, 7))
//----------------------------------------
//	DIOE_PORT_B_STATE:
//		Bit0:	Cm_OR_RST
				#define SET__Cm_OR_RST (SETBIT(DIOE_PORT_B_STATE, 0))
				#define CLR__Cm_OR_RST (CLRBIT(DIOE_PORT_B_STATE, 0))
//		Bit1:	C_range_shunt_1
				#define SET__C_range_shunt_1 (SETBIT(DIOE_PORT_B_STATE, 1))
				#define CLR__C_range_shunt_1 (CLRBIT(DIOE_PORT_B_STATE, 1))
//		Bit2:	C_range_shunt_2
				#define SET__C_range_shunt_2 (SETBIT(DIOE_PORT_B_STATE, 2))
				#define CLR__C_range_shunt_2 (CLRBIT(DIOE_PORT_B_STATE, 2))
//		Bit3:	C_range_shunt_3
				#define SET__C_range_shunt_3 (SETBIT(DIOE_PORT_B_STATE, 3))
				#define CLR__C_range_shunt_3 (CLRBIT(DIOE_PORT_B_STATE, 3))
//		Bit4:	PI_Var_Int_A0
				#define SET__PI_Var_Int_A0 (SETBIT(DIOE_PORT_B_STATE, 4))
				#define CLR__PI_Var_Int_A0 (CLRBIT(DIOE_PORT_B_STATE, 4))
//		Bit5:	PI_Var_Int_A1
				#define SET__PI_Var_Int_A1 (SETBIT(DIOE_PORT_B_STATE, 5))
				#define CLR__PI_Var_Int_A1 (CLRBIT(DIOE_PORT_B_STATE, 5))
//		Bit6:	PI_Var_Prop_A0
				#define SET__PI_Var_Prop_A0 (SETBIT(DIOE_PORT_B_STATE, 6))
				#define CLR__PI_Var_Prop_A0 (CLRBIT(DIOE_PORT_B_STATE, 6))
//		Bit7:	PI_Var_Prop_A1
				#define SET__PI_Var_Prop_A1 (SETBIT(DIOE_PORT_B_STATE, 7))
				#define CLR__PI_Var_Prop_A1 (CLRBIT(DIOE_PORT_B_STATE, 7))

//========================================
//  Status and Mode Bytes bit definitions:
//----------------------------------------
//	SysStatus_A:
//		Bit0:	Output ON
				#define SET__Status_Ouput_On (SETBIT(SysStatus_A, 0))
				#define CLR__Status_Ouput_On (CLRBIT(SysStatus_A, 0))
				#define is_Output_On ((SysStatus_A & BIT(0)))
//		Bit1:	Calibration Mode
				#define SET__Status_Calibrate (SETBIT(SysStatus_A, 1))
				#define CLR__Status_Calibrate (CLRBIT(SysStatus_A, 1))
				#define is_Status_Calibrate (SysStatus_A & BIT(1))
//		Bit2:	Voltage Input Error
				#define SET__Status_Voltage_Error (SETBIT(SysStatus_A, 2))
				#define CLR__Status_Voltage_Error (CLRBIT(SysStatus_A, 2))
				#define is_Status_Voltage_Error (SysStatus_A & BIT(2))
//		Bit3:	Current Input Error
				#define SET__Status_Current_Error (SETBIT(SysStatus_A, 3))
				#define CLR__Status_Current_Error (CLRBIT(SysStatus_A, 3))
				#define is_Status_Current_Error (SysStatus_A & BIT(3))
//		Bit4:	Over Current Clamp ON
				#define SET__Status_Curr_Bypass_ON (SETBIT(SysStatus_A, 4))
				#define CLR__Status_Curr_Bypass_ON (CLRBIT(SysStatus_A, 4))
				#define is_Status_Curr_Bypass_ON (SysStatus_A & BIT(4))
//		Bit5:	Bias Voltage Error
				#define SET__Status_Bias_Error (SETBIT(SysStatus_A, 5))
				#define CLR__Status_Bias_Error (CLRBIT(SysStatus_A, 5))
				#define is_Status_Bias_Error (SysStatus_A & BIT(5))
//		Bit6:	NTC 1 Over Temperature
				#define SET__Status_NTC1_Over_Temp (SETBIT(SysStatus_A, 6))
				#define CLR__Status_NTC1_Over_Temp (CLRBIT(SysStatus_A, 6))
				#define is_Status_NTC1_Over_Temp (SysStatus_A & BIT(6))
//		Bit7:	NTC 2 Over Temperature
				#define SET__Status_NTC2_Over_Temp (SETBIT(SysStatus_A, 7))
				#define CLR__Status_NTC2_Over_Temp (CLRBIT(SysStatus_A, 7))
				#define is_Status_NTC2_Over_Temp (SysStatus_A & BIT(7))
//----------------------------------------
//	SysStatus_B:
//		Bit0:	Main Timer Over Run
				#define SET__Status_MainTimerOverRun (SETBIT(SysStatus_B, 0))
				#define CLR__Status_MainTimerOverRun (CLRBIT(SysStatus_B, 0))
				#define is_MainTimerOverRun ((SysStatus_B & BIT(0)))
//		Bit1:	New IV data available
				#define SET__Status_NewIvDataAvailable (SETBIT(SysStatus_B, 1))
				#define CLR__Status_NewIvDataAvailable (CLRBIT(SysStatus_B, 1))
				#define is_NewIvDataAvailable ((SysStatus_B & BIT(1)))
//		Bit2:	Volt Range Hold Up
				#define SET__Status_RangeVoltHoldUp (SETBIT(SysStatus_B, 2))
				#define CLR__Status_RangeVoltHoldUp (CLRBIT(SysStatus_B, 2))
				#define is_RangeVoltHoldUp ((SysStatus_B & BIT(2)))
//		Bit3:	Curr Range Hold Up
				#define SET__Status_RangeCurrHoldUp (SETBIT(SysStatus_B, 3))
				#define CLR__Status_RangeCurrHoldUp (CLRBIT(SysStatus_B, 3))
				#define is_RangeCurrHoldUp ((SysStatus_B & BIT(3)))
//		Bit4:	Heat dissipation device Temperature status
				#define SET__Status_HEAT_NTC_Over_Temp (SETBIT(SysStatus_B, 4))
				#define CLR__Status_HEAT_NTC_Over_Temp (CLRBIT(SysStatus_B, 4))
				#define is_Status_HEAT_NTC_Over_Temp (SysStatus_B & BIT(4))
//		Bit5:	Heat dissipation device alert pin active
				#define SET__Status_HEAT_NTC_Alert (SETBIT(SysStatus_B, 5))
				#define CLR__Status_HEAT_NTC_Alert (CLRBIT(SysStatus_B, 5))
				#define is_Status_HEAT_NTC_Alert (SysStatus_B & BIT(5))
//		Bit6:	Heat dissipation device offline
				#define SET__Status_HEAT_NTC_offline (SETBIT(SysStatus_B, 6))
				#define CLR__Status_HEAT_NTC_offline (CLRBIT(SysStatus_B, 6))
				#define is_Status_HEAT_NTC_offline (SysStatus_B & BIT(6))
//		Bit7:	none
	
//----------------------------------------
//	SysControl:
//		Bit0:	Control Output Enable (0 - Off; 1 On)
				#define SET__SysControl_Ouput_On (SETBIT(SysControl, 0))
				#define CLR__SysControl_Ouput_On (CLRBIT(SysControl, 0))
				#define is_SysControl_Ouput_On ((SysControl & BIT(0)))
//		Bit1:	Control IV meas (0 - Off; 1 On)
				#define SET__SysControl_IV_Meas (SETBIT(SysControl, 1))
				#define CLR__SysControl_IV_Meas (CLRBIT(SysControl, 1))
				#define is_SysControl_IV_Meas ((SysControl & BIT(1)))
//		Bit2:	Control Transient meas (0 - Off; 1 On)
				#define SET__SysControl_Trans_Meas (SETBIT(SysControl, 2))
				#define CLR__SysControl_Trans_Meas (CLRBIT(SysControl, 2))
				#define is_SysControl_Trans_Meas ((SysControl & BIT(2)))
//		Bit3:	none
//		Bit4:	none
//		Bit5:	none
//		Bit6:	none
//		Bit7:	Control Calibration Mode (0 - Off; 1 On)
				#define SET__SysControl_Cal_Mode (SETBIT(SysControl, 7))
				#define CLR__SysControl_Cal_Mode (CLRBIT(SysControl, 7))
				#define is_SysControl_Cal_Mode ((SysControl & BIT(7)))
//----------------------------------------
//	Timer_CTR:
//		Bit0:	Measurement Start Flag
				#define SET__Timer_CTR_Main_Flag (SETBIT(Timer_CTR, 0))
				#define CLR__Timer_CTR_Main_Flag (CLRBIT(Timer_CTR, 0))
				#define is_Timer_CTR_Main_Flag_On ((Timer_CTR & BIT(0)))
//		Bit1:	Control Start Flag
				#define SET__Timer_CTR_Control_Flag (SETBIT(Timer_CTR, 1))
				#define CLR__Timer_CTR_Control_Flag (CLRBIT(Timer_CTR, 1))
				#define is_Timer_CTR_Control_Flag_On ((Timer_CTR & BIT(1)))
//		Bit2:	none
//		Bit3:	Temperature Measurement  Flag
				#define SET__Timer_TEMP_MEAS_Flag (SETBIT(Timer_CTR, 3))
				#define CLR__Timer_TEMP_MEAS_Flag (CLRBIT(Timer_CTR, 3))
				#define is_Timer_RTD_MEAS_Flag_On ((Timer_CTR & BIT(3)))
//		Bit4:	none
//		Bit5:	none
//		Bit6:	none
//		Bit7:	none
//----------------------------------------
//	UartStatus:
//		Bit0:	none
//		Bit1:	none
//		Bit2:	none
//		Bit3:	none
//		Bit4:	none
//		Bit5:	none
//		Bit6:	IV Data Transfer indicator
				#define SET__COM_Status_IV_Data_Transfer (SETBIT(UartStatus, 6))
				#define CLR__COM_Status_IV_Data_Transfer (CLRBIT(UartStatus, 6))
				#define is_COM_Status_IV_Data_Transfer_On ((UartStatus & BIT(6)))
//		Bit7:	IV Buffer ready indicator
				#define SET__COM_Status_IV_Buffer_Ready (SETBIT(UartStatus, 7))
				#define CLR__COM_Status_IV_Buffer_Ready (CLRBIT(UartStatus, 7))
				#define is_COM_Status_IV_Buffer_Ready_On ((UartStatus & BIT(7)))
//----------------------------------------
//	SysConfig:
//		Bit0:	Temp - Sensor readout enabled
				#define is_SysConfig_TEMP_On ((SysConfig & BIT(0)))
//		Bit1:	Temp - RTD on SPI MAX31865
				#define is_SysConfig_TEMP_MAX31865 ((SysConfig & BIT(1)))
//		Bit2:	Temp - Power dissipation device
				#define SET__SysConfig_TEMP_Heat_Monitoring (SETBIT(SysConfig, 2))
				#define CLR__SysConfig_TEMP_Heat_Monitoring (CLRBIT(SysConfig, 2))
				#define is_SysConfig_TEMP_Heat_Monitoring_On ((SysConfig & BIT(2)))
//		Bit3:	none
//		Bit4:	none
//		Bit5:	none
//		Bit6:	none
//		Bit7:	none
	

//Temperature alert for heat dissipation device
#define is_ADC121C021_ALERT_Active   (!is_EXP_DIO_2_Set) //Active low configuration


//===========================================================================================
// EXTERN VARIABLES and STRUCTURES
extern volatile uint8_t SysStatus_A;
extern volatile uint8_t SysStatus_B;
extern volatile uint8_t SysControl;
extern volatile uint8_t Timer_CTR;
extern volatile uint8_t UartStatus;
extern volatile uint8_t PV_Status;
extern volatile uint8_t SysConfig;
extern volatile uint8_t Temp_Sensor_Type;
extern volatile uint8_t EEMEM_Written;
extern volatile uint8_t Timer_Control_Match;
extern volatile uint8_t Timer_Temp_Meas_Match;
extern volatile float TEMP_MAX_Heat_NTC;  
extern volatile uint8_t TEMP_Heat_dissipation_devices;


//===========================================================================================
// EXTERN EEPROM VARIABLES
extern EEMEM uint16_t EROM_Loop_Timer_1_Comp_Match;
extern EEMEM uint8_t EROM_Timer_Control_Match;
extern EEMEM uint8_t EROM_Timer_Temp_Meas_Match;
extern EEMEM uint8_t EROM_SysConfig;
extern EEMEM uint8_t EROM_SysControl;
extern EEMEM uint8_t EROM_Temp_Sensor_Type;
extern EEMEM float EROM_TEMP_MAX_Heat_NTC; 
extern EEMEM uint8_t EROM_TEMP_Heat_dissipation_devices;

//===========================================================================================
// FUNCTION Prototypes
void Set_DDR_and_Default_State();
void RS485_get_com_address_external_pins();

//end
#endif /* MPPT_PCB_MCU__Main_H_ */