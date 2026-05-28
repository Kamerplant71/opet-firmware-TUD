/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Martin Bliss
	Version:	1.00
	Date:		07.09.2021
	-----------------------------------------
	ATMega 324 Micro controller support
	-----------------------------------------
	     Input Output Header FILE
	=========================================
*/

#ifndef MPPT_PCB_MCU__IO_H_
#define MPPT_PCB_MCU__IO_H_

//======================================================================
// Definitions and constants
#define ADC_MUX_Set_Delay_us 100
#define DAC_MAX_COUNTS 65530
#define ADC_Cycl_Avr_Buff_MAX 101
#define IC2_COM_DELAY_us 10

//===========================================================================================
// EXTERN VARIABLES and STRUCTURES

// AI channels
extern volatile float AI_Volt;
extern volatile float AI_Curr;
extern volatile float AI_Curr_Corr;
extern volatile float AI_Offset;
extern volatile float AI_Bias;
extern volatile float AI_NTC_Temp_1;
extern volatile float AI_NTC_Temp_2;
extern volatile float AI_RTD_Temp;

// AI calibration factors (for PV Curr & Volt see Range)
extern volatile float CAL_Bias_A0;
extern volatile float CAL_Bias_A1;
extern volatile float CAL_RTD_A0;
extern volatile float CAL_RTD_A1;
extern volatile float CAL_RTD_A2;
extern volatile float CAL_NTC_1_InvGain;
extern volatile float CAL_NTC_1_RSer;
extern volatile float CAL_NTC_1_InvR25;
extern volatile float CAL_NTC_1_InvBeta;
extern volatile float CAL_NTC_2_InvGain;
extern volatile float CAL_NTC_2_RSer;
extern volatile float CAL_NTC_2_InvR25;
extern volatile float CAL_NTC_2_InvBeta;

// AI input feedback
extern volatile uint8_t AI_Over_Load;
extern volatile uint8_t AI_Under_Load;

// AO calibration factors
extern volatile float CAL_DAC_A0;
extern volatile float CAL_DAC_A1;

// IO EXpander Variables
extern volatile uint8_t DIOE_PORT_A_STATE;
extern volatile uint8_t DIOE_PORT_B_STATE;

// AI Meas settings
extern volatile uint16_t ADC_Num_Avr_VoltCurr;
extern volatile uint8_t ADC_Cycl_Avr_VoltCurr;
extern volatile uint8_t ADC_Num_Avr_Other;
extern volatile float ADC_Avr_Mult_VoltCurr;
extern volatile float ADC_Avr_Mult_Other;
extern volatile uint32_t ADC_Cycl_Avr_Buff_Volt[ADC_Cycl_Avr_Buff_MAX];
extern volatile uint32_t ADC_Cycl_Avr_Buff_Curr[ADC_Cycl_Avr_Buff_MAX];

//===========================================================================================
// EXTERN EEPROM VARIABLES
extern EEMEM uint16_t EROM_ADC_Num_Avr_VoltCurr;
extern EEMEM uint8_t EROM_ADC_Cycl_Avr_VoltCurr;
extern EEMEM uint8_t EROM_ADC_Num_Avr_Other;
extern EEMEM float EROM_CAL_Bias_A0;
extern EEMEM float EROM_CAL_Bias_A1;
extern EEMEM float EROM_CAL_RTD_A0;
extern EEMEM float EROM_CAL_RTD_A1;
extern EEMEM float EROM_CAL_RTD_A2;
extern EEMEM float EROM_CAL_NTC_1_InvGain;
extern EEMEM float EROM_CAL_NTC_1_RSer;
extern EEMEM float EROM_CAL_NTC_1_InvR25;
extern EEMEM float EROM_CAL_NTC_1_InvBeta;
extern EEMEM float EROM_CAL_NTC_2_InvGain;
extern EEMEM float EROM_CAL_NTC_2_RSer;
extern EEMEM float EROM_CAL_NTC_2_InvR25;
extern EEMEM float EROM_CAL_NTC_2_InvBeta;
extern EEMEM float EROM_CAL_DAC_A0;
extern EEMEM float EROM_CAL_DAC_A1;


//===========================================================================================
// ADC Channel definitions

#define Set_ADC_Mux_Voltage	({\
	CLRBIT(ADC_MUX_A0_port, ADC_MUX_A0_pin);\
	CLRBIT(ADC_MUX_A1_port, ADC_MUX_A1_pin);\
	CLRBIT(ADC_MUX_A2_port, ADC_MUX_A2_pin);})

#define Set_ADC_Mux_Current	({\
	SETBIT(ADC_MUX_A0_port, ADC_MUX_A0_pin);\
	CLRBIT(ADC_MUX_A1_port, ADC_MUX_A1_pin);\
	CLRBIT(ADC_MUX_A2_port, ADC_MUX_A2_pin);})

#define Set_ADC_Mux_Offset	({\
	CLRBIT(ADC_MUX_A0_port, ADC_MUX_A0_pin);\
	SETBIT(ADC_MUX_A1_port, ADC_MUX_A1_pin);\
	CLRBIT(ADC_MUX_A2_port, ADC_MUX_A2_pin);})

#define Set_ADC_Mux_Bias	({\
	SETBIT(ADC_MUX_A0_port, ADC_MUX_A0_pin);\
	SETBIT(ADC_MUX_A1_port, ADC_MUX_A1_pin);\
	CLRBIT(ADC_MUX_A2_port, ADC_MUX_A2_pin);})
	
#define Set_ADC_Mux_NTC_1	({\
	SETBIT(ADC_MUX_A0_port, ADC_MUX_A0_pin);\
	CLRBIT(ADC_MUX_A1_port, ADC_MUX_A1_pin);\
	SETBIT(ADC_MUX_A2_port, ADC_MUX_A2_pin);})

#define Set_ADC_Mux_NTC_2	({\
	CLRBIT(ADC_MUX_A0_port, ADC_MUX_A0_pin);\
	CLRBIT(ADC_MUX_A1_port, ADC_MUX_A1_pin);\
	SETBIT(ADC_MUX_A2_port, ADC_MUX_A2_pin);})

#define Set_ADC_Mux_GND	({\
	SETBIT(ADC_MUX_A0_port, ADC_MUX_A0_pin);\
	SETBIT(ADC_MUX_A1_port, ADC_MUX_A1_pin);\
	SETBIT(ADC_MUX_A2_port, ADC_MUX_A2_pin);})

//===========================================================================================
// IC2 communication bus definitions
#define START 0x08
#define REPEATED_START 0x10
#define MT_SLA_ACK 0x18
#define MT_DATA_ACK 0x28
#define MR_SLA_ACK 0x40
#define MR_DATA_ACK 0x50
#define MR_DATA_NAK 0x58

//===========================================================================================
// DIO Expander Channel definitions (IC2 = PCA9555)
#define DIO_ADR_Write	0b01000000
#define DIO_ADR_Read	0b01000001
#define DIO_REG_DDR		6		// data direction register
#define DIO_REG_GPOI	2		// GPOI pin output access
#define DIO_DATA_DDRA	0b00000000	// all write
#define DIO_DATA_DDRB	0b00000000	// all write

//===========================================================================================
// MCP9600 TC ADC IC2 definitions (IC2 = MPC9600)
#define MCP9600_ADR_Write		0b11000000
#define MCP9600_ADR_Read		0b11000001
#define MCP9600_REG_DEV_Config	0b00000110	// chip and ADC config
#define MCP9600_DATA_DEV_Config	0b00000000	// 18bit mode, no burst, normal operation
#define MCP9600_REG_TC_Config	0b00000101	// thermocouple and filter config
#define MCP9600_DATA_TC_Config	0b00000000	// K-Type and no filter
#define MCP9600_REG_TC_Temp		0b00000000	// measurement result register, compensated data

//===========================================================================================
// FUNCTION Prototypes
void Meas_Analog_Inputs();
void Meas_AI_Volt_Only();
void Meas_AI_Curr_Only();

int16_t ADC_Convert_MAX11163();
uint32_t ADC_Convert_Avr_MAX11163(uint16_t Nu_Avr);
uint32_t ADC_Convert_Avr_OLUL_MAX11163(uint16_t Nu_Avr);

void Set_DAC_Output_Volts(float Voltage);
void Set_DAC_Output_RAW(uint16_t DAC_val);

void DIOExp_config_init();
void DIOExp_Write_Outputs(uint8_t REG_byte, uint8_t DATA_Port_A, uint8_t DATA_Port_B);
uint16_t DIOExp_Read_Ports(uint8_t REG_byte);


void TEMP_MAX31865_Setup();
int32_t TEMP_MAX31865_Measure();
void TEMP_MCP9600_Setup();
float TEMP_MCP9600_Measure();

void TEMP_MAX31856_Setup();
float TEMP_MAX31856_Measure();

//end
#endif /* MPPT_PCB_MCU__IO_H_ */