/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Martin Bliss
	Version:	1.16
	Date:		08.03.2024
	-----------------------------------------
	ATMega 128 Micro controller support
	-----------------------------------------
	   Range Selection C-code file
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
// MEMSPACE DATA

//PROGMEM  const uint16_t SINE_DATA[] = {0,1,3}

//===========================================================================================
// VARIABLES and STRUCTURES

// Range Control Values
volatile uint8_t Range_Volt_ID_Act; // range ID integer (0:1V to 5:100V)
volatile float Range_Volt_Val_Act;  // range value in V
volatile uint8_t Range_Volt_ID_Man;
volatile uint8_t Range_Volt_Enable;	// if byte number associated with range number is true, range is enabled
volatile uint8_t Range_Curr_ID_Act; // range ID integer (0:0.5mA to 5:150mA) or(0:50mA to 5:15A)
volatile float Range_Curr_Val_Act;  // range value in A
volatile uint8_t Range_Curr_ID_Man;
volatile uint8_t Range_Curr_Enable;
volatile float Threshold_Volt_Over_Range[Volt_range_max_ID+1];
volatile float Threshold_Volt_Under_Range[Volt_range_max_ID+1];
volatile float Threshold_Curr_Over_Range[Curr_range_max_ID+1];
volatile float Threshold_Curr_Under_Range[Curr_range_max_ID+1];
volatile uint16_t Range_Curr_Switch_Freq_Counter;
volatile uint16_t Range_Curr_Switch_Freq_Ctr_Max;
volatile uint16_t Range_Volt_Switch_Freq_Counter;
volatile uint16_t Range_Volt_Switch_Freq_Ctr_Max;
volatile uint8_t Range_Curr_Switch_Delay_Counter;
volatile uint8_t Range_Curr_Switch_Delay_Ctr_Max;
volatile uint8_t Range_Volt_Switch_Delay_Counter;
volatile uint8_t Range_Volt_Switch_Delay_Ctr_Max;

// Voltage Range 
volatile float AI_Volt_Offset;
volatile float AI_Volt_Scale;
volatile float AI_Volt_R_Leak;
volatile float AI_Volt_Range_Val;

// Current Range
volatile float AI_Curr_Offset;
volatile float AI_Curr_Scale;
volatile float AI_Curr_Range_Val;

// Range Status
volatile uint8_t Range_Status_A;
volatile uint8_t Range_Status_B;


//===========================================================================================
// EEPROM VARIABLES

// Range control Variables
EEMEM uint8_t EROM_Range_Status_A = 0;
EEMEM uint8_t EROM_Range_Volt_ID_Man = RANGE_ID_AUTORANGE;
EEMEM uint8_t EROM_Range_Curr_ID_Man = RANGE_ID_AUTORANGE;
EEMEM uint8_t EROM_Range_Volt_Enable = 0b11111111; // if byte number associated with range number is true, range is enabled
EEMEM uint8_t EROM_Range_Curr_Enable = 0b11111111;
EEMEM uint16_t EROM_Range_Curr_Switch_Freq_Ctr_Max = 300;
EEMEM uint16_t EROM_Range_Volt_Switch_Freq_Ctr_Max = 300;
EEMEM uint8_t EROM_Range_Curr_Switch_Delay_Ctr_Max = 150;
EEMEM uint8_t EROM_Range_Volt_Switch_Delay_Ctr_Max = 150;

// Voltage Calibration values
EEMEM float EROM_V_R1_A0 = -648.9;		// offset value in reading counts
EEMEM float EROM_V_R1_A1 = 6.87500E-05;		// scale value multiplier to Volts
EEMEM float EROM_V_R1_Val_Nom = 1.0;	// nominal range value in Volts
EEMEM float EROM_V_R1_R_Leak = 9.09091E-07;	// effective leakage due to resistance in [A/V] (used for current loss correction)

EEMEM float EROM_V_R2_A0 = -648.9;
EEMEM float EROM_V_R2_A1 = 6.87500E-05;
EEMEM float EROM_V_R2_Val_Nom = 4.2;
EEMEM float EROM_V_R2_R_Leak = 9.09091E-07;

EEMEM float EROM_V_R3_A0 = -648.9;
EEMEM float EROM_V_R3_A1 = 1.60527E-04;
EEMEM float EROM_V_R3_Val_Nom = 10.0;
EEMEM float EROM_V_R3_R_Leak = 6.10657E-06;

EEMEM float EROM_V_R4_A0 = -648.9;
EEMEM float EROM_V_R4_A1 = 4.74594E-04;
EEMEM float EROM_V_R4_Val_Nom = 30.0;
EEMEM float EROM_V_R4_R_Leak = 8.68309E-06;

EEMEM float EROM_V_R5_A0 = -648.9;
EEMEM float EROM_V_R5_A1 = 1.58574E-03;
EEMEM float EROM_V_R5_Val_Nom = 100.0;
EEMEM float EROM_V_R5_R_Leak = 9.60586E-06;


// Current Calibration values (see  board config definitions section in Main.h)
// 150mA Low Current Boards
#ifdef PCBconfig_Current_is_150mA

	EEMEM float EROM_C_R1_A0 = -648.9;		// offset value in reading counts
	EEMEM float EROM_C_R1_A1 = 8.16128E-09;		// scale value multiplier to Amps
	EEMEM float EROM_C_R1_Val_Nom = 0.0005;	// nominal range value in Amps

	EEMEM float EROM_C_R2_A0 = -648.9;
	EEMEM float EROM_C_R2_A1 = 2.42664E-08;
	EEMEM float EROM_C_R2_Val_Nom = 0.0015;

	EEMEM float EROM_C_R3_A0 = -648.9;
	EEMEM float EROM_C_R3_A1 = 8.16128E-08;
	EEMEM float EROM_C_R3_Val_Nom = 0.005;

	EEMEM float EROM_C_R4_A0 = -648.9;
	EEMEM float EROM_C_R4_A1 = 2.42664E-07;
	EEMEM float EROM_C_R4_Val_Nom = 0.015;

	EEMEM float EROM_C_R5_A0 = -648.9;
	EEMEM float EROM_C_R5_A1 = 8.14496E-07;
	EEMEM float EROM_C_R5_Val_Nom = 0.05;

	EEMEM float EROM_C_R6_A0 = -648.9;
	EEMEM float EROM_C_R6_A1 = 2.42179E-06;
	EEMEM float EROM_C_R6_Val_Nom = 0.15;

#endif /* PCBconfig_Current_is_150mA */

// 340mA Medium Current Boards
#ifdef PCBconfig_Current_is_340mA

EEMEM float EROM_C_R1_A0 = -648.9;		// offset value in reading counts
EEMEM float EROM_C_R1_A1 = 1.75538E-08;		// scale value multiplier to Amps
EEMEM float EROM_C_R1_Val_Nom = 0.00107;	// nominal range value in Amps

EEMEM float EROM_C_R2_A0 = -648.9;
EEMEM float EROM_C_R2_A1 = 5.21937E-08;
EEMEM float EROM_C_R2_Val_Nom = 0.0032; // is 3.2mA, no adjustment made here

EEMEM float EROM_C_R3_A0 = -648.9;
EEMEM float EROM_C_R3_A1 = 1.80198E-07;
EEMEM float EROM_C_R3_Val_Nom = 0.011;

EEMEM float EROM_C_R4_A0 = -648.9;
EEMEM float EROM_C_R4_A1 = 5.35794E-07;
EEMEM float EROM_C_R4_Val_Nom = 0.032; // is 33mA, but use top of range on calibrator for better cal accuracy

EEMEM float EROM_C_R5_A0 = -648.9;
EEMEM float EROM_C_R5_A1 = 1.85113E-06;
EEMEM float EROM_C_R5_Val_Nom = 0.11;

EEMEM float EROM_C_R6_A0 = -648.9;
EEMEM float EROM_C_R6_A1 = 5.50406E-06;
EEMEM float EROM_C_R6_Val_Nom = 0.32; // is 340mA, but use top of range on calibrator for better cal accuracy

#endif /* PCBconfig_Current_is_340mA */

// 15A Hight Current Boards
#ifdef PCBconfig_Current_is_15A

	EEMEM float EROM_C_R1_A0 = -648.9;		// offset value in reading counts
	EEMEM float EROM_C_R1_A1 = 8.14496E-07;		// scale value multiplier to Amps
	EEMEM float EROM_C_R1_Val_Nom = 0.05;	// nominal range value in Amps

	EEMEM float EROM_C_R2_A0 = -648.9;
	EEMEM float EROM_C_R2_A1 = 2.42179E-06;
	EEMEM float EROM_C_R2_Val_Nom = 0.15;

	EEMEM float EROM_C_R3_A0 = -648.9;
	EEMEM float EROM_C_R3_A1 = 8.14496E-06;
	EEMEM float EROM_C_R3_Val_Nom = 0.5;

	EEMEM float EROM_C_R4_A0 = -648.9;
	EEMEM float EROM_C_R4_A1 = 2.42179E-05;
	EEMEM float EROM_C_R4_Val_Nom = 1.5;

	EEMEM float EROM_C_R5_A0 = -648.9;
	EEMEM float EROM_C_R5_A1 = 8.14496E-05;
	EEMEM float EROM_C_R5_Val_Nom = 5.0;

	EEMEM float EROM_C_R6_A0 = -648.9;
	EEMEM float EROM_C_R6_A1 = 2.42179E-04;
	EEMEM float EROM_C_R6_Val_Nom = 15.0;

#endif /* PCBconfig_Current_is_15A */

// ggTODO use a time to reduce range switching rates (especially for current to keep the relay alive)

//===========================================================================================
// FUNCTIONS
//===========================================================================================
//-------------------------------------------------------------------------------------------
// Process Input Range Selection
void Check_Input_Range_Selection(){
	/* start checking that the ranges are within adequate boundaries
	- also deals with the over current clamp status
	- the range switch counters reduce high frequency switching in case of instability or other failures*/
	
	// count down voltage range switch frequency counter, if not at zero already
	if (Range_Volt_Switch_Freq_Counter > 0) {
		Range_Volt_Switch_Freq_Counter--;
		SET__Status_RangeVoltHoldUp;
	}
	else CLR__Status_RangeVoltHoldUp;
	// count down current range switch frequency counter, if not at zero already
	if (Range_Curr_Switch_Freq_Counter > 0) {
		Range_Curr_Switch_Freq_Counter--;
		SET__Status_RangeCurrHoldUp;
	}
	else CLR__Status_RangeCurrHoldUp;
		
	// Voltage Measurement Range
	if (is_SysControl_Cal_Mode && is_R_Stat_Cal_Mode_Volt_Act) {
		// Voltage calibration mode
		SET__R_Stat_Volt_R_Manual;
	}
	if (is_manual_Volt_Range_Set) { // Manual voltage range control
		if(Range_Volt_ID_Act != Range_Volt_ID_Man){
			SET_VOLTAGE_RANGE(Range_Volt_ID_Man);
		}
	}
	else if (is_Output_On) { // Output is On, Auto-Range - && !Check_if_Active_Errors_Output_Disabled() removed as range down switching disabled
		Process_Voltage_Auto_Range();
	}
	else { // Output is OFF
		Range_Volt_Switch_Freq_Counter = Range_Volt_Switch_Freq_Ctr_Max; // stop range switching when enabling output for delay time
		if (Range_Volt_ID_Act != Volt_range_max_ID) {
			SET_VOLTAGE_RANGE(Volt_range_max_ID);
		}
	}
		
	// Current Measurement Range
	// Check over current indicator bit
	if(is_DI_Curr_Bypass_ON) { // over current clamp active & bypassing shunts
		SET__Status_Curr_Bypass_ON;
	}
	else {	// over current clamp disabled & shunts OK
		CLR__Status_Curr_Bypass_ON; 
	}	
	// check current range settings
	if (is_SysControl_Cal_Mode && is_R_Stat_Cal_Mode_Curr_Act) {
		// Current calibration mode
		SET__R_Stat_Curr_R_Manual;
	}
	if (is_manual_Curr_Range_Set) { // Manual voltage range control
		if(Range_Curr_ID_Act != Range_Curr_ID_Man){
			SET_CURRENT_RANGE(Range_Curr_ID_Man);
		}
	}
	else if (is_Output_On) { // Output is On, Auto-Range -  - && !Check_if_Active_Errors_Output_Disabled() removed as range down switching disabled
		Process_Current_Auto_Range();
	}
	else { // Output is OFF
		Range_Curr_Switch_Freq_Counter = Range_Curr_Switch_Freq_Ctr_Max; // stop range switching when enabling output for delay time
		SET__Status_RangeCurrHoldUp;
		if (Range_Curr_ID_Act != Curr_range_max_ID) {
			SET_CURRENT_RANGE(Curr_range_max_ID);
		}
	}
		
}

//-------------------------------------------------------------------------------------------
// Process Voltage Auto Range
void Process_Voltage_Auto_Range(){
	/* this function checks if the actual range selected is adequate for the control output and measured input signal
	- Volt measurement range can be above what is controlled at the DAC in cases of noise and at range boundary
	- if DAC request is above voltage measurement range it will go up one by one until its there
	- DAC controlled V-range is good but not ideal either... */
	
	// store new range
	uint8_t New_Range = Range_Volt_ID_Act;
	
	if (is_Volt_Over_Load) {
		// voltage overload during at least one multi-measurement
		New_Range ++;
	}
	else {	// signal is not out of range
		// calc most suitable range
		New_Range = Get_Best_Voltage_Range_From_Value(AI_Volt);
	}
	// check if requested range is enabled and select next up if not
	New_Range = Check_Voltage_Range_Enabled_Correct(New_Range);
	// make sure this is not going out of boundaries
	if (New_Range > Volt_range_max_ID){
		// reached range is at maximum available
		New_Range = Volt_range_max_ID;
	}
	// select new range if different to the old one
	if (New_Range > Range_Volt_ID_Act) {
		// Switch range up after delay even if switching frequency counter is not reset
		if (Range_Volt_Switch_Delay_Counter > 0) Range_Volt_Switch_Delay_Counter--; // delay switching
		else SET_VOLTAGE_RANGE(New_Range);
	}
	else if (New_Range < Range_Volt_ID_Act) {
		// only set a lower range if the counter has expired!
		if (!is_RangeVoltHoldUp) {
			if (Range_Volt_Switch_Delay_Counter > 0) Range_Volt_Switch_Delay_Counter--; // delay switching
			else SET_VOLTAGE_RANGE(New_Range);
		}
	}
	
}

//-------------------------------------------------------------------------------------------
// Process Current Auto Range
void Process_Current_Auto_Range(){
	/* this function checks if the actual range selected is adequate for the measured input signal */
	
	// store new range
	uint8_t New_Range = Range_Curr_ID_Act;
			
	// Check over current indicator bit (here again in case of IV or transient measurements)
	if(is_DI_Curr_Bypass_ON) { // over current bypass active & bypassing shunt current
		SET__Status_Curr_Bypass_ON;
	}
	else {	// over current bypass disabled & shunts OK
		CLR__Status_Curr_Bypass_ON;
	}
	// check range
	if (is_Curr_Over_Load || is_Status_Curr_Bypass_ON) {	// Range setting is too low, got up
		// current overload during at least one multi-measurement or current bypass clamp is set and 
		if ((New_Range + 2) <= Curr_range_max_ID){
			// if act range is low enough switch 2 at a time to reduce relay actions and wear out
			// it'll readjust if needed in the next round...
			New_Range = New_Range + 2;
		}
		else {
			// act range near max so just go up one
			New_Range = New_Range + 1;
		}
		
		// reset over range state
		if (is_Status_Curr_Bypass_ON){
			// clear current over-range state at start-up
			Reset_Over_Current_Clamp();
		}
	}
	else {	// signal is not out of range
		// calc most suitable range
		New_Range = Get_Best_Current_Range_From_Value(AI_Curr);
	}
	// check if requested range is enabled and select next up if not
	New_Range = Check_Current_Range_Enabled_Correct(New_Range);
	// make sure this is not going out of boundaries
	if (New_Range > Curr_range_max_ID){
		// reached range is at maximum available
		New_Range = Curr_range_max_ID;
	}
	// select new range if different to the old one
	if (New_Range > Range_Curr_ID_Act) {
		// Switch range up after delay even if switching frequency counter is not reset
		if (Range_Curr_Switch_Delay_Counter > 0) Range_Curr_Switch_Delay_Counter--; // delay switching
		else SET_CURRENT_RANGE(New_Range);
	}
	else if (New_Range < Range_Curr_ID_Act) {
		// only set a lower range if the counter has expired!
		if (!is_RangeCurrHoldUp) {
			if (Range_Curr_Switch_Delay_Counter > 0) Range_Curr_Switch_Delay_Counter--; // delay switching
			else SET_CURRENT_RANGE(New_Range);
		}
	}
	else Range_Curr_Switch_Delay_Counter = Range_Curr_Switch_Delay_Ctr_Max; // reset delay counter if range OK

}

//-------------------------------------------------------------------------------------------
// Get optimal voltage range based on value
uint8_t Get_Best_Voltage_Range_From_Value(float Value) {
	/* returns the best voltage range based on the value given 
	- if the new range is adjacent to the active voltage range the range overlap zone is checked
	to prohibit changing range too often */
	
	uint8_t i, New_Range;
	i = Volt_range_max_ID + 1; // + 1 gets to zero & would not process range 0 otherwise....
	New_Range = Volt_range_max_ID;
	// loop through all ranges top to bottom
	while(i){
		// if value within max & min
		if ((Value < Threshold_Volt_Over_Range[i-1]) && (Value > Threshold_Volt_Under_Range[i-1])) {
			// Value within Range, use & break out
			New_Range = i-1;
			break;
		}
		i--;
	}
	// check if the new range is adjacent to ACT range and do not switch if within overlap zone (0.9 - 1.03 of scale)
	if (New_Range == (Range_Volt_ID_Act + 1)){
		// new range is one larger than old one
		if (Value < Threshold_Volt_Over_Range[Range_Volt_ID_Act]) {
			// value not above the top threshold to change range upwards (scale x1.03)
			New_Range = Range_Volt_ID_Act;
		}
	}
	else if (New_Range == (Range_Volt_ID_Act -1)) {
		// new range is one smaller than old one
		if (Value > Threshold_Volt_Under_Range[Range_Volt_ID_Act]) {
			// value not below the lower threshold to change range downwards (scale x 0.9)
			// ... case this should not happen as the top while loop has this checked...
			// ... leave it just in case ...
			New_Range = Range_Volt_ID_Act;
		}
	}
	// Return that
	return New_Range;
	
}

//-------------------------------------------------------------------------------------------
// Get optimal current range based on value
uint8_t Get_Best_Current_Range_From_Value(float Value) {
	/* returns the best current range based on the value given 
	- if the new range is adjacent to the active current range the range overlap zone is checked
	to prohibit changing range too often */
	
	uint8_t i, New_Range;
	i = Curr_range_max_ID + 1; // + 1 gets to zero & would not process range 0 otherwise....
	New_Range = Curr_range_max_ID;
	// loop through all ranges top to bottom
	while(i){
		// if value within max & min
		if ((Value < Threshold_Curr_Over_Range[i-1]) && (Value > Threshold_Curr_Under_Range[i-1])) {
			// Value within Range, use & break out
			New_Range = i-1;
			break;
		}
		i--;
	}
	// check if the new range is adjacent to ACT range and do not switch if within overlap zone (0.9 - 1.03 of scale)
	if (New_Range == (Range_Curr_ID_Act + 1)){
		// new range is one larger than old one
		if (Value < Threshold_Curr_Over_Range[Range_Curr_ID_Act]) {
			// value not above the top threshold to change range upwards (scale x1.03)
			New_Range = Range_Curr_ID_Act;
		}
	}
	else if (New_Range == (Range_Curr_ID_Act -1)) {
		// new range is one smaller than old one
		if (Value > Threshold_Curr_Under_Range[Range_Curr_ID_Act]) {
			// value not below the lower threshold to change range downwards (scale x 0.9)
			// ... case this should not happen as the top while loop has this checked...
			// ... leave it just in case ...
			New_Range = Range_Curr_ID_Act;
		}
	}
	// Return that
	return New_Range;
	
}


//-------------------------------------------------------------------------------------------
// Reset Current Over-Range
void Reset_Over_Current_Clamp(){
	/* this resets the current over range protection mechanism 
	- it resets the flip-flop with a clock cycle */
	
	// set OC reset line on GPIO-Ext
	SET__Cm_OR_RST;
	// Transfer new DO line state to GPIOE
	DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
	// Wait a bit
	_delay_us(100);	
	// clear OC reset line on GPIO-Ext
	CLR__Cm_OR_RST;
	// Transfer new DO line state to GPIOE
	DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
	// Wait a bit
	_delay_us(100);
		
}


//-------------------------------------------------------------------------------------------
// Prepare Auto Range Selection
void Preare_Auto_Range_Selection(){
	/* this piece of artwork prepares the range control variables for the auto range selection functions
	 - calculates thresholds at which ranges go up or down
	 - resets the counters to limit range adjustment frequency*/
	float Volt_Range_Values[Volt_range_max_ID];
	float Curr_Range_Values[Curr_range_max_ID];
	
	// reset counters to 0 - meaning allow switching range immediately
	Range_Volt_Switch_Freq_Counter = 0;
	Range_Curr_Switch_Freq_Counter = 0;
	
	// get nominal voltage range value array
	Volt_Range_Values[0] = EEPROM_READ_FLOAT(&EROM_V_R1_A1) * (ADC_Range_Norm_Scale_Count/4.2);
	Volt_Range_Values[1] = EEPROM_READ_FLOAT(&EROM_V_R2_A1) * ADC_Range_Norm_Scale_Count;
	Volt_Range_Values[2] = EEPROM_READ_FLOAT(&EROM_V_R3_A1) * ADC_Range_Norm_Scale_Count;
	Volt_Range_Values[3] = EEPROM_READ_FLOAT(&EROM_V_R4_A1) * ADC_Range_Norm_Scale_Count;
	Volt_Range_Values[4] = EEPROM_READ_FLOAT(&EROM_V_R5_A1) * ADC_Range_Norm_Scale_Count;
	
	// get nominal current range value array
	Curr_Range_Values[0] = EEPROM_READ_FLOAT(&EROM_C_R1_A1) * ADC_Range_Norm_Scale_Count;
	Curr_Range_Values[1] = EEPROM_READ_FLOAT(&EROM_C_R2_A1) * ADC_Range_Norm_Scale_Count;
	Curr_Range_Values[2] = EEPROM_READ_FLOAT(&EROM_C_R3_A1) * ADC_Range_Norm_Scale_Count;
	Curr_Range_Values[3] = EEPROM_READ_FLOAT(&EROM_C_R4_A1) * ADC_Range_Norm_Scale_Count;
	Curr_Range_Values[4] = EEPROM_READ_FLOAT(&EROM_C_R5_A1) * ADC_Range_Norm_Scale_Count;
	Curr_Range_Values[5] = EEPROM_READ_FLOAT(&EROM_C_R6_A1) * ADC_Range_Norm_Scale_Count;
		
	// calculate Threshold_Volt_Over_Range
	Threshold_Volt_Over_Range[0] = Volt_Range_Values[0] * Over_Range_Threshold_Fact;
	Threshold_Volt_Over_Range[1] = Volt_Range_Values[1] * Over_Range_Threshold_Fact;
	Threshold_Volt_Over_Range[2] = Volt_Range_Values[2] * Over_Range_Threshold_Fact;
	Threshold_Volt_Over_Range[3] = Volt_Range_Values[3] * Over_Range_Threshold_Fact;
	Threshold_Volt_Over_Range[4] = Volt_Range_Values[4] * Over_Range_Threshold_Fact;
	
	// calculate Threshold_Volt_Under_Range
	Threshold_Volt_Under_Range[0] = Volt_Range_Values[0] * -1.0;
	Threshold_Volt_Under_Range[1] = Volt_Range_Values[0] * Under_Range_Threshold_Fact;
	Threshold_Volt_Under_Range[2] = Volt_Range_Values[1] * Under_Range_Threshold_Fact;
	Threshold_Volt_Under_Range[3] = Volt_Range_Values[2] * Under_Range_Threshold_Fact;
	Threshold_Volt_Under_Range[4] = Volt_Range_Values[3] * Under_Range_Threshold_Fact;
		
	// calculate Threshold_Curr_Over_Range
	Threshold_Curr_Over_Range[0] = Curr_Range_Values[0] * Over_Range_Threshold_Fact;
	Threshold_Curr_Over_Range[1] = Curr_Range_Values[1] * Over_Range_Threshold_Fact;
	Threshold_Curr_Over_Range[2] = Curr_Range_Values[2] * Over_Range_Threshold_Fact;
	Threshold_Curr_Over_Range[3] = Curr_Range_Values[3] * Over_Range_Threshold_Fact;
	Threshold_Curr_Over_Range[4] = Curr_Range_Values[4] * Over_Range_Threshold_Fact;
	Threshold_Curr_Over_Range[5] = Curr_Range_Values[5] * Over_Range_Threshold_Fact;
	
	// calculate Threshold_Curr_Under_Range
	Threshold_Curr_Under_Range[0] = Curr_Range_Values[0] * -1.0;
	Threshold_Curr_Under_Range[1] = Curr_Range_Values[0] * Under_Range_Threshold_Fact;
	Threshold_Curr_Under_Range[2] = Curr_Range_Values[1] * Under_Range_Threshold_Fact;
	Threshold_Curr_Under_Range[3] = Curr_Range_Values[2] * Under_Range_Threshold_Fact;
	Threshold_Curr_Under_Range[4] = Curr_Range_Values[3] * Under_Range_Threshold_Fact;
	Threshold_Curr_Under_Range[5] = Curr_Range_Values[4] * Under_Range_Threshold_Fact;
	
}


//-------------------------------------------------------------------------------------------
// Set Voltage Range
void SET_VOLTAGE_RANGE(uint8_t Range_ID){
	/* this function sets a specific voltage range via the GPIO and loads the calibration factors */
	
	// RESET Range adjust frequency counter (reduce maximum switching to reduce relay wear)
	Range_Volt_Switch_Freq_Counter = Range_Volt_Switch_Freq_Ctr_Max;
	Range_Volt_Switch_Delay_Counter = Range_Volt_Switch_Delay_Ctr_Max;
	
	// load EEPROM values
	EEPROM_LOAD_VOLTAGE_RANGE(Range_ID);
			
	#ifdef PCBconfig_Vswitch_is_MAX4665 // analog switch used to change range
		// RESET Voltage Range control DO lines
		CLR__V_range_Spare;
		CLR__V_range_10V;
		CLR__V_range_100V;
		CLR__V_range_30V;
		// SET new Voltage Range control DO lines dependent on Range
		if(Range_ID == 0){ // 1V range
			DO_Nothing;
		}
		else if (Range_ID == 1) { // 3V range
			DO_Nothing;
		}
		else if (Range_ID == 2) { // 10V range
			SET__V_range_10V;
		}
		else if (Range_ID == 3) { // 30V range
			SET__V_range_30V;
		}
		else { // 100V range
			SET__V_range_100V;
		}
		// Transfer new DO line state to GPIOE
		DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
		// reset DAC control value update new scale
		Set_DAC_Output_Volts(PV_Volt_DAC_ACT);
	#endif	// PCBconfig_Vswitch_is_MAX4665
	
	#ifdef PCBconfig_Vswitch_is_1CL_Relay // mechanical 2 coil latching relay used to change range
		
		uint8_t Switch_Latch_Relay_10V_Spare;
		uint8_t Switch_Latch_Relay_100V_30V;
		Switch_Latch_Relay_10V_Spare = 1;
		Switch_Latch_Relay_100V_30V = 1;
		// check latch relay 10V_Spare status matches request
		if ((Range_Volt_ID_Act == 0) || (Range_Volt_ID_Act == 1) || (Range_Volt_ID_Act == 2)) {
			if ((Range_ID == 0) || (Range_ID == 1) || (Range_ID == 2))  Switch_Latch_Relay_10V_Spare = 0;
		}
		else if ((Range_Volt_ID_Act == 3) || (Range_Volt_ID_Act == 4)) {
			if ((Range_ID == 3) || (Range_ID == 4))   Switch_Latch_Relay_10V_Spare = 0;
		}
		// check latch relay 100V_30V status matches request
		if ((Range_Volt_ID_Act == 0) || (Range_Volt_ID_Act == 1) || (Range_Volt_ID_Act == 4)) {
			if ((Range_ID == 0) || (Range_ID == 1) || (Range_ID == 4))  Switch_Latch_Relay_100V_30V = 0;
		}
		else if ((Range_Volt_ID_Act == 2) || (Range_Volt_ID_Act == 3)) {
			if ((Range_ID == 2) || (Range_ID == 3))   Switch_Latch_Relay_100V_30V = 0;
		}
		// RESET Voltage Range control DO lines
		if (Switch_Latch_Relay_10V_Spare == 1) {
			CLR__V_range_Spare;
			CLR__V_range_10V;
		}
		if (Switch_Latch_Relay_100V_30V == 1) {
			CLR__V_range_100V;
			CLR__V_range_30V;
		}
		// SET new Voltage Range control relay lines dependent on Range
		if(Range_ID == 0){ // 1V range (virtual range)
			if (Switch_Latch_Relay_100V_30V == 1) SET__V_range_30V;
			if (Switch_Latch_Relay_10V_Spare == 1) SET__V_range_Spare;
		}
		else if (Range_ID == 1) { // 3V range
			if (Switch_Latch_Relay_100V_30V == 1) SET__V_range_30V;
			if (Switch_Latch_Relay_10V_Spare == 1) SET__V_range_Spare;
		}
		else if (Range_ID == 2) { // 10V range
			if (Switch_Latch_Relay_100V_30V == 1) SET__V_range_100V;
			if (Switch_Latch_Relay_10V_Spare == 1) SET__V_range_Spare;
		}
		else if (Range_ID == 3) { // 30V range
			if (Switch_Latch_Relay_100V_30V == 1) SET__V_range_100V;
			if (Switch_Latch_Relay_10V_Spare == 1) SET__V_range_10V;
		}
		else { // 100V range
			if (Switch_Latch_Relay_100V_30V == 1) SET__V_range_30V;
			if (Switch_Latch_Relay_10V_Spare == 1) SET__V_range_10V;
		}
		// Transfer new DO line state to GPIOE
		DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
		
		// wait until latching relay is set / reset
		_delay_ms(3); // max operation time 3ms (quick little fella)
		// reset DAC control value update new scale
		Set_DAC_Output_Volts(PV_Volt_DAC_ACT);
		_delay_ms(7); // delay to settle a bit & make sure the relays are there
		// RESET voltage range relay control lines (active line on relay driver means break mode, output lines pulled low)
		SET__V_range_Spare;
		SET__V_range_10V;
		SET__V_range_100V;
		SET__V_range_30V;
		// Transfer new DO line state to GPIOE
		DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
	#endif	// PCBconfig_Vswitch_is_1CL_Relay
		
	// wait until new signals settled
	_delay_ms(10); // just in case
	Range_Volt_ID_Act = Range_ID;
	SET__R_Stat_Volt_Clear_Buffer;	// clear voltage buffer on next voltage measurement as its in counts and not calibrated values
	CLR__Status_MainTimerOverRun;
	CLR__Timer_CTR_Main_Flag;
}

//-------------------------------------------------------------------------------------------
// Set Current Range
void SET_CURRENT_RANGE(uint8_t Range_ID){
	/* this function sets a specific current range via the GPIO and loads the calibration factors */
	uint8_t Switch_Latch_Relay;
	Switch_Latch_Relay = 1;
	
	// check latch relay status matches request
	if ((Range_Curr_ID_Act == 1) || (Range_Curr_ID_Act == 3) || (Range_Curr_ID_Act == 5)) {
		if ((Range_ID == 1) || (Range_ID == 3) || (Range_ID == 5))  Switch_Latch_Relay = 0;
	}
	else if ((Range_Curr_ID_Act == 0) || (Range_Curr_ID_Act == 2) || (Range_Curr_ID_Act == 4)) {
		if ((Range_ID == 0) || (Range_ID == 2) || (Range_ID == 4))  Switch_Latch_Relay = 0;
	}
	
	
	// RESET Range adjust frequency counter (reduce maximum switching to reduce relay wear)
	Range_Curr_Switch_Freq_Counter = Range_Curr_Switch_Freq_Ctr_Max;
	Range_Curr_Switch_Delay_Counter = Range_Curr_Switch_Delay_Ctr_Max;
	
	// RESET Current Range control DO lines (do not reset shunt MosFET driver -> make before break)
	CLR__Cm_OR_RST;
	CLR__C_range_mux_A0;
	CLR__C_range_mux_A1;
	CLR__C_range_gain_1;
	CLR__C_range_gain_2;
		
	// SET new Current Range control DO lines dependent on Range
	if(Range_ID == 0){ // 50mA or 0.5mA range
		SET__C_range_shunt_3;
		if (Switch_Latch_Relay == 1) SET__C_range_gain_2;
		SET__C_range_mux_A1;
	}
	else if (Range_ID == 1) { // 150mA or 1.5mA range
		SET__C_range_shunt_3;
		if (Switch_Latch_Relay == 1) SET__C_range_gain_1;
		SET__C_range_mux_A1;
	}
	else if (Range_ID == 2) { // 500mA or 5mA range
		SET__C_range_shunt_2;
		if (Switch_Latch_Relay == 1) SET__C_range_gain_2;
		SET__C_range_mux_A0;
	}
	else if (Range_ID == 3) { // 1.5A or 15mA range
		SET__C_range_shunt_2;
		if (Switch_Latch_Relay == 1) SET__C_range_gain_1;
		SET__C_range_mux_A0;
	}
	else if (Range_ID == 4) { // 5A or 50mA range
		SET__C_range_shunt_1;
		if (Switch_Latch_Relay == 1) SET__C_range_gain_2;
	}
	else { // 15A or 150mA range
		SET__C_range_shunt_1;
		if (Switch_Latch_Relay == 1) SET__C_range_gain_1;
	}
	
	// Transfer new DO line state to GPIOE
	DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
	// wait until latching relay is set / reset
	_delay_ms(10); // max operation time 3ms (quick little fella)
	
	// Reset Relay gain control lines
	#ifdef PCBconfig_Curr_switch_is_2CL_Relay
		// power off MosFET driver input lines
		CLR__C_range_gain_1;
		CLR__C_range_gain_2;	
	#endif	// PCBconfig_Curr_switch_is_2CL_Relay
	
	#ifdef PCBconfig_Curr_switch_is_1CL_Relay
		// set motor h-bridge driver to break mode, with output low
		SET__C_range_gain_1;
		SET__C_range_gain_2;
	#endif	// PCBconfig_Curr_switch_is_1CL_Relay
	
	
	// Disable all other shunts (made before breaking)
	if(Range_ID == 0){ // 50mA or 0.5mA range
		CLR__C_range_shunt_1;
		CLR__C_range_shunt_2;
	}
	else if (Range_ID == 1) { // 150mA or 1.5mA range
		CLR__C_range_shunt_1;
		CLR__C_range_shunt_2;
	}
	else if (Range_ID == 2) { // 500mA or 5mA range
		CLR__C_range_shunt_1;
		CLR__C_range_shunt_3;
	}
	else if (Range_ID == 3) { // 1.5A or 15mA range
		CLR__C_range_shunt_1;
		CLR__C_range_shunt_3;
	}
	else if (Range_ID == 4) { // 5A or 50mA range
		CLR__C_range_shunt_2;
		CLR__C_range_shunt_3;
	}
	else { // 15A or 150mA range
		CLR__C_range_shunt_2;
		CLR__C_range_shunt_3;
	}
	
	// Transfer new DO line state to GPIOE (disable latching relay control and shunts not needed)
	DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
	
	// load EEPROM values
	EEPROM_LOAD_CURRENT_RANGE(Range_ID);
		
	// wait until new signals settled
	_delay_ms(10); // just in case
	Range_Curr_ID_Act = Range_ID;
	SET__R_Stat_Curr_Clear_Buffer;	// clear voltage buffer on next voltage measurement as its in counts and not calibrated values
	CLR__Status_MainTimerOverRun;
	CLR__Timer_CTR_Main_Flag;
}


//-------------------------------------------------------------------------------------------
// Check Voltage Range and correct if not enabled
uint8_t Check_Voltage_Range_Enabled_Correct(uint8_t Range_ID){
	/* this function makes sure that the selected range is enabled, and selects the next higher range if not */
	
	//check range loop
	bool KeepChecking = true;
	while (KeepChecking) {
		if ((Range_Volt_Enable & BIT(Range_ID))) KeepChecking = false;
		else Range_ID++;
		
		if (Range_ID > Volt_range_max_ID){
			Range_ID = Volt_range_max_ID;
			 KeepChecking = false;
		}
	}
	
	// return checked and updated range
	return Range_ID;
}

//-------------------------------------------------------------------------------------------
// Check Current Range and correct if not enabled
uint8_t Check_Current_Range_Enabled_Correct(uint8_t Range_ID){
	/* this function makes sure that the selected range is enabled, and selects the next higher range if not */
	//check range loop
	bool KeepChecking = true;
	while (KeepChecking) {
		if (Range_Curr_Enable & BIT(Range_ID)) KeepChecking = false;
		else Range_ID++;
		
		if (Range_ID > Curr_range_max_ID){
			Range_ID = Curr_range_max_ID;
			KeepChecking = false;
		}
	}
	
	// return checked and updated range
	return Range_ID;
}