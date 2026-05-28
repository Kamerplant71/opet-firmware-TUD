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
	   IV & Transient meas C-code file
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


//===========================================================================================
// VARIABLES and STRUCTURES

//Transient measurement variables
volatile float PV_Volt_Trs;
volatile float Trs_Delay_us;
volatile float Trs_Meas_Time_us;
volatile float Trs_Delay_Res_us;

// IV measurements Variables
volatile uint8_t IV_Mode;
volatile uint8_t IV_Meas_Points;
volatile uint16_t IV_Point_Set_Delay_ms;
volatile uint16_t IV_Setup_Signal_Set_Time;
volatile uint8_t IV_Point_Meas_Nu_AVR_Sets;
volatile uint8_t IV_Point_Meas_Nu_AVR_VoltCurr;
volatile uint8_t IV_Report;
volatile float IV_Voc_Overshoot_Fact;
volatile float IV_Cos_Max_Phase;
volatile float PV_IV_Current[MAX_IV_POINTS];
volatile float PV_IV_Voltage[MAX_IV_POINTS];
volatile float PV_DAC_Points[MAX_IV_POINTS];
volatile uint8_t IV_Range_Volt_Last;
volatile uint8_t IV_Range_Curr_Last;

//===========================================================================================
// EEPROM VARIABLES
EEMEM uint8_t EROM_IV_Meas_Points = 100;
EEMEM uint8_t EROM_IV_Mode = 0b00000011; // measure IV: cos sweep, asymmetric volt
EEMEM float EROM_IV_Cos_Max_Phase = 1.57079;
EEMEM float EROM_IV_Voc_Overshoot_Fact = 1.01;
EEMEM uint16_t EROM_IV_Point_Set_Delay_ms = 5;
EEMEM uint8_t EROM_IV_Point_Meas_Nu_AVR_Sets = 1;
EEMEM uint8_t EROM_IV_Point_Meas_Nu_AVR_VoltCurr = 50;

//===========================================================================================
// FUNCTIONS
//===========================================================================================
//-------------------------------------------------------------------------------------------
// IV measurement Function
void PV_IV_MEAS(){
	/* This function measures the IV curve... good luck!
	- first sets optimal voltage range at VOC and optimal current range at ISC
	- then calculates all Voltage points (linear or cosine distribution)
	- then each point is measured with specific timing
	*/
	// need those variables
	float Voc;
	uint8_t i, k;
	uint16_t j;
	bool STOP_IV;
	
	
	// reset IV result result flags
	IV_Report = 0;
	STOP_IV = false;
	
	// save old control values to restore at end
	uint8_t Curr_Range_Old;
	uint8_t Volt_Range_Old;
	float DAC_Volt_Old;
	Curr_Range_Old = Range_Curr_ID_Act;
	Volt_Range_Old = Range_Volt_ID_Act;
	DAC_Volt_Old = PV_Volt_DAC_ACT;
	
	// calc variables for averaging (not wanting to do this 100 times)
	float Volt_AVR_Mult, Curr_AVR_Mult;
	if (is_IV_Mode__Asym_Volt) {
		Volt_AVR_Mult = 1.0 / ((float) (IV_Point_Meas_Nu_AVR_VoltCurr) * ((float) (IV_Point_Meas_Nu_AVR_Sets) + 1.0));
		Curr_AVR_Mult = 1.0 / ((float) (IV_Point_Meas_Nu_AVR_VoltCurr) * (float) (IV_Point_Meas_Nu_AVR_Sets));
	}
	else {
		Volt_AVR_Mult = 1.0 / ((float) (IV_Point_Meas_Nu_AVR_VoltCurr) * (float) (IV_Point_Meas_Nu_AVR_Sets));
		Curr_AVR_Mult = Volt_AVR_Mult;
	}
	if (IV_Point_Set_Delay_ms >= IV_Setup_Max_Delay_For_Mul) IV_Setup_Signal_Set_Time = 60000;
	else IV_Setup_Signal_Set_Time = IV_Point_Set_Delay_ms * IV_Setup_Signal_Set_Multiplier + IV_Range_Set_Settle_Time_ms;
		
	// Get optimal ranges for voltage and current + record VOC
	Voc = IV_MEAS__Get_Voc_Set_Optimum_Range();
	IV_MEAS__Get_Curr_Optimum_Range();
			
	// Calculate the measurement voltage output points
	if (is_IV_Mode__Cos_Sweep) {
		float Phase_Angle, Cos_End_Val, Phase_Act;
		// calc phase angle
		Phase_Angle = IV_Cos_Max_Phase / (float) (IV_Meas_Points-1);
		Voc = IV_Voc_Overshoot_Fact * Voc; // measure over VOC to get to get down to I=0
		// determine cos end value (sin really) & scale Voc value to reach endpoint
		Cos_End_Val = sin(Phase_Angle * (float) (IV_Meas_Points-1));
		if (IV_Cos_Max_Phase <= 1.57079) {
			// first quarter only
			Voc = Voc * 1 / Cos_End_Val;
		}
		else {
			// going into second quarter
			Voc = Voc * 1 / (2 - Cos_End_Val);
		}
		// calculate points
		for (i=0; i < IV_Meas_Points; i++) {
			// calc points with cos distribution
			Phase_Act = Phase_Angle * i;
			if (Phase_Act <= 1.57079) {
				// first quarter
				PV_DAC_Points[i] = Voc * sin(Phase_Act);
			}
			else {
				// second quarter
				PV_DAC_Points[i] = Voc * (2 - sin(Phase_Act));
			}
		}
	}
	else { // linear voltage distribution
		float Volt_Step_Size;
		Voc = IV_Voc_Overshoot_Fact * Voc; // measure over VOC to get to get down to I=0
		Volt_Step_Size = Voc / ((float)(IV_Meas_Points-1));
		// calc points with linear distribution
		for (i=0; i < IV_Meas_Points; i++) {
			PV_DAC_Points[i] = Volt_Step_Size * (float) (i);
		}
	}
	// set IV start point
	if(is_IV_Mode__Reverse) Set_DAC_Output_Volts(PV_DAC_Points[(IV_Meas_Points-1)]);
	else Set_DAC_Output_Volts(PV_DAC_Points[0]);
	// wait until settled
	j = IV_Setup_Signal_Set_Time;
	wdt_reset();
	while (j--){
		_delay_ms(1);
		wdt_reset();
	}
	
	// measure IV curve
	for (i=0; i < IV_Meas_Points; i++) {
		// set voltage
		if(is_IV_Mode__Reverse) Set_DAC_Output_Volts(PV_DAC_Points[(IV_Meas_Points-i-1)]);
		else Set_DAC_Output_Volts(PV_DAC_Points[i]);
		// reset watchdog
		wdt_reset();
		// wait until settled
		j = IV_Point_Set_Delay_ms;
		k = 0;
		while (j) {
			// reset watchdog
			wdt_reset();
			// measure temperature & control fans & over temperature takes 870 to 876us
			if (k == 1) {
				if (meas_NTC1_Temp_control_fan_over_Temp()){
					STOP_IV = true;
					SET__IV_Report_OverTemp_Active;
				}
				_delay_us(403);
			}
			else if (k == 2) {
				if (meas_NTC2_Temp_control_fan_over_Temp()){
					STOP_IV = true;
					SET__IV_Report_OverTemp_Active;
				}
				_delay_us(403);
			}
			else if (k == 3) {
				meas_Bias_Volt_Range_Check();
				_delay_us(655); // remaining time at 20 measurement averages
			}
			else {
				// do nothing delay
				_delay_us(999);
			}
			
			// delay remainder of time if not over temperature
			if (STOP_IV) break;
			// set counter variables
			j--;
			k++;
			if(k >= 10) k = 0;
			
		}
		// break if over temperature
		if (STOP_IV) break;
		// measure current and voltage
		IV_MEAS__Meas_Curr_Volt_Point(Volt_AVR_Mult, Curr_AVR_Mult);
		// transfer new data
		PV_IV_Voltage[i] = AI_Volt;
		PV_IV_Current[i] = AI_Curr_Corr;
	}
	
	// finish up now...
	if (is_DI_Curr_Bypass_ON) SET__IV_Report_OverCurr_Bypass_Active; // check if clamp active
	else CLR__IV_Report_OverCurr_Bypass_Active;
	SET__Track_Status_new_IV_curve;
	SET__Status_NewIvDataAvailable;
	CLR__SysControl_IV_Meas;
	PV_Volt_DAQ_New = DAC_Volt_Old;		// reset voltage
	// set old range and voltage again
	if(Range_Curr_ID_Act != Curr_Range_Old) {
		Range_Curr_Switch_Freq_Counter = 0;
		SET_CURRENT_RANGE(Curr_Range_Old);
	}
	if(Range_Volt_ID_Act != Volt_Range_Old) {
		Range_Volt_Switch_Freq_Counter = 0;
		SET_VOLTAGE_RANGE(Volt_Range_Old);
	}
	Set_DAC_Output_Volts(PV_Volt_DAQ_New);
	Set_ADC_Mux_Voltage;
	//settle last voltage
	j = IV_Setup_Signal_Set_Time;
	wdt_reset();
	while (j--){
		_delay_ms(1);
		wdt_reset();
	}
	CLR__Status_MainTimerOverRun;
	CLR__Timer_CTR_Main_Flag;
	
}

//-------------------------------------------------------------------------------------------
// IV measurement Function
void PV_TRANS_MEAS(){
	/* transient measurement function for debugging and control regulation testing 
	- could use a time delay feedback, accurate to nop delay increments...
	*/
	// required variables
	uint16_t Temp_Data[IV_Meas_Points];
	uint8_t	i;
	uint16_t n, j;
	uint8_t Act_IV_Point;
	uint16_t Delay_nu_nops;
	uint8_t Volt_Range_Old = Range_Volt_ID_Act;
	uint8_t Curr_Range_Old = Range_Curr_ID_Act;
	float PV_Volt_DAC_Old = PV_Volt_DAC_ACT;
	
	// calc signal setting  times needed
	if (IV_Point_Set_Delay_ms >= IV_Setup_Max_Delay_For_Mul) IV_Setup_Signal_Set_Time = 60000;
	else IV_Setup_Signal_Set_Time = IV_Point_Set_Delay_ms * IV_Setup_Signal_Set_Multiplier + IV_Range_Set_Settle_Time_ms;
	
	// calculate delay time in nops (no operation loops)
	if (Trs_Delay_us <= Trs_Meas_Time_us) {
		Trs_Delay_us = Trs_Meas_Time_us;
	}
	Delay_nu_nops = (uint16_t) round((Trs_Delay_us - Trs_Meas_Time_us)/Trs_Delay_Res_us);
	
	// set voltage range if in auto-range mode
	if (!is_manual_Volt_Range_Set) { // set optimal voltage range
		// select best voltage range based on highest voltage control point
		Range_Volt_Switch_Freq_Counter = 0;
		if (PV_Volt_Setpoint > PV_Volt_Trs) {
			SET_VOLTAGE_RANGE(Get_Best_Voltage_Range_From_Value(PV_Volt_Setpoint));
		}
		else {
			SET_VOLTAGE_RANGE(Get_Best_Voltage_Range_From_Value(PV_Volt_Trs));
		}
	}
	// set current range if in auto-range mode
	if (!is_manual_Curr_Range_Set) { // set optimal current range
		// select best current range
		IV_MEAS__Get_Curr_Optimum_Range(); // also gets it to 0V start point
	}
		
	// set start voltage
	Set_DAC_Output_Volts(PV_Volt_Setpoint);
	Set_ADC_Mux_Voltage; // set voltage input MUX address
	//settle start voltage
	j = IV_Setup_Signal_Set_Time;
	wdt_reset();
	while (j--){
		_delay_ms(1);
		wdt_reset();
	}
	i = IV_Meas_Points;
	Act_IV_Point = 0;
	// set step voltage
	Set_DAC_Output_Volts(PV_Volt_Trs);
	// measure voltage transient
	while(i) {
		Temp_Data[Act_IV_Point] = ADC_Convert_MAX11163();	// measure Voltage channel
		i --; // counting down to  zero
		Act_IV_Point ++; // next data point
		n = Delay_nu_nops; // reset n
		while(n){
			n --; // counting down to  zero
			asm volatile ("nop" :: );
			//_delay_us(1); // delay time to wait
		}
	}
	// process voltage data
	i = IV_Meas_Points;
	while(i) {
		i--; // counting to zero
		PV_IV_Voltage[i] = (float) Temp_Data[i]; // - AI_Volt_Offset) * AI_Volt_Scale;
	}
		
	// set start voltage
	Set_DAC_Output_Volts(PV_Volt_Setpoint);
	Set_ADC_Mux_Current; // set current input MUX address
	//settle start voltage
	j = IV_Setup_Signal_Set_Time;
	wdt_reset();
	while (j--){
		_delay_ms(1);
		wdt_reset();
	}
	i = IV_Meas_Points;
	Act_IV_Point = 0;
	// set step voltage
	Set_DAC_Output_Volts(PV_Volt_Trs);
	// measure current transient
	while(i) {
		Temp_Data[Act_IV_Point] = ADC_Convert_MAX11163();	// measure Current channel
		i --; // counting down to  zero
		Act_IV_Point ++; // next data point
		n = Delay_nu_nops; // reset n
		while(n){
			n --; // counting down to  zero
			asm volatile ("nop" :: );
			//_delay_us(1); // delay time to wait
		}
	}
	// process current data
	i = IV_Meas_Points;
	while(i) {
		i--; // counting to zero
		PV_IV_Current[i] = (float) Temp_Data[i]; // - AI_Curr_Offset) * AI_Curr_Scale;
	}
		
	// transient measurement finished, reset old conditions
	CLR__SysControl_Trans_Meas;
	SET__Status_NewIvDataAvailable;
	Set_DAC_Output_Volts(PV_Volt_DAC_ACT); // set back to last output voltage
	if(Range_Curr_ID_Act != Curr_Range_Old) {
		Range_Curr_Switch_Freq_Counter = 0;
		SET_CURRENT_RANGE(Curr_Range_Old);
	}
	if(Range_Volt_ID_Act != Volt_Range_Old) {
		Range_Volt_Switch_Freq_Counter = 0;
		SET_VOLTAGE_RANGE(Volt_Range_Old);
	}
	Set_DAC_Output_Volts(PV_Volt_DAC_Old);
	Set_ADC_Mux_Voltage;
	//settle last voltage
	j = IV_Setup_Signal_Set_Time;
	wdt_reset();
	while (j--){
		_delay_ms(1);
		wdt_reset();
	}
	CLR__Status_MainTimerOverRun;
	CLR__Timer_CTR_Main_Flag;

}


//-------------------------------------------------------------------------------------------
// Get to Voc & set optimum range 
float IV_MEAS__Get_Voc_Set_Optimum_Range() {
	/* function returns VOC, the measured voltage after setting range and controlling DAC at full scale
	- function does not change range in manual mode
	- if in manual ranging, returns voltage at max DAC control, which may not be VOC if its in a lower range
	*/
	uint8_t i;
	uint16_t j;
	uint8_t OLD_Range;
	
	//Switch over to last range used during IV measurements if larger than active
	// this should reduce range switching when active load is lower voltage range such as at ISC load
	if (!is_manual_Volt_Range_Set) {
		if (IV_Range_Volt_Last > Range_Volt_ID_Act){
			SET_VOLTAGE_RANGE(IV_Range_Volt_Last);
		}
	}
	
	// Set max voltage
	PV_Volt_DAQ_New = Threshold_Volt_Over_Range[Volt_range_max_ID];
	Set_DAC_Output_Volts(PV_Volt_DAQ_New);	// Reset DAC to max voltage to get a VOC measurement
	// wait until settled
	j = IV_Setup_Signal_Set_Time;
	wdt_reset();
	while (j--){
		_delay_ms(1);
		wdt_reset();
	}
		
	// move to optimum range
	i = Volt_range_max_ID+1; // maximum rounds
	while (i--){
		Meas_AI_Volt_Only();
		if (!(is_manual_Volt_Range_Set)) {
			OLD_Range = Range_Volt_ID_Act;
			Range_Volt_Switch_Freq_Counter = 0;	// reset switching frequency limiter
			Range_Volt_Switch_Delay_Counter = 0; // remove switching delay
			Process_Voltage_Auto_Range();
			// get out a here if range has not changed
			if (OLD_Range == Range_Volt_ID_Act) {
				IV_Range_Volt_Last = Range_Volt_ID_Act;
				break;
			}
			else { // range changed, wait until settled & measure again
				_delay_ms(IV_Range_Set_Settle_Time_ms); // wait new range settled
			}
		}
		else { break; }
	}
	return AI_Volt;

}

//-------------------------------------------------------------------------------------------
// Get to ISC & set optimum range 
void IV_MEAS__Get_Curr_Optimum_Range() {
	/* function returns ISC, the measured current after setting range and controlling DAC at zero scale
	- function does not change range in manual mode
	- if in manual ranging, returns voltage at max DAC control, which may not be VOC if its in a lower range
	*/
	uint8_t i;
	uint16_t j;
	uint8_t OLD_Range;
	
	//Switch over to last range used during IV measurements if larger than active
	// this should reduce range switching when active load is lower current range such as at VOC load
	if (!is_manual_Curr_Range_Set) {
		if (IV_Range_Curr_Last > Range_Curr_ID_Act){
			SET_CURRENT_RANGE(IV_Range_Curr_Last);	
		}
	}
	
	// Set Voltage Range if not in Manual mode
	PV_Volt_DAQ_New = 0; // Set DAC voltage to zero for ISC
	Set_DAC_Output_Volts(PV_Volt_DAQ_New);	
	// wait until settled
	j = IV_Setup_Signal_Set_Time;
	wdt_reset();
	while (j--){
		_delay_ms(1);
		wdt_reset();
	}
	// reset over range state
	if (is_DI_Curr_Bypass_ON){ // read actual DI here as not updated yet
		// clear current over-range in case of an intermittent over current spike
		// reduces chance of changing range when not really needed
		Reset_Over_Current_Clamp();
		_delay_ms(IV_Range_Set_Settle_Time_ms); // wait until range signal settled again
		
	}
	// get optimum range
	i = Curr_range_max_ID+1; // maximum rounds
	while (i--){
		Meas_AI_Curr_Only();
		if (!is_manual_Curr_Range_Set) {
			OLD_Range = Range_Curr_ID_Act;
			Range_Curr_Switch_Freq_Counter = 0;	// reset switching frequency limiter
			Range_Curr_Switch_Delay_Counter = 0; // remove switching delay
			Process_Current_Auto_Range();
			// get out a here if range has not changed
			if (OLD_Range == Range_Curr_ID_Act) {
				IV_Range_Curr_Last = Range_Curr_ID_Act;
				break;
			}
			else { // range changed, wait until settled & measure again
				_delay_ms(IV_Range_Set_Settle_Time_ms); // wait until range signal settled
			}
		}
		else { break; }
	}
	
}

//-------------------------------------------------------------------------------------------
// Measure current and voltage of IV point
void IV_MEAS__Meas_Curr_Volt_Point(float Volt_AVR_Mult, float Curr_AVR_Mult){
	/* function measures current & voltage signals after settling
	- measures asymmetric voltage to reduce impact on possible changing signals
	- measures and averages sets of voltage & current multiple times as configured
	- this one is optimized to run a little faster
	*/
	// need those variables
	uint8_t i;
	uint32_t Sum_Volt, Sum_Curr;
	float Temp_Float;
	Sum_Volt = 0;
	Sum_Curr = 0;
	// measure current and voltage
	i = IV_Point_Meas_Nu_AVR_Sets; 
	while (i) {
		// measure Voltage
		Set_ADC_Mux_Voltage; // set voltage input MUX address
		_delay_us(ADC_MUX_Set_Delay_us); // time it takes to set the channel and settle the signal at the ADC
		Sum_Volt = Sum_Volt + ADC_Convert_Avr_OLUL_MAX11163(IV_Point_Meas_Nu_AVR_VoltCurr);	// measure Voltage channel
		// get voltage OL & UL flags
		if (AI_Under_Load) {
			SET__IV_Report_Volt_Under_Load;
		}
		else if (AI_Over_Load) {
			SET__IV_Report_Volt_Over_Load;
		}
		// measure Current
		Set_ADC_Mux_Current;
		_delay_us(ADC_MUX_Set_Delay_us); // time it takes to set the channel and settle the signal at the ADC
		Sum_Curr = Sum_Curr + ADC_Convert_Avr_OLUL_MAX11163(IV_Point_Meas_Nu_AVR_VoltCurr);	// measure Voltage channel
		// get voltage OL & UL flags
		if (AI_Under_Load) {
			SET__IV_Report_Curr_Under_Load;
		}
		else if (AI_Over_Load) {
			SET__IV_Report_Curr_Over_Load;
		}
		i--; // count down
	}
	// make last asymmetric Voltage measurement if requested
	if (is_IV_Mode__Asym_Volt) {
		Set_ADC_Mux_Voltage; // set voltage input MUX address
		_delay_us(ADC_MUX_Set_Delay_us); // time it takes to set the channel and settle the signal at the ADC
		Sum_Volt = Sum_Volt + ADC_Convert_Avr_OLUL_MAX11163(IV_Point_Meas_Nu_AVR_VoltCurr);	// measure Voltage channel
		// get voltage OL & UL flags
		if (AI_Under_Load) {
			SET__R_Stat_Volt_Under_Load;
		}
		else if (AI_Over_Load) {
			SET__R_Stat_Volt_Over_Load;
		}
	}
	// Calculate results
	Temp_Float = (float) Sum_Volt * Volt_AVR_Mult ;
	AI_Volt = (Temp_Float + AI_Volt_Offset) * AI_Volt_Scale;
	Temp_Float = (float) Sum_Curr * Curr_AVR_Mult;
	AI_Curr = (Temp_Float + AI_Curr_Offset) * AI_Curr_Scale;
	AI_Curr_Corr = AI_Curr + AI_Volt * AI_Volt_R_Leak; 
	
}

//-------------------------------------------------------------------------------------------
// find the DAC Voltage at maximum power
float get_DAC_Volt_at_Pmax_from_IV(){
	/* this function returns the DAC control Voltage at maximum power from an IV curve
	*/
	// initialize the variables
	float V_Pmax, Pmax, P_actual;
	uint8_t i;
	// preset
	V_Pmax = 0;
	Pmax = 0;
	i = IV_Meas_Points;
	
	while (i)
	{
		i--;
		P_actual = PV_IV_Voltage[i] * PV_IV_Current[i];
		if (P_actual > Pmax) {
			Pmax = P_actual;
			V_Pmax = PV_IV_Voltage[i];
		}
	}
	
	return V_Pmax;
}


//-------------------------------------------------------------------------------------------
// IV curve measure NTC 1 temperature & control fans & over temperature
bool meas_NTC1_Temp_control_fan_over_Temp(){
	/* this function measures the temperature of NTC 2 sensor controls following:
		- switch on of fans when threshold is reached
		- break from IV measurements when over temperature is measured
	*/
	// temporal variables
	uint32_t Temp_uint32;
	float Temp_float;
	
	// meas NTC Temp Ch 1
	Set_ADC_Mux_NTC_1; // set NTC Temp 1 input MUX address
	_delay_us(ADC_MUX_Set_Delay_us); // remainder to get to the 100us MUX channel switching
	Temp_uint32 = ADC_Convert_Avr_MAX11163(IV_Temp_Bias_Mon_No_Meas);	// measure NTC Temp 1 channel
	Set_ADC_Mux_GND; // set GND input MUX address, resting  - no delay needed at end
	Temp_float = (float) Temp_uint32 * IV_Temp_Bias_Mon_No_Meas_Devider;
	Temp_float = Temp_float * CAL_NTC_1_InvGain; // counts without gain
	Temp_float = (Temp_float / (65536 - Temp_float)) * CAL_NTC_1_RSer; // Resistance NTC
	AI_NTC_Temp_1 = 1/ (log(Temp_float * CAL_NTC_1_InvR25) * CAL_NTC_1_InvBeta + 3.35402E-03) - 273.15;
	
	// control fans dependent on temperature only if in Auto mode
	if ((!is_FAN_MAN_Mode) && (!is_FAN_IS_ENABLED)) {
		if((AI_NTC_Temp_2 > FAN_NTC2_T_On) || (AI_NTC_Temp_1 > FAN_NTC1_T_On)){
			CLR__LRELAY_FAN_Off;
			SET__LRELAY_FAN_On;
			SET__FAN_IS_ENABLED;
			SET__FAN_ENABLED_Request;
			FAN_Switch_Counter = FAN_Switch_Timer_Max;			
			// wait until latching relay is set/reset
			_delay_ms(FAN_RELAY_SWITCH_TIME_MS);
			// set control to break mode on motor controller
			SET__LRELAY_FAN_Off;
			SET__LRELAY_FAN_On;
		}
	}
	
	//return value based on over-under temperature state
	return  Check_Temp_Disconnect_State();
}


//-------------------------------------------------------------------------------------------
// IV curve measure NTC 2 temperature & control fans & over temperature
bool meas_NTC2_Temp_control_fan_over_Temp(){
	/* this function measures the temperature of NTC 1 sensor controls following:
		- switch on of fans when threshold is reached
		- break from IV measurements when over temperature is measured
	*/
	// temporal variables
	uint32_t Temp_uint32;
	float Temp_float;
	
	// meas NTC Temp Ch 2
	Set_ADC_Mux_NTC_2; // set NTC Temp 2 input MUX address
	_delay_us(ADC_MUX_Set_Delay_us); // remainder to get to the 100us MUX channel switching
	Temp_uint32 = ADC_Convert_Avr_MAX11163(IV_Temp_Bias_Mon_No_Meas);	// measure NTC Temp 2 channel
	Set_ADC_Mux_GND; // set bias volt channel - no delay calcs take lots of time already... ~270us
	Temp_float = (float) Temp_uint32 * IV_Temp_Bias_Mon_No_Meas_Devider;
	Temp_float = Temp_float * CAL_NTC_2_InvGain; // counts without gain
	Temp_float = (Temp_float / (65536 - Temp_float)) * CAL_NTC_2_RSer; // Resistance NTC
	AI_NTC_Temp_2 = 1/ (log(Temp_float * CAL_NTC_2_InvR25) * CAL_NTC_2_InvBeta + 3.35402E-03) - 273.15;
	
	// control fans dependent on temperature only if in Auto mode
	if ((!is_FAN_MAN_Mode) && (!is_FAN_IS_ENABLED)) {
		if((AI_NTC_Temp_2 > FAN_NTC2_T_On) || (AI_NTC_Temp_1 > FAN_NTC1_T_On)){
			CLR__LRELAY_FAN_Off;
			SET__LRELAY_FAN_On;
			SET__FAN_IS_ENABLED;
			SET__FAN_ENABLED_Request;
			FAN_Switch_Counter = FAN_Switch_Timer_Max;			
			// wait until latching relay is set/reset
			_delay_ms(FAN_RELAY_SWITCH_TIME_MS);
			// set control to break mode on motor controller
			SET__LRELAY_FAN_Off;
			SET__LRELAY_FAN_On;
		}
	}
	
	//return value based on over-under temperature state
	return  Check_Temp_Disconnect_State();
}

	
//-------------------------------------------------------------------------------------------
// IV curve measure bias voltage range check
void meas_Bias_Volt_Range_Check(){
	/* this function measures the Bias voltage & controls following:
		- set bias volt fault if bias voltage is out of range
		- bias voltage out of range will not cause break rom IV curve measurement
	*/
	// temporal variables
	uint32_t Temp_uint32;
	float Temp_float;
	
	Set_ADC_Mux_Bias; // set bias volt channel
	_delay_us(ADC_MUX_Set_Delay_us); // remainder to get to the 100us MUX channel switching
	Temp_uint32 = ADC_Convert_Avr_MAX11163(IV_Temp_Bias_Mon_No_Meas);	// measure bias channel
	Set_ADC_Mux_GND; // set GND input MUX address, resting  - no delay needed at end
	Temp_float = (float) Temp_uint32 * IV_Temp_Bias_Mon_No_Meas_Devider;
	AI_Bias = (Temp_float * CAL_Bias_A1) + CAL_Bias_A0;
	
	// check bias voltage fault
	if ((AI_Bias < BiasV_Range_Min) || (AI_Bias > BiasV_Range_Max)) {
		SET__IV_Report_BiasVolt_Fault;
	}
		
}
	




