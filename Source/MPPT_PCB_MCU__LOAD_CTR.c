/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Martin Bliss
	Version:	1.13
	Date:		28.02.2024
	-----------------------------------------
	ATMega 324 Micro controller support
	-----------------------------------------
	  Load Control functions C-Code FILE
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

volatile int32_t F_CPU_CAL;
volatile char Sample_Name[NAME_LENGTH];
volatile uint8_t BiasV_Error_Timout_Counter;
volatile uint8_t BiasV_Error_Delay_Counter;

// Load control Variables
volatile uint8_t PV_Mode_CTR;
volatile uint8_t PV_Mode_ACT;

// Output Voltage Control Variables
volatile float PV_Volt_Setpoint;	// manual voltage control set-point
volatile float PV_Volt_DAQ_New; // new set-point to be send to the DAC, control this to indicate changes to DAC to Control loop
volatile float PV_Volt_DAC_ACT; // actual control point if the DAQ, only set when communicated to DAC

// Output current/mppt control variables
volatile float PV_Curr_Setpoint; // manual current control set-point
volatile uint8_t Track_Status;
volatile float Track_Volt_Step_Size;
volatile float Track_Power_Old;
volatile float PV_Active_Load_Power;
volatile float LOAD_MPPT_Max_Step_CND;
volatile float LOAD_MPPT_Min_Step_CND;
volatile float LOAD_MPPT_Step_Increase_Fact;
volatile float LOAD_MPPT_Step_Reduction_Fact;
volatile float LOAD_MPPT_Tolerance_CND;
volatile uint16_t LOAD_MPPT_Timer_Update_Delay;
volatile uint16_t LOAD_MPPT_Update_Counter;
volatile float LOAD_CurrT_Max_Step_CND;
volatile float LOAD_CurrT_Min_Step_CND;
volatile float LOAD_CurrT_Step_Increase_Fact;
volatile float LOAD_CurrT_Step_Reduction_Fact;
volatile float LOAD_CurrT_Tolerance_CND;

// Fan control variables
volatile uint8_t FAN_CTR_State;
volatile float FAN_NTC1_T_On;
volatile float FAN_NTC1_T_Off;
volatile float FAN_NTC2_T_On;
volatile float FAN_NTC2_T_Off;
volatile float FAN_Power_On;
volatile float FAN_Power_Off;
volatile uint16_t FAN_Switch_Counter;
volatile uint16_t FAN_Switch_Timer_Max;

// Temperature limit variables
volatile float Temp_NTC1_MIN_DisCon;
volatile float Temp_NTC1_MAX_DisCon;
volatile float Temp_NTC1_MIN_ReCon;
volatile float Temp_NTC1_MAX_ReCon;
volatile float Temp_NTC2_MIN_DisCon;
volatile float Temp_NTC2_MAX_DisCon;
volatile float Temp_NTC2_MIN_ReCon;
volatile float Temp_NTC2_MAX_ReCon;

// Bias Voltage Limit variables
volatile float BiasV_Range_Min;
volatile float BiasV_Range_Max;

//===========================================================================================
// EEPROM VARIABLES
EEMEM int32_t EROM_F_CPU_CAL = F_CPU;
EEMEM char EROM_Sample_Name[NAME_LENGTH] = "Not Sure!";
EEMEM float EROM_Temp_NTC1_MIN_DisCon = 0.0;
EEMEM float EROM_Temp_NTC1_MAX_DisCon = 45.0;
EEMEM float EROM_Temp_NTC1_MIN_ReCon = 5.0;
EEMEM float EROM_Temp_NTC1_MAX_ReCon = 40.0;
EEMEM float EROM_Temp_NTC2_MIN_DisCon = 0.0;
EEMEM float EROM_Temp_NTC2_MAX_DisCon = 70.0;
EEMEM float EROM_Temp_NTC2_MIN_ReCon = 5.0;
EEMEM float EROM_Temp_NTC2_MAX_ReCon = 50.0;
EEMEM float EROM_BiasV_Range_Min = 1.0;
EEMEM float EROM_BiasV_Range_Max = 6.0;
EEMEM float EROM_LOAD_MPPT_Max_Step_CND =  200.0;
EEMEM float EROM_LOAD_MPPT_Min_Step_CND =  3.0;
EEMEM float EROM_LOAD_MPPT_Step_Increase_Fact =  1.2;
EEMEM float EROM_LOAD_MPPT_Step_Reduction_Fact =  0.6;
EEMEM float EROM_LOAD_MPPT_Tolerance_CND = 300.0;
EEMEM uint16_t EROM_LOAD_MPPT_Timer_Update_Delay = 1;
EEMEM float EROM_LOAD_CurrT_Max_Step_CND =  200.0;
EEMEM float EROM_LOAD_CurrT_Min_Step_CND =  1.0;
EEMEM float EROM_LOAD_CurrT_Step_Increase_Fact =  1.2;
EEMEM float EROM_LOAD_CurrT_Step_Reduction_Fact =  0.6;
EEMEM float EROM_LOAD_CurrT_Tolerance_CND = 5.0;
EEMEM uint8_t EROM_PV_Mode_CTR = 0;
EEMEM float EROM_PV_Volt_Setpoint = 0.0;
EEMEM float EROM_PV_Curr_Setpoint = 0.0;
EEMEM uint8_t EROM_FAN_CTR_Mode = 0;
EEMEM float EROM_FAN_NTC1_T_ON = 35.0;
EEMEM float EROM_FAN_NTC1_T_OFF = 30.0;
EEMEM float EROM_FAN_NTC2_T_ON = 35.0;
EEMEM float EROM_FAN_NTC2_T_OFF = 30.0;
EEMEM float EROM_FAN_Power_ON = 10.0;
EEMEM float EROM_FAN_Power_OFF = 5.0;
EEMEM uint16_t EROM_FAN_Switch_Timer_Max = 4800;

//===========================================================================================
// FUNCTIONS
//===========================================================================================
//-------------------------------------------------------------------------------------------
// MAIN PV MPPT Control
void LOAD_CTR_MAIN(){
	
	// Calculate actual load power
	PV_Active_Load_Power = AI_Volt * AI_Curr_Corr;
		
	// Check if calibration mode entered!
	if (is_SysControl_Cal_Mode) {
		// cal mode, make sure output stays off
		SET__Status_Calibrate;
		CLR__SysControl_Ouput_On;
	}
	else {
		CLR__Status_Calibrate;
	}
	
	// Check operating temperature conditions
	Process_Operating_Temperature_State();
		
	// Process Bias Power State
	Process_Bias_Voltage_State();
	
	// Process Fan Control
	Process_Fan_Control();
	
	// Process Error LED on state
	Process_Error_LED_State();
	
	// Check PV output control bit
	if (is_SysControl_Ouput_On) {
		// PV Control Enabled, indicate output enabled
		// ... done in else cases now to reduce confusion
		
				
		// check if IV Measurement requested
		if (is_SysControl_IV_Meas) {		// IV measurement mode
			if (Check_Temp_Disconnect_State() || is_Status_Bias_Error ) {
				// error active, don't measure IV
				CLR__Status_Ouput_On;
				CLR__LED_Output_On;
				CLR__Output_On; // disable PI-CTR output to keep at VOC
				CLR__SysControl_IV_Meas; // chancel IV measurement
			}
			else {
				SET__Status_Ouput_On;
				SET__LED_Output_On;
				SET__Output_On; // enable PI-CTR output
				PV_IV_MEAS();
			}
		}
		// Transient Measurement mode
		else if (is_SysControl_Trans_Meas) {// Transient measurement mode
			if (Check_Temp_Disconnect_State() || is_Status_Bias_Error ) {
				// error active, don't measure IV, don't measure Transient
				CLR__Status_Ouput_On;
				CLR__LED_Output_On;
				CLR__Output_On; // disable PI-CTR output to keep at VOC
				CLR__SysControl_Trans_Meas; // chancel Transient measurement
			}	
			else {
				SET__Status_Ouput_On;
				SET__LED_Output_On;
				SET__Output_On; // enable PI-CTR output
				PV_TRANS_MEAS();
			}
		}
		else { // Load mode active
			if (Check_if_Active_Errors_Output_Disabled()) {
				// error active, don't measure IV, switch off loading
				SET__Status_Ouput_On;
				SET__LED_Output_On;
				CLR__Output_On; // disable PI-CTR output to keep at VOC
				// RESET Range adjust frequency counter - stay in same range unless a higher range needed for measurements
				Range_Volt_Switch_Freq_Counter = Range_Volt_Switch_Freq_Ctr_Max;
				Range_Curr_Switch_Freq_Counter = Range_Curr_Switch_Freq_Ctr_Max;
			}
			else if (PV_Mode_CTR == PV_Mode__VOC){
				PV_Mode_ACT = PV_Mode_CTR;
				SET__Status_Ouput_On;
				SET__LED_Output_On;
				CLR__Output_On; // disable PI-CTR output to keep at VOC
			}
			else if (PV_Mode_CTR == PV_Mode__ISC){
				PV_Mode_ACT = PV_Mode_CTR;
				SET__Status_Ouput_On;
				SET__LED_Output_On;
				SET__Output_On; // enable PI-CTR output
				PV_Volt_DAQ_New = 0; // set to 0V ISC
			}
			else if (PV_Mode_CTR == PV_Mode__Const_Volt){
				PV_Mode_ACT = PV_Mode_CTR;
				SET__Status_Ouput_On;
				SET__LED_Output_On;
				SET__Output_On; // enable PI-CTR output
				PV_Volt_DAQ_New = PV_Volt_Setpoint;	// SET DAC to manual value
			}
			else if (PV_Mode_CTR == PV_Mode__Const_Curr){
				SET__Status_Ouput_On;
				SET__LED_Output_On;
				SET__Output_On; // enable PI-CTR output
				if(PV_Mode_ACT == PV_Mode_CTR) { 
					PV_Mode__Constant_Current_Main(); 
				}
				else { 
					PV_Mode__Constant_Current_Init(); 
				}
			}
			else if (PV_Mode_CTR == PV_Mode__MPPT_Simple){
				SET__Status_Ouput_On;
				SET__LED_Output_On;
				SET__Output_On; // enable PI-CTR output
				if(PV_Mode_ACT == PV_Mode_CTR) {
					PV_Mode__MPPT_Simple_Main();
				}
				else {
					PV_Mode__MPPT_Simple_Init();
				}
			}
			else { // PV mode setting error
				// request switching off output next round
				PV_Mode_CTR = PV_Mode__NONE;
				CLR__Status_Ouput_On;
				CLR__LED_Output_On;
				CLR__Output_On;
				CLR__SysControl_Ouput_On;
			}
		}
				
	}
	else {
		// PV Control DISABLED - Disable Output
		CLR__Status_Ouput_On;		// Clear PV Enabled Control
		CLR__LED_Output_On;			// disable LOAD enabled indicator
		CLR__Output_On;				// disable PI-CTR output to keep at VOC
		CLR__SysControl_IV_Meas;	// Clear IV start control
		CLR__SysControl_Trans_Meas;	// clear Transient measurement control
		PV_Mode_ACT = PV_Mode__NONE;
		PV_Volt_DAQ_New = DAC_MAX_COUNTS;		// SET DAC to maxV
	}
	
	// Set PV control voltage
	if (PV_Volt_DAC_ACT != PV_Volt_DAQ_New) {
		Set_DAC_Output_Volts(PV_Volt_DAQ_New);
	}
		
}


//-------------------------------------------------------------------------------------------
// Process operation temperature states of NTC 1 & NTC 2
void Process_Operating_Temperature_State(){
	/* this function processes the status of the operation temperature disconnect and reconnect
	for NTC1 (board temperature NTC) and NTC2 (the diver MosFET NTC) */
	
	// check NTC 1 - board temperature
	if (is_Status_NTC1_Over_Temp) {
		// over/under temperature signal set, check if its within range again
		if ((AI_NTC_Temp_1 > Temp_NTC1_MIN_ReCon) && (AI_NTC_Temp_1 < Temp_NTC1_MAX_ReCon)) {
			CLR__Status_NTC1_Over_Temp;
		}
	}
	else {
		// temperature was normal, check if has gone out of range
		if ((AI_NTC_Temp_1 < Temp_NTC1_MIN_DisCon) || (AI_NTC_Temp_1 > Temp_NTC1_MAX_DisCon)) {
			SET__Status_NTC1_Over_Temp;
		}
	}
	
	// check NTC 2 - board temperature
	if (is_Status_NTC2_Over_Temp) {
		// over/under temperature signal set, check if its within range again
		if ((AI_NTC_Temp_2 > Temp_NTC2_MIN_ReCon) && (AI_NTC_Temp_2 < Temp_NTC2_MAX_ReCon)) {
			CLR__Status_NTC2_Over_Temp;
		}
	}
	else {
		// temperature was normal, check if has gone out of range
		if ((AI_NTC_Temp_2 < Temp_NTC2_MIN_DisCon) || (AI_NTC_Temp_2 > Temp_NTC2_MAX_DisCon)) {
			SET__Status_NTC2_Over_Temp;
		}
	}
	
}


//-------------------------------------------------------------------------------------------
// Process Bias Voltage Supply status
void Process_Bias_Voltage_State(){
	/* this function processes the bias voltage input signal, it indicates an error in case its out of range
	   which is used to disable MosFET control as necessary
	- if voltage is too low or too high, disconnect MosFET control signal
	- if voltage is within range allows enabling of MosFET control after a timeout */
	
	// check if in range
	if ((AI_Bias < BiasV_Range_Min) || (AI_Bias > BiasV_Range_Max)) {
		if(BiasV_Error_Delay_Counter > 0) BiasV_Error_Delay_Counter--;
		else {
			SET__Status_Bias_Error;
			BiasV_Error_Timout_Counter = BIAS_VOLT_ERROR_TIMEOUT_MAX;
		}
	}
	else {
		// only reset bias voltage error after its been fine for the entire count down time...
		if(BiasV_Error_Timout_Counter > 0) BiasV_Error_Timout_Counter--;
		else CLR__Status_Bias_Error;
		BiasV_Error_Delay_Counter = BIAS_VOLT_ERROR_DELAY_MAX;
	}
	
}


//-------------------------------------------------------------------------------------------
// Process Bias Voltage Supply status
void Process_Fan_Control(){
	/* this function controls the fan (on/off) dependent on temperature, power or manual control
	- hysteresis control for Power and temperature
	- separate control for NTC1 and NTC2
	- switch frequency counter is limiting max switch frequency of fan
	- actual fan state is written to output at end of function*/
		
	// Auto mode fan control - temperature and power
	if (!is_FAN_MAN_Mode) {
		
		if (FAN_Switch_Counter > 0) FAN_Switch_Counter --;
		
		if (!is_FAN_IS_ENABLED) { // Fan is not on yet
			if((AI_NTC_Temp_2 > FAN_NTC2_T_On) || (AI_NTC_Temp_1 > FAN_NTC1_T_On) || (PV_Active_Load_Power > FAN_Power_On)){
				SET__FAN_ENABLED_Request;
				FAN_Switch_Counter = FAN_Switch_Timer_Max;
			}
		}
		if (is_FAN_IS_ENABLED) { // Fan is already on
			if((AI_NTC_Temp_2 < FAN_NTC2_T_Off) && (AI_NTC_Temp_1 < FAN_NTC1_T_Off) && (PV_Active_Load_Power < FAN_Power_Off)){
				if (FAN_Switch_Counter == 0) CLR__FAN_ENABLED_Request;
			}
		}
	}
		
	//set fan state - if in manual or auto mode this is when the request is executed...
	// if statement does not like boolean == comparators...
	if ((is_FAN_ENABLED_Request && !is_FAN_IS_ENABLED) || (!is_FAN_ENABLED_Request && is_FAN_IS_ENABLED)) {
		// write new fan state
		if (is_FAN_ENABLED_Request) {
			CLR__LRELAY_FAN_Off;
			SET__LRELAY_FAN_On;
			SET__FAN_IS_ENABLED;
		}
		else {
			SET__LRELAY_FAN_Off;
			CLR__LRELAY_FAN_On;
			CLR__FAN_IS_ENABLED;
		}
		// wait until latching relay is set/reset
		_delay_ms(FAN_RELAY_SWITCH_TIME_MS);
		// set control to break mode on motor controller
		SET__LRELAY_FAN_Off;
		SET__LRELAY_FAN_On;
	}

}


//-------------------------------------------------------------------------------------------
// Process Bias Voltage Supply status
void Reset_Fan_Control(){
	/* this function resets the fan control
	- fan is switched off*/
	FAN_Switch_Counter = 0;
	
	// write new fan state
	SET__LRELAY_FAN_Off;
	CLR__LRELAY_FAN_On;
	CLR__FAN_IS_ENABLED;
	
	// wait until latching relay is set/reset
	_delay_ms(FAN_RELAY_SWITCH_TIME_MS);
	// set control to break mode on motor controller
	SET__LRELAY_FAN_Off;
	SET__LRELAY_FAN_On;
	
}


//-------------------------------------------------------------------------------------------
// Toggle the Error LED depend on detected Errors
void Process_Error_LED_State(){
	/* it makes red LED on board light up when sugar hits the fan */
	if (Check_if_Active_Errors_Output_Disabled() 
	  || is_Status_Curr_Bypass_ON || is_Status_Voltage_Error || is_Status_Current_Error) SET__LED_Error;
	else CLR__LED_Error;
		
}

//-------------------------------------------------------------------------------------------
// Check if any active errors lead to disabling the output!
bool Check_if_Active_Errors_Output_Disabled(){
	/* this function checks if there are any active errors that cause disabling of the output */
	// TODO the Current bypass might need a timeout function at witch output off is initiated!
	if (is_Status_NTC1_Over_Temp || is_Status_NTC2_Over_Temp || is_Status_Bias_Error || is_Status_HEAT_NTC_Over_Temp || is_Status_HEAT_NTC_offline ) {
		// error active
		return true;
	}
	// no error active that turns off output
	return false;
	
}

//-------------------------------------------------------------------------------------------
// Check Disconnect temperature states of NTC 1 & NTC 2
bool Check_Temp_Disconnect_State(){
	/* this function checks if NTC1 or NTC2 are still out of disconnect temperature bounds
	- this is used to decide if IV or transient measurements can be allowed */
	bool is_Overtemperature;
	is_Overtemperature = false;
	
	// check NTC 1 - board temperature is exceeding disconnect temperatures
	if ((AI_NTC_Temp_1 < Temp_NTC1_MIN_DisCon) || (AI_NTC_Temp_1 > Temp_NTC1_MAX_DisCon)) {
		is_Overtemperature = true;
	}
	// check NTC 2 - board temperature
	if ((AI_NTC_Temp_2 < Temp_NTC2_MIN_DisCon) || (AI_NTC_Temp_2 > Temp_NTC2_MAX_DisCon)) {
		is_Overtemperature = true;
	}
			
	return is_Overtemperature;
}
	

//-------------------------------------------------------------------------------------------
// Constant Current Control Initialization
void PV_Mode__Constant_Current_Init(){
	/* function initializes the constant current mode */
	Track_Status = 0; // reset all of it!
	PV_Mode_ACT = PV_Mode_CTR;
	PV_Volt_DAQ_New = 0;
	SET__Track_Status_Dir_Volt_Up;
	Track_Volt_Step_Size = AI_Volt_Scale * LOAD_CurrT_Max_Step_CND;
		
}


//-------------------------------------------------------------------------------------------
// Constant Current Control Main
void PV_Mode__Constant_Current_Main(){
	/* this is the main function controlling the constant current output
	- its called once per cycle to regulate current using voltage control
	- works a bit like the MPPT controller, but regulates specific current instead of max power*/
	
	// check if reached Voc, reverse if so
	if (PV_Volt_DAQ_New > (AI_Volt + (AI_Volt_Scale * 500.0))) {
		PV_Volt_DAQ_New = AI_Volt;
		CLR__Track_Status_Dir_Volt_Up;
	}
	
	// control voltage dependent on current
	if(AI_Curr_Corr < (PV_Curr_Setpoint - AI_Curr_Scale * LOAD_CurrT_Tolerance_CND)) {
		// current too low, voltage too high, reduce voltage
		PV_Volt_DAQ_New = PV_Volt_DAQ_New - Track_Volt_Step_Size;
		if (is_Track_Status_Dir_Volt_Up) {
			CLR__Track_Status_Dir_Volt_Up;
			Track_Volt_Step_Size = Track_Volt_Step_Size * LOAD_CurrT_Step_Reduction_Fact;
		}
		else { // if ya going the correct direction already increase step size...
			Track_Volt_Step_Size = Track_Volt_Step_Size * LOAD_CurrT_Step_Increase_Fact;
		}
	}
	else if(AI_Curr_Corr > (PV_Curr_Setpoint + AI_Curr_Scale * LOAD_CurrT_Tolerance_CND)){
		// current too hight, voltage too low, increase voltage
		PV_Volt_DAQ_New = PV_Volt_DAQ_New + Track_Volt_Step_Size;
		if (!(is_Track_Status_Dir_Volt_Up)) {
			SET__Track_Status_Dir_Volt_Up;
			Track_Volt_Step_Size = Track_Volt_Step_Size * LOAD_CurrT_Step_Reduction_Fact;
		}
		else { // if ya going the correct direction already increase step size to get there faster...
			Track_Volt_Step_Size = Track_Volt_Step_Size * LOAD_CurrT_Step_Increase_Fact;
		}
	}
	else {
		// do nothing, its within range
	}
	// Check Voltage Step Size is in Range
	if (Track_Volt_Step_Size > (AI_Volt_Scale * LOAD_CurrT_Max_Step_CND)) {
		Track_Volt_Step_Size = AI_Volt_Scale * LOAD_CurrT_Max_Step_CND;
	}
	else if (Track_Volt_Step_Size < (AI_Volt_Scale * LOAD_CurrT_Min_Step_CND)) {
		Track_Volt_Step_Size = AI_Volt_Scale * LOAD_CurrT_Min_Step_CND;
	}
	
	// Check Voltage set point is within boundaries
	if (PV_Volt_DAQ_New < 0){
		PV_Volt_DAQ_New = 0;
		SET__Track_Status_Dir_Volt_Up;
	}
		
}

//-------------------------------------------------------------------------------------------
// MPPT Tracker Main - the simple one
void PV_Mode__MPPT_Simple_Init(){
	/* function initializes the simple MPPT tracker mode */
	Track_Status = 0; // reset all of it!
	Track_Power_Old = -1000.0; 
	PV_Active_Load_Power = -1000.0;
	LOAD_MPPT_Update_Counter = LOAD_MPPT_Timer_Update_Delay;
	PV_Mode_ACT = PV_Mode_CTR;
	// get Voc & set optimal start voltage
	PV_Volt_DAQ_New = 0.65 * IV_MEAS__Get_Voc_Set_Optimum_Range();
	SET__Track_Status_Dir_Volt_Up;
	Track_Volt_Step_Size = AI_Volt_Scale * LOAD_MPPT_Max_Step_CND;
	
}

//-------------------------------------------------------------------------------------------
// MPPT Tracker Main - the simple one
void PV_Mode__MPPT_Simple_Main(){
	/* this is the main function that gets the simple hill climber MPPT going */
	
	LOAD_MPPT_Update_Counter--;
	
	if (LOAD_MPPT_Update_Counter > 0) {
		return; // if counter not run down don't to anything
	}
	// reset counter
	LOAD_MPPT_Update_Counter = LOAD_MPPT_Timer_Update_Delay;	
	
	// get new power value  ...!!!....  done at start of Load Control function for power related control
	// PV_Active_Load_Power = AI_Volt * AI_Curr_Corr;
	
	if(is_Track_Status_new_IV_curve){ 
		// new IV curve data, set voltage to new voltage at PMAX
		CLR__Track_Status_new_IV_curve;
		PV_Volt_DAQ_New = get_DAC_Volt_at_Pmax_from_IV();
	}
	else { 
		// normal MPPT, no new IV curves
		// check if V set reached "Voc", go down if so
		if (PV_Volt_DAQ_New > (AI_Volt + (AI_Volt_Scale * 500.0))) {
			// Set point too far over PV voltage - likely over VOC, reset & got down
			PV_Volt_DAQ_New = (AI_Volt * 0.9);
			CLR__Track_Status_Dir_Volt_Up;
			//Track_Volt_Step_Size = AI_Volt_Scale * LOAD_MPPT_Max_Step_CND;
			Track_Volt_Step_Size = Track_Volt_Step_Size * LOAD_MPPT_Step_Increase_Fact; // increase step size to get there faster
			Track_Power_Old = PV_Active_Load_Power; // set new power to old
		}
		else {
			// Calculate the next step and step size
			if (PV_Active_Load_Power > (Track_Power_Old + AI_Volt_Scale * AI_Curr_Scale * LOAD_MPPT_Tolerance_CND)) {
				// power has gone up above noise threshold, lets keep going in same direction
				if (is_Track_Status_Dir_Volt_Up){
					PV_Volt_DAQ_New = PV_Volt_DAQ_New + Track_Volt_Step_Size;
				}
				else{
					PV_Volt_DAQ_New = PV_Volt_DAQ_New - Track_Volt_Step_Size;
				}
				Track_Volt_Step_Size = Track_Volt_Step_Size * LOAD_MPPT_Step_Increase_Fact; // increase step size to get there faster
				Track_Power_Old = PV_Active_Load_Power; // set new power to old
			}
			else if (PV_Active_Load_Power < (Track_Power_Old - AI_Volt_Scale * AI_Curr_Scale * LOAD_MPPT_Tolerance_CND)) {
				// power has gone down under noise threshold, lets change direction and step size
				Track_Volt_Step_Size = Track_Volt_Step_Size * LOAD_MPPT_Step_Reduction_Fact; // reduce step size not overstep it
				if (is_Track_Status_Dir_Volt_Up){
					CLR__Track_Status_Dir_Volt_Up;
					PV_Volt_DAQ_New = PV_Volt_DAQ_New - Track_Volt_Step_Size;
				}
				else{
					SET__Track_Status_Dir_Volt_Up;
					PV_Volt_DAQ_New = PV_Volt_DAQ_New + Track_Volt_Step_Size;
				}
				Track_Power_Old = PV_Active_Load_Power; // set new power to old
			}
			else {
				// power is still withing noise threshold, don't change direction or step size & keep old value
				if (is_Track_Status_Dir_Volt_Up){
					PV_Volt_DAQ_New = PV_Volt_DAQ_New + Track_Volt_Step_Size;
				}
				else{
					PV_Volt_DAQ_New = PV_Volt_DAQ_New - Track_Volt_Step_Size;
				}
			}
		}
	}
	
	// Check Voltage Step Size is in Range
	if (Track_Volt_Step_Size > (AI_Volt_Scale * LOAD_MPPT_Max_Step_CND)) {
		Track_Volt_Step_Size = AI_Volt_Scale * LOAD_MPPT_Max_Step_CND;
	}
	else if (Track_Volt_Step_Size < (AI_Volt_Scale * LOAD_MPPT_Min_Step_CND)) {
		Track_Volt_Step_Size = AI_Volt_Scale * LOAD_MPPT_Min_Step_CND;
	}
	// Check Voltage set point is within boundaries
	if (PV_Volt_DAQ_New < 0){
		SET__Track_Status_Dir_Volt_Up;
		Track_Volt_Step_Size = AI_Volt_Scale * LOAD_MPPT_Max_Step_CND;
		PV_Volt_DAQ_New = Track_Volt_Step_Size;
		Track_Power_Old = PV_Active_Load_Power; // set new power to old
	}
		
}