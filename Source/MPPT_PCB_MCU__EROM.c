/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Martin Bliss
	Version:	1.16
	Date:		08.03.2024
	-----------------------------------------
	ATMega 324 Micro controller support
	-----------------------------------------
	     EEPROM IO function C-Code FILE
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


//===========================================================================================
// EEPROM VARIABLES
EEMEM uint8_t Eeprom_VALID	=  EEMEM_WRITE_VALUE;

//===========================================================================================
// Functions
//===========================================================================================


//-------------------------------------------------------------------------------------------
// Load EEPROM into memory
void EEPROM_LOAD_VAL_CONFIG() {

	EEMEM_Written = EEPROM_READ_UINT8(&Eeprom_VALID);
	if (EEMEM_Written == EEMEM_WRITE_VALUE) {		// load eeprom data
		EEPROM_READ_STRING(&Sample_Name[0], &EROM_Sample_Name[0], NAME_LENGTH);
		SysConfig = EEPROM_READ_UINT8(&EROM_SysConfig);
		Temp_Sensor_Type = EEPROM_READ_UINT8(&EROM_Temp_Sensor_Type);
		F_CPU_CAL = EEPROM_READ_INT32(&EROM_F_CPU_CAL);
		Timer_Control_Match = EEPROM_READ_UINT8(&EROM_Timer_Control_Match);
		Timer_Temp_Meas_Match = EEPROM_READ_UINT8(&EROM_Timer_Temp_Meas_Match);
		IV_Meas_Points = EEPROM_READ_UINT8(&EROM_IV_Meas_Points);
		ADC_Num_Avr_VoltCurr = EEPROM_READ_UINT16(&EROM_ADC_Num_Avr_VoltCurr);
		ADC_Cycl_Avr_VoltCurr = EEPROM_READ_UINT8(&EROM_ADC_Cycl_Avr_VoltCurr);
		ADC_Num_Avr_Other = EEPROM_READ_UINT8(&EROM_ADC_Num_Avr_Other);
		Range_Status_A = EEPROM_READ_UINT8(&EROM_Range_Status_A);
		Range_Volt_ID_Man = EEPROM_READ_UINT8(&EROM_Range_Volt_ID_Man);
		Range_Curr_ID_Man = EEPROM_READ_UINT8(&EROM_Range_Curr_ID_Man);
		Range_Volt_Enable = EEPROM_READ_UINT8(&EROM_Range_Volt_Enable);
		Range_Curr_Enable = EEPROM_READ_UINT8(&EROM_Range_Curr_Enable);
		Range_Volt_Switch_Freq_Ctr_Max = EEPROM_READ_UINT16(&EROM_Range_Volt_Switch_Freq_Ctr_Max);
		Range_Curr_Switch_Freq_Ctr_Max = EEPROM_READ_UINT16(&EROM_Range_Curr_Switch_Freq_Ctr_Max);
		Range_Volt_Switch_Delay_Ctr_Max = EEPROM_READ_UINT8(&EROM_Range_Volt_Switch_Delay_Ctr_Max);
		Range_Curr_Switch_Delay_Ctr_Max = EEPROM_READ_UINT8(&EROM_Range_Curr_Switch_Delay_Ctr_Max);
		IV_Mode = EEPROM_READ_UINT8(&EROM_IV_Mode);
		IV_Cos_Max_Phase = EEPROM_READ_FLOAT(&EROM_IV_Cos_Max_Phase);
		IV_Voc_Overshoot_Fact = EEPROM_READ_FLOAT(&EROM_IV_Voc_Overshoot_Fact);
		IV_Point_Meas_Nu_AVR_Sets = EEPROM_READ_UINT8(&EROM_IV_Point_Meas_Nu_AVR_Sets);
		IV_Point_Meas_Nu_AVR_VoltCurr = EEPROM_READ_UINT8(&EROM_IV_Point_Meas_Nu_AVR_VoltCurr);
		IV_Point_Set_Delay_ms = EEPROM_READ_UINT16(&EROM_IV_Point_Set_Delay_ms);
		PI_CTR_Prop_ID_Man = EEPROM_READ_UINT8(&EROM_PI_CTR_Prop_ID);
		PI_CTR_Int_ID_Man = EEPROM_READ_UINT8(&EROM_PI_CTR_Int_ID);
		CAL_Bias_A0 = EEPROM_READ_FLOAT(&EROM_CAL_Bias_A0);
		CAL_Bias_A1 = EEPROM_READ_FLOAT(&EROM_CAL_Bias_A1);
		CAL_RTD_A0 = EEPROM_READ_FLOAT(&EROM_CAL_RTD_A0);
		CAL_RTD_A1 = EEPROM_READ_FLOAT(&EROM_CAL_RTD_A1);
		CAL_RTD_A2 = EEPROM_READ_FLOAT(&EROM_CAL_RTD_A2);
		CAL_NTC_1_InvGain = EEPROM_READ_FLOAT(&EROM_CAL_NTC_1_InvGain);
		CAL_NTC_1_RSer = EEPROM_READ_FLOAT(&EROM_CAL_NTC_1_RSer);
		CAL_NTC_1_InvR25 = EEPROM_READ_FLOAT(&EROM_CAL_NTC_1_InvR25);
		CAL_NTC_1_InvBeta = EEPROM_READ_FLOAT(&EROM_CAL_NTC_1_InvBeta);
		CAL_NTC_2_InvGain = EEPROM_READ_FLOAT(&EROM_CAL_NTC_2_InvGain);
		CAL_NTC_2_RSer = EEPROM_READ_FLOAT(&EROM_CAL_NTC_2_RSer);
		CAL_NTC_2_InvR25 = EEPROM_READ_FLOAT(&EROM_CAL_NTC_2_InvR25);
		CAL_NTC_2_InvBeta = EEPROM_READ_FLOAT(&EROM_CAL_NTC_2_InvBeta);
		CAL_DAC_A0 = EEPROM_READ_FLOAT(&EROM_CAL_DAC_A0);
		CAL_DAC_A1 = EEPROM_READ_FLOAT(&EROM_CAL_DAC_A1);
		Temp_NTC1_MIN_DisCon = EEPROM_READ_FLOAT(&EROM_Temp_NTC1_MIN_DisCon);
		Temp_NTC1_MAX_DisCon = EEPROM_READ_FLOAT(&EROM_Temp_NTC1_MAX_DisCon);
		Temp_NTC1_MIN_ReCon = EEPROM_READ_FLOAT(&EROM_Temp_NTC1_MIN_ReCon);
		Temp_NTC1_MAX_ReCon = EEPROM_READ_FLOAT(&EROM_Temp_NTC1_MAX_ReCon);
		Temp_NTC2_MIN_DisCon = EEPROM_READ_FLOAT(&EROM_Temp_NTC2_MIN_DisCon);
		Temp_NTC2_MAX_DisCon = EEPROM_READ_FLOAT(&EROM_Temp_NTC2_MAX_DisCon);
		Temp_NTC2_MIN_ReCon = EEPROM_READ_FLOAT(&EROM_Temp_NTC2_MIN_ReCon);
		Temp_NTC2_MAX_ReCon = EEPROM_READ_FLOAT(&EROM_Temp_NTC2_MAX_ReCon);
		BiasV_Range_Min = EEPROM_READ_FLOAT(&EROM_BiasV_Range_Min);
		BiasV_Range_Max = EEPROM_READ_FLOAT(&EROM_BiasV_Range_Max);
		LOAD_MPPT_Max_Step_CND = EEPROM_READ_FLOAT(&EROM_LOAD_MPPT_Max_Step_CND);
		LOAD_MPPT_Min_Step_CND = EEPROM_READ_FLOAT(&EROM_LOAD_MPPT_Min_Step_CND);
		LOAD_MPPT_Step_Increase_Fact = EEPROM_READ_FLOAT(&EROM_LOAD_MPPT_Step_Increase_Fact);
		LOAD_MPPT_Step_Reduction_Fact = EEPROM_READ_FLOAT(&EROM_LOAD_MPPT_Step_Reduction_Fact);
		LOAD_MPPT_Tolerance_CND = EEPROM_READ_FLOAT(&EROM_LOAD_MPPT_Tolerance_CND);
		LOAD_MPPT_Tolerance_CND = LOAD_MPPT_Tolerance_CND * LOAD_MPPT_Tolerance_CND;
		LOAD_MPPT_Timer_Update_Delay = EEPROM_READ_UINT16(&EROM_LOAD_MPPT_Timer_Update_Delay);
		LOAD_CurrT_Max_Step_CND = EEPROM_READ_FLOAT(&EROM_LOAD_CurrT_Max_Step_CND);
		LOAD_CurrT_Min_Step_CND = EEPROM_READ_FLOAT(&EROM_LOAD_CurrT_Min_Step_CND);
		LOAD_CurrT_Step_Increase_Fact = EEPROM_READ_FLOAT(&EROM_LOAD_CurrT_Step_Increase_Fact);
		LOAD_CurrT_Step_Reduction_Fact = EEPROM_READ_FLOAT(&EROM_LOAD_CurrT_Step_Reduction_Fact);
		LOAD_CurrT_Tolerance_CND = EEPROM_READ_FLOAT(&EROM_LOAD_CurrT_Tolerance_CND);
		SysControl = EEPROM_READ_UINT8(&EROM_SysControl);
		PV_Mode_CTR = EEPROM_READ_UINT8(&EROM_PV_Mode_CTR);
		PV_Volt_Setpoint = EEPROM_READ_FLOAT(&EROM_PV_Volt_Setpoint);
		PV_Curr_Setpoint = EEPROM_READ_FLOAT(&EROM_PV_Curr_Setpoint);
		FAN_CTR_State = EEPROM_READ_UINT8(&EROM_FAN_CTR_Mode);
		FAN_NTC1_T_On = EEPROM_READ_FLOAT(&EROM_FAN_NTC1_T_ON);
		FAN_NTC1_T_Off = EEPROM_READ_FLOAT(&EROM_FAN_NTC1_T_OFF);
		FAN_NTC2_T_On = EEPROM_READ_FLOAT(&EROM_FAN_NTC2_T_ON);
		FAN_NTC2_T_Off = EEPROM_READ_FLOAT(&EROM_FAN_NTC2_T_OFF);
		FAN_Power_On = EEPROM_READ_FLOAT(&EROM_FAN_Power_ON);
		FAN_Power_Off = EEPROM_READ_FLOAT(&EROM_FAN_Power_OFF); 
		FAN_Switch_Timer_Max = EEPROM_READ_UINT16(&EROM_FAN_Switch_Timer_Max);
		TEMP_MAX_Heat_NTC =  EEPROM_READ_FLOAT(&EROM_TEMP_MAX_Heat_NTC);
		TEMP_Heat_dissipation_devices = EEPROM_READ_UINT8(&EROM_TEMP_Heat_dissipation_devices);
	}
	else { // load default data, eeprom empty
		Copy_String(Sample_Name, "NoErom!");
		SysConfig = PCBconfig_TEMP_is_enabled; // see board config definitions section in Main.h
		Temp_Sensor_Type = PCBconfig_TEMP_Sensor_Type;
		F_CPU_CAL = F_CPU;
		Timer_Control_Match = CONTROL_TIMER_MULT;
		Timer_Temp_Meas_Match = TEMP_MEAS_TIMER_MUILT;
		IV_Meas_Points = 10;
		ADC_Num_Avr_VoltCurr = 1;
		ADC_Cycl_Avr_VoltCurr = 1;
		ADC_Num_Avr_Other = 1;
		Range_Status_A = 0;
		Range_Volt_ID_Man = Volt_range_max_ID;
		Range_Curr_ID_Man = Curr_range_max_ID;
		Range_Volt_Enable = 255;
		Range_Curr_Enable = 255;
		Range_Volt_Switch_Freq_Ctr_Max = 250;
		Range_Curr_Switch_Freq_Ctr_Max = 250;
		Range_Volt_Switch_Freq_Ctr_Max = 100;
		Range_Curr_Switch_Freq_Ctr_Max = 100;
		IV_Mode = 0;
		IV_Cos_Max_Phase = 1.57079;
		IV_Voc_Overshoot_Fact = 1.01;
		IV_Point_Meas_Nu_AVR_Sets = 1;
		IV_Point_Meas_Nu_AVR_VoltCurr = 1;
		IV_Point_Set_Delay_ms = 5;
		PI_CTR_Prop_ID_Man = 0;
		PI_CTR_Int_ID_Man = 0;
		CAL_Bias_A0 = 0.0;
		CAL_Bias_A1 = 1.0;
		CAL_RTD_A0 = 0;
		CAL_RTD_A1 = 1;
		CAL_RTD_A2 = 0;
		CAL_NTC_1_InvGain = 0.465116;
		CAL_NTC_1_RSer = 5e4;
		CAL_NTC_1_InvR25 = 1.0E-04;
		CAL_NTC_1_InvBeta = 2.91121E-04;
		CAL_NTC_2_InvGain = 0.465116;
		CAL_NTC_2_RSer = 5e4;
		CAL_NTC_2_InvR25 = 1e-4;
		CAL_NTC_2_InvBeta = 2.50752E-04;
		CAL_DAC_A0 = 648.0;
		CAL_DAC_A1 = 1.0;
		Temp_NTC1_MIN_DisCon = 0.0;
		Temp_NTC1_MAX_DisCon = 40.0;
		Temp_NTC1_MIN_ReCon = 5.0;
		Temp_NTC1_MAX_ReCon = 35.0;
		Temp_NTC2_MIN_DisCon = 0.0;
		Temp_NTC2_MAX_DisCon = 80.0;
		Temp_NTC2_MIN_ReCon = 5.0;
		Temp_NTC2_MAX_ReCon = 60.0;
		BiasV_Range_Min = 15000;
		BiasV_Range_Max = 45000;
		LOAD_MPPT_Max_Step_CND = 1000.0;
		LOAD_MPPT_Min_Step_CND = 10.0;
		LOAD_MPPT_Step_Increase_Fact = 1.2;
		LOAD_MPPT_Step_Reduction_Fact = 0.6;
		LOAD_MPPT_Tolerance_CND = 1000.0 * 1000.0;
		LOAD_MPPT_Timer_Update_Delay = 1;
		LOAD_CurrT_Max_Step_CND = 1000.0;
		LOAD_CurrT_Min_Step_CND = 1.0;
		LOAD_CurrT_Step_Increase_Fact = 1.2;
		LOAD_CurrT_Step_Reduction_Fact = 0.6;
		LOAD_CurrT_Tolerance_CND = 10.0;
		SysControl = 0;
		PV_Mode_CTR = 0;
		PV_Volt_Setpoint = 0.0;
		PV_Curr_Setpoint = 0.0;
		FAN_CTR_State = 0;
		FAN_NTC1_T_On = 30.0;
		FAN_NTC1_T_Off = 25.0;
		FAN_NTC2_T_On = 30.0;
		FAN_NTC2_T_Off = 25.0;
		FAN_Power_On = 10.0;
		FAN_Power_Off = 5.0;
		FAN_Switch_Timer_Max = 2400;
		TEMP_MAX_Heat_NTC = 70.0;
		TEMP_Heat_dissipation_devices = PCBconfig_TEMP_Heat_Monitoring_devices;
	}

	// Calculate new variables
	ADC_Avr_Mult_VoltCurr = 1.0 / ((float) ADC_Num_Avr_VoltCurr * (float) ADC_Cycl_Avr_VoltCurr);
	ADC_Avr_Mult_Other = 1.0 / ((float) ADC_Num_Avr_Other);
	Trs_Meas_Time_us = (1 / ((float) (F_CPU_CAL)/1e6)) * Trs_Meas_Time_cycls;
	Trs_Delay_Res_us = (1 / ((float) (F_CPU_CAL)/1e6)) * Trs_Delay_Res_cycls;
	
}

//-------------------------------------------------------------------------------------------
// Load Timer_1 value
uint16_t EEROM_LOAD_TIMER_1_CM (){
	uint16_t Timer_1_value;
	
	if (EEMEM_Written == EEMEM_WRITE_VALUE) {		// load value from eeprom data
		Timer_1_value = EEPROM_READ_UINT16(&EROM_Loop_Timer_1_Comp_Match);
	}
	else {
		Timer_1_value = TIMER_1_COMP_MATCH; // default value
	}
	
	return Timer_1_value;
	
}

//-------------------------------------------------------------------------------------------
// Load Voltage Range Values
void EEPROM_LOAD_VOLTAGE_RANGE (uint8_t Range_ID){
	/* this function loads calibration for a specific measurement range */
	if (EEMEM_Written == EEMEM_WRITE_VALUE) {		// load value from EEPROM data
		if (Range_ID == 0) { // 1V Range
			AI_Volt_Offset = EEPROM_READ_FLOAT(&EROM_V_R1_A0);
			AI_Volt_Scale = EEPROM_READ_FLOAT(&EROM_V_R1_A1);
			AI_Volt_R_Leak = EEPROM_READ_FLOAT(&EROM_V_R1_R_Leak);
			AI_Volt_Range_Val = EEPROM_READ_FLOAT(&EROM_V_R1_Val_Nom);
		}
		else if (Range_ID == 1) { // 3V range
			AI_Volt_Offset = EEPROM_READ_FLOAT(&EROM_V_R2_A0);
			AI_Volt_Scale = EEPROM_READ_FLOAT(&EROM_V_R2_A1);
			AI_Volt_R_Leak = EEPROM_READ_FLOAT(&EROM_V_R2_R_Leak);
			AI_Volt_Range_Val = EEPROM_READ_FLOAT(&EROM_V_R2_Val_Nom);
		}
		else if (Range_ID == 2) { // 10V range
			AI_Volt_Offset = EEPROM_READ_FLOAT(&EROM_V_R3_A0);
			AI_Volt_Scale = EEPROM_READ_FLOAT(&EROM_V_R3_A1);
			AI_Volt_R_Leak = EEPROM_READ_FLOAT(&EROM_V_R3_R_Leak);
			AI_Volt_Range_Val = EEPROM_READ_FLOAT(&EROM_V_R3_Val_Nom);
		}
		else if (Range_ID == 3) { // 30V range
			AI_Volt_Offset = EEPROM_READ_FLOAT(&EROM_V_R4_A0);
			AI_Volt_Scale = EEPROM_READ_FLOAT(&EROM_V_R4_A1);
			AI_Volt_R_Leak = EEPROM_READ_FLOAT(&EROM_V_R4_R_Leak);
			AI_Volt_Range_Val = EEPROM_READ_FLOAT(&EROM_V_R4_Val_Nom);
		}
		else { // 100V range
			AI_Volt_Offset = EEPROM_READ_FLOAT(&EROM_V_R5_A0);
			AI_Volt_Scale = EEPROM_READ_FLOAT(&EROM_V_R5_A1);
			AI_Volt_R_Leak = EEPROM_READ_FLOAT(&EROM_V_R5_R_Leak);
			AI_Volt_Range_Val = EEPROM_READ_FLOAT(&EROM_V_R5_Val_Nom);
		}
	}
	else { // EEPROM not written - value not valid
		AI_Volt_Offset = 0.0;
		AI_Volt_Scale = 1.0;
		AI_Volt_R_Leak = 0;
		AI_Volt_Range_Val = 0.0;
	}
	// overwrite scale and offset in calibration mode
	if ((is_SysControl_Cal_Mode) && (is_R_Stat_Cal_Mode_Volt_Act)) {
		AI_Volt_Offset = 0.0;
		AI_Volt_Scale = 1.0;
	}
		
}

//-------------------------------------------------------------------------------------------
// Load Current Range Values
void EEPROM_LOAD_CURRENT_RANGE (uint8_t Range_ID){
	/* this function loads calibration for a specific measurement range */
	if (EEMEM_Written == EEMEM_WRITE_VALUE) {		// load value from EEPROM data
		if(Range_ID == 0){ // 50mA or 0.5mA range
			AI_Curr_Offset = EEPROM_READ_FLOAT(&EROM_C_R1_A0);
			AI_Curr_Scale = EEPROM_READ_FLOAT(&EROM_C_R1_A1);
			AI_Curr_Range_Val = EEPROM_READ_FLOAT(&EROM_C_R1_Val_Nom);
		}
		else if (Range_ID == 1) { // 150mA or 1.5mA range
			AI_Curr_Offset = EEPROM_READ_FLOAT(&EROM_C_R2_A0);
			AI_Curr_Scale = EEPROM_READ_FLOAT(&EROM_C_R2_A1);
			AI_Curr_Range_Val = EEPROM_READ_FLOAT(&EROM_C_R2_Val_Nom);
		}
		else if (Range_ID == 2) { // 500mA or 5mA range
			AI_Curr_Offset = EEPROM_READ_FLOAT(&EROM_C_R3_A0);
			AI_Curr_Scale = EEPROM_READ_FLOAT(&EROM_C_R3_A1);
			AI_Curr_Range_Val = EEPROM_READ_FLOAT(&EROM_C_R3_Val_Nom);
		}
		else if (Range_ID == 3) { // 1.5A or 15mA range
			AI_Curr_Offset = EEPROM_READ_FLOAT(&EROM_C_R4_A0);
			AI_Curr_Scale = EEPROM_READ_FLOAT(&EROM_C_R4_A1);
			AI_Curr_Range_Val = EEPROM_READ_FLOAT(&EROM_C_R4_Val_Nom);
		}
		else if (Range_ID == 4) { // 5A or 50mA range
			AI_Curr_Offset = EEPROM_READ_FLOAT(&EROM_C_R5_A0);
			AI_Curr_Scale = EEPROM_READ_FLOAT(&EROM_C_R5_A1);
			AI_Curr_Range_Val = EEPROM_READ_FLOAT(&EROM_C_R5_Val_Nom);
		}
		else { // 15A or 150mA range
			AI_Curr_Offset = EEPROM_READ_FLOAT(&EROM_C_R6_A0);
			AI_Curr_Scale = EEPROM_READ_FLOAT(&EROM_C_R6_A1);
			AI_Curr_Range_Val = EEPROM_READ_FLOAT(&EROM_C_R6_Val_Nom);
		}
	}
	else { // EEPROM not written - value not valid
		AI_Curr_Offset = 0.0;
		AI_Curr_Scale = 1.0;
		AI_Curr_Range_Val = 0.0;
	}
	// overwrite scale and offset in calibration mode
	if ((is_SysControl_Cal_Mode) && (is_R_Stat_Cal_Mode_Curr_Act)) {
		AI_Curr_Offset = 0.0;
		AI_Curr_Scale = 1.0;
	}
	
}


//-------------------------------------------------------------------------------------------
// Calibration mode - Set Range Scale
void EEPROM_Range_Cal_Set_Scale(float Scale) {
	/* writes the scale value of the active calibration range to eeprom */
	if (is_Status_Calibrate) {
		if (is_R_Stat_Cal_Mode_Volt_Act) {
			// Set Scale of active voltage range
			if(Range_Volt_ID_Act == 0){ // 1V range
				EEPROM_WRITE_FLOAT(Scale, &EROM_V_R1_A1);	// Write EEPROM value
			}
			else if (Range_Volt_ID_Act == 1) { // 3V range
				EEPROM_WRITE_FLOAT(Scale, &EROM_V_R2_A1);	// Write EEPROM value
			}
			else if (Range_Volt_ID_Act == 2) { // 10V range
				EEPROM_WRITE_FLOAT(Scale, &EROM_V_R3_A1);	// Write EEPROM value
			}
			else if (Range_Volt_ID_Act == 3) { // 30V range
				EEPROM_WRITE_FLOAT(Scale, &EROM_V_R4_A1);	// Write EEPROM value
			}
			else if (Range_Volt_ID_Act == 4) { // 100V range
				EEPROM_WRITE_FLOAT(Scale, &EROM_V_R5_A1);	// Write EEPROM value
			}
		}
		else if(is_R_Stat_Cal_Mode_Curr_Act) {
			// Set Scale of active current range
			if(Range_Curr_ID_Act == 0){ // 50mA or 0.5mA range
				EEPROM_WRITE_FLOAT(Scale, &EROM_C_R1_A1);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 1) { // 150mA or 1.5mA range
				EEPROM_WRITE_FLOAT(Scale, &EROM_C_R2_A1);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 2) { // 500mA or 5mA range
				EEPROM_WRITE_FLOAT(Scale, &EROM_C_R3_A1);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 3) { // 1.5A or 15mA range
				EEPROM_WRITE_FLOAT(Scale, &EROM_C_R4_A1);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 4) { // 5A or 50mA range 
				EEPROM_WRITE_FLOAT(Scale, &EROM_C_R5_A1);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 5) { // 15A or 150mA range
				EEPROM_WRITE_FLOAT(Scale, &EROM_C_R6_A1);	// Write EEPROM value
			}
		}
	}
}


//-------------------------------------------------------------------------------------------
// Calibration mode - Set Range Offset
void EEPROM_Range_Cal_Set_Offset(float Offset) {
	/* writes the offset value of the active calibration range to eeprom */
	if (is_Status_Calibrate) {
		if (is_R_Stat_Cal_Mode_Volt_Act) {
			if(Range_Volt_ID_Act == 0){ // 1V range
				EEPROM_WRITE_FLOAT(Offset, &EROM_V_R1_A0);	// Write EEPROM value
			}
			else if (Range_Volt_ID_Act == 1) { // 3V range
				EEPROM_WRITE_FLOAT(Offset, &EROM_V_R2_A0);	// Write EEPROM value
			}
			else if (Range_Volt_ID_Act == 2) { // 10V range
				EEPROM_WRITE_FLOAT(Offset, &EROM_V_R3_A0);	// Write EEPROM value
			}
			else if (Range_Volt_ID_Act == 3) { // 30V range
				EEPROM_WRITE_FLOAT(Offset, &EROM_V_R4_A0);	// Write EEPROM value
			}
			else if (Range_Volt_ID_Act == 4){ // 100V range
				EEPROM_WRITE_FLOAT(Offset, &EROM_V_R5_A0);	// Write EEPROM value
			}
		}
		else if(is_R_Stat_Cal_Mode_Curr_Act) {
			if(Range_Curr_ID_Act == 0){ // 50mA or 0.5mA range
				EEPROM_WRITE_FLOAT(Offset, &EROM_C_R1_A0);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 1) { // 150mA or 1.5mA range
				EEPROM_WRITE_FLOAT(Offset, &EROM_C_R2_A0);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 2) { // 500mA or 5mA range
				EEPROM_WRITE_FLOAT(Offset, &EROM_C_R3_A0);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 3) { // 1.5A or 15mA range
				EEPROM_WRITE_FLOAT(Offset, &EROM_C_R4_A0);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 4) { // 5A or 50mA range
				EEPROM_WRITE_FLOAT(Offset, &EROM_C_R5_A0);	// Write EEPROM value
			}
			else if (Range_Curr_ID_Act == 5) { // 15A or 150mA range
				EEPROM_WRITE_FLOAT(Offset, &EROM_C_R6_A0);	// Write EEPROM value
			}
		}
	}
}


//-------------------------------------------------------------------------------------------
// Calibration mode - Read Range Scale
float EEPROM_Range_Cal_Get_Scale() {
	/* returns/reads the scale of the active calibration range from eeprom */
	float EEPROM_Val_FT;
	EEPROM_Val_FT = -1.0;	
	if (is_Status_Calibrate) {
		if (is_R_Stat_Cal_Mode_Volt_Act) {
			if(Range_Volt_ID_Act == 0){ // 1V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R1_A1);	// read EEPROM value
			}
			else if (Range_Volt_ID_Act == 1) { // 3V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R2_A1);	// read EEPROM value
			}
			else if (Range_Volt_ID_Act == 2) { // 10V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R3_A1);	// read EEPROM value
			}
			else if (Range_Volt_ID_Act == 3) { // 30V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R4_A1);	// read EEPROM value
			}
			else if (Range_Volt_ID_Act == 4) { // 100V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R5_A1);	// read EEPROM value
			}
		}
		else if(is_R_Stat_Cal_Mode_Curr_Act) {
			if(Range_Curr_ID_Act == 0){ // 50mA or 0.5mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R1_A1);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 1) { // 150mA or 1.5mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R2_A1);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 2) { // 500mA or 5mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R3_A1);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 3) { // 1.5A or 15mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R4_A1);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 4) { // 5A or 50mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R5_A1);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 5) { // 15A or 150mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R6_A1);	// read EEPROM value
			}
		}
	}
	return EEPROM_Val_FT;
}


//-------------------------------------------------------------------------------------------
// Calibration mode - Read Range Offset
float EEPROM_Range_Cal_Get_Offset() {
	/* returns/reads the offset of the active calibration range from eeprom */
	float EEPROM_Val_FT;
	EEPROM_Val_FT = -1;
	if (is_Status_Calibrate) {
		if (is_R_Stat_Cal_Mode_Volt_Act) {
			if(Range_Volt_ID_Act == 0){ // 1V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R1_A0);	// read EEPROM value
			}
			else if (Range_Volt_ID_Act == 1) { // 3V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R2_A0);	// read EEPROM value
			}
			else if (Range_Volt_ID_Act == 2) { // 10V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R3_A0);	// read EEPROM value
			}
			else if (Range_Volt_ID_Act == 3) { // 30V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R4_A0);	// read EEPROM value
			}
			else if (Range_Volt_ID_Act == 4) { // 100V range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_V_R5_A0);	// read EEPROM value
			}
		}
		else if(is_R_Stat_Cal_Mode_Curr_Act) {
			if(Range_Curr_ID_Act == 0){ // 50mA or 0.5mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R1_A0);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 1) { // 150mA or 1.5mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R2_A0);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 2) { // 500mA or 5mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R3_A0);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 3) { // 1.5A or 15mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R4_A0);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 4) { // 5A or 50mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R5_A0);	// read EEPROM value
			}
			else if (Range_Curr_ID_Act == 5) { // 15A or 150mA range
				EEPROM_Val_FT = EEPROM_READ_FLOAT(&EROM_C_R6_A0);	// read EEPROM value
			}
		}
	}
	return EEPROM_Val_FT;
}

// ##########################################################################################
//-------------------------------------------------------------------------------------------
// EEPROM COM IO
//-------------------------------------------------------------------------------------------
// WRITE TO EEPROM VIA UART
void EEPROM_ACCESS_UART(char * EEPROM_Command, uint8_t EEPROM_Write){

	uint8_t EEPROM_Adr;
	uint32_t EEPROM_Val_U32;
	char EEPROM_Val_Char[UART_Value_LENGH - EEPROM_ADR_LENGH];
	uint8_t EEPROM_Com_Str_Pos;
	uint8_t EEPROM_Val_Str_Pos;

	EEPROM_Val_U32 = 0;

	// Check for write or read ACCESS
	if (EEPROM_Write == 1) {
	// Extract EEPROM Address
		EEPROM_Com_Str_Pos = 0;
 		EEPROM_Val_Str_Pos = 0;
		while (EEPROM_Command[EEPROM_Com_Str_Pos] != UART_SEP_CHAR) {
			if (EEPROM_Command[EEPROM_Com_Str_Pos] == '\0') {	//Check if end of string
				goto EEPROM_WRITE_UART_ADR_ERROR;			
			}
			EEPROM_Val_Char[EEPROM_Com_Str_Pos] = EEPROM_Command[EEPROM_Com_Str_Pos];
			EEPROM_Com_Str_Pos++;
			if(EEPROM_Com_Str_Pos > EEPROM_ADR_LENGH) {
				EEPROM_WRITE_UART_ADR_ERROR:;			
				UART_WriteString ("ERR_EEPROM_ADR!");
				goto EEPROM_WRITE_EEROM_END;
			}
		}
		EEPROM_Val_Char[EEPROM_Com_Str_Pos] = '\0';
		EEPROM_Adr = (uint8_t) atoi(EEPROM_Val_Char);	

		// Extract VALUE to be written to EEPROM
		EEPROM_Com_Str_Pos++;		// skip address separation character
		while (EEPROM_Command[EEPROM_Com_Str_Pos] != '\0') {	
			EEPROM_Val_Char[EEPROM_Val_Str_Pos] = EEPROM_Command[EEPROM_Com_Str_Pos];
			EEPROM_Com_Str_Pos++;
			EEPROM_Val_Str_Pos++;
			if(EEPROM_Val_Str_Pos > UART_Value_LENGH - EEPROM_ADR_LENGH) {
				UART_WriteString ("ERR_EEPROM_VAL!");
				goto EEPROM_WRITE_EEROM_END;
			}
		}
		EEPROM_Val_Char[EEPROM_Val_Str_Pos] = '\0';
		
		// Write Output string header
		itoa (EEPROM_Adr, EEPROM_Command, 10);
		AddEnd_String(EEPROM_Command, "	");
	}
	else {
		// Extract Address to READ
		EEPROM_Adr = (uint8_t) atoi(EEPROM_Command);
		AddEnd_String(EEPROM_Command, "	");
	}
	
	//---------------------------
	// WRITE uint8 Eeprom_VALID
	if (EEPROM_Adr == 0) {
		COM_EROM_ACCESS_UINT8(&Eeprom_VALID, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// ID & Config Variables
	//---------------------------
	// WRITE string EROM_Sample_Name
	if (EEPROM_Adr == 1) {
		char temp[NAME_LENGTH];
		if (EEPROM_Write == 1) {
			Copy_String(temp, EEPROM_Val_Char);
			EEPROM_WRITE_STRING(&temp[0], &EROM_Sample_Name[0], NAME_LENGTH-1);	// Write EEPROM value
		}
		EEPROM_READ_STRING(&temp[0], &EROM_Sample_Name[0], NAME_LENGTH-1);	// read EEPROM value
		AddEnd_String(EEPROM_Command, temp);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_SysConfig
	if (EEPROM_Adr == 2) {
		COM_EROM_ACCESS_UINT8(&EROM_SysConfig, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_Temp_Sensor_Type
	if (EEPROM_Adr == 3) {
		COM_EROM_ACCESS_UINT8(&EROM_Temp_Sensor_Type, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// Timing Variables
	//---------------------------
	// WRITE uint32 EROM_F_CPU_CAL
	if (EEPROM_Adr == 5) {
		if (EEPROM_Write == 1) {
			EEPROM_Val_U32 = (uint32_t) atol (EEPROM_Val_Char);
			EEPROM_WRITE_INT32(EEPROM_Val_U32, &EROM_F_CPU_CAL);
		}
		EEPROM_Val_U32 = EEPROM_READ_INT32(&EROM_F_CPU_CAL);
		ltoa(EEPROM_Val_U32, EEPROM_Val_Char, 10);
		AddEnd_String(EEPROM_Command, EEPROM_Val_Char);
		goto EEPROM_WRITE_EEROM_END;
	}
	//---------------------------
	// WRITE uint16 EROM_Loop_Timer_1_Comp_Match
	if (EEPROM_Adr == 6) {
		COM_EROM_ACCESS_UINT16(&EROM_Loop_Timer_1_Comp_Match, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_Timer_Control_Match
	if (EEPROM_Adr == 7) {
		COM_EROM_ACCESS_UINT8(&EROM_Timer_Control_Match, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_Timer_Temp_Meas_Match
	if (EEPROM_Adr == 8) {
		COM_EROM_ACCESS_UINT8(&EROM_Timer_Temp_Meas_Match, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// Config Variables
	//---------------------------
	// WRITE uint16 EROM_ADC_Num_Avr_VoltCurr
	if (EEPROM_Adr == 10) {
		COM_EROM_ACCESS_UINT16(&EROM_ADC_Num_Avr_VoltCurr, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_ADC_Num_Avr_Other
	if (EEPROM_Adr == 11) {
		COM_EROM_ACCESS_UINT8(&EROM_ADC_Num_Avr_Other, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_ADC_Cycl_Avr_VoltCurr
	if (EEPROM_Adr == 12) {
		COM_EROM_ACCESS_UINT8(&EROM_ADC_Cycl_Avr_VoltCurr, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
		
	//###########################
	// Range control Variables
	//---------------------------
	// WRITE uint8_t EROM_Range_Status_A
	if (EEPROM_Adr == 20) {
		COM_EROM_ACCESS_UINT8(&EROM_Range_Status_A, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8_t EROM_Range_Volt_ID_Man
	if (EEPROM_Adr == 21) {
		COM_EROM_ACCESS_UINT8(&EROM_Range_Volt_ID_Man, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8_t EROM_Range_Curr_ID_Man
	if (EEPROM_Adr == 22) {
		COM_EROM_ACCESS_UINT8(&EROM_Range_Curr_ID_Man, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint16 EROM_Range_Volt_Switch_Freq_Ctr_Max
	if (EEPROM_Adr == 23) {
		COM_EROM_ACCESS_UINT16(&EROM_Range_Volt_Switch_Freq_Ctr_Max, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint16 EROM_Range_Curr_Switch_Freq_Ctr_Max
	if (EEPROM_Adr == 24) {
		COM_EROM_ACCESS_UINT16(&EROM_Range_Curr_Switch_Freq_Ctr_Max, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint16 EROM_Range_Volt_Switch_Delay_Ctr_Max
	if (EEPROM_Adr == 85) {
		COM_EROM_ACCESS_UINT8(&EROM_Range_Volt_Switch_Delay_Ctr_Max, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint16 EROM_Range_Curr_Switch_Delay_Ctr_Max
	if (EEPROM_Adr == 86) {
		COM_EROM_ACCESS_UINT8(&EROM_Range_Curr_Switch_Delay_Ctr_Max, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// VOLTAGE Range Variables
	//-------V Range 1-----------
	// WRITE float EROM_V_R1_A0
	if (EEPROM_Adr == 25) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R1_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R1_A1
	if (EEPROM_Adr == 26) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R1_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R1_Val_Nom
	if (EEPROM_Adr == 27) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R1_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R1_R_Leak
	if (EEPROM_Adr == 28) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R1_R_Leak, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------V Range 2-----------
	// WRITE float EROM_V_R2_A0
	if (EEPROM_Adr == 30) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R2_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R2_A1
	if (EEPROM_Adr == 31) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R2_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R2_Val_Nom
	if (EEPROM_Adr == 32) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R2_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R2_R_Leak
	if (EEPROM_Adr == 33) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R2_R_Leak, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------V Range 3-----------
	// WRITE float EROM_V_R3_A0
	if (EEPROM_Adr == 35) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R3_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R3_A1
	if (EEPROM_Adr == 36) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R3_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R3_Val_Nom
	if (EEPROM_Adr == 37) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R3_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R3_R_Leak
	if (EEPROM_Adr == 38) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R3_R_Leak, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------V Range 4-----------
	// WRITE float EROM_V_R4_A0
	if (EEPROM_Adr == 40) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R4_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R4_A1
	if (EEPROM_Adr == 41) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R4_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R4_Val_Nom
	if (EEPROM_Adr == 42) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R4_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R4_R_Leak
	if (EEPROM_Adr == 43) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R4_R_Leak, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------V Range 5-----------
	// WRITE float EROM_V_R5_A0
	if (EEPROM_Adr == 45) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R5_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R5_A1
	if (EEPROM_Adr == 46) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R5_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R5_Val_Nom
	if (EEPROM_Adr == 47) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R5_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_V_R5_R_Leak
	if (EEPROM_Adr == 48) {
		COM_EROM_ACCESS_FLOAT(&EROM_V_R5_R_Leak, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// CURRENT Range Variables
	//-------C Range 1-----------
	// WRITE float EROM_C_R1_A0
	if (EEPROM_Adr == 55) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R1_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R1_A1
	if (EEPROM_Adr == 56) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R1_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R1_Val_Nom
	if (EEPROM_Adr == 57) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R1_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------C Range 2-----------
	// WRITE float EROM_C_R2_A0
	if (EEPROM_Adr == 60) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R2_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R2_A1
	if (EEPROM_Adr == 61) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R2_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R2_Val_Nom
	if (EEPROM_Adr == 62) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R2_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------C Range 3-----------
	// WRITE float EROM_C_R3_A0
	if (EEPROM_Adr == 65) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R3_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R3_A1
	if (EEPROM_Adr == 66) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R3_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R3_Val_Nom
	if (EEPROM_Adr == 67) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R3_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------C Range 4-----------
	// WRITE float EROM_C_R4_A0
	if (EEPROM_Adr == 70) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R4_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R4_A1
	if (EEPROM_Adr == 71) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R4_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R4_Val_Nom
	if (EEPROM_Adr == 72) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R4_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------C Range 5-----------
	// WRITE float EROM_C5_R1_A0
	if (EEPROM_Adr == 75) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R5_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R5_A1
	if (EEPROM_Adr == 76) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R5_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R5_Val_Nom
	if (EEPROM_Adr == 77) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R5_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------C Range 6-----------
	// WRITE float EROM_C_R6_A0
	if (EEPROM_Adr == 80) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R6_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R6_A1
	if (EEPROM_Adr == 81) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R6_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_C_R6_Val_Nom
	if (EEPROM_Adr == 82) {
		COM_EROM_ACCESS_FLOAT(&EROM_C_R6_Val_Nom, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//------Range Enabled-------
	// WRITE uint8 EROM_Range_Volt_Enable
	if (EEPROM_Adr == 83) {
		COM_EROM_ACCESS_UINT8(&EROM_Range_Volt_Enable, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_Range_Curr_Enable
	if (EEPROM_Adr == 84) {
		COM_EROM_ACCESS_UINT8(&EROM_Range_Curr_Enable, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// Other Calibration Variables
	//---------------------------
	//-------Bias Voltage--------
	// WRITE float EROM_CAL_Bias_A0
	if (EEPROM_Adr == 90) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_Bias_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_Bias_A1
	if (EEPROM_Adr == 91) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_Bias_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-------RTD PT100 -----------
	// WRITE float EROM_CAL_RTD_A0
	if (EEPROM_Adr == 95) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_RTD_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_RTD_A1
	if (EEPROM_Adr == 96) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_RTD_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_RTD_A2
	if (EEPROM_Adr == 97) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_RTD_A2, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-----------NTC 1-----------
	// WRITE float EROM_CAL_NTC_1_InvGain
	if (EEPROM_Adr == 100) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_NTC_1_InvGain, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_NTC_1_RSer
	if (EEPROM_Adr == 101) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_NTC_1_RSer, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_NTC_1_InvR25
	if (EEPROM_Adr == 102) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_NTC_1_InvR25, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_NTC_1_InvBeta
	if (EEPROM_Adr == 103) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_NTC_1_InvBeta, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-----------NTC 2-----------
	// WRITE float EROM_CAL_NTC_2_InvGain
	if (EEPROM_Adr == 105) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_NTC_2_InvGain, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_NTC_2_RSer
	if (EEPROM_Adr == 106) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_NTC_2_RSer, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_NTC_2_InvR25
	if (EEPROM_Adr == 107) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_NTC_2_InvR25, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_NTC_2_InvBeta
	if (EEPROM_Adr == 108) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_NTC_2_InvBeta, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//-----------DAC-----------
	// WRITE float EROM_CAL_DAC_A0
	if (EEPROM_Adr == 110) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_DAC_A0, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_CAL_DAC_A1
	if (EEPROM_Adr == 111) {
		COM_EROM_ACCESS_FLOAT(&EROM_CAL_DAC_A1, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// PI Controller Variables
	//---------------------------
	// WRITE uint8 EROM_PI_CTR_Prop_ID
	if (EEPROM_Adr == 115) {
		COM_EROM_ACCESS_UINT8(&EROM_PI_CTR_Prop_ID, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_PI_CTR_Int_ID
	if (EEPROM_Adr == 116) {
		COM_EROM_ACCESS_UINT8(&EROM_PI_CTR_Int_ID, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// Temperature limit Variables
	//---------------------------
	//----------- NTC1 ----------
	// WRITE float EROM_Temp_NTC1_MIN_DisCon
	if (EEPROM_Adr == 125) {
		COM_EROM_ACCESS_FLOAT(&EROM_Temp_NTC1_MIN_DisCon, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_Temp_NTC1_MAX_DisCon
	if (EEPROM_Adr == 126) {
		COM_EROM_ACCESS_FLOAT(&EROM_Temp_NTC1_MAX_DisCon, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_Temp_NTC1_MIN_ReCon
	if (EEPROM_Adr == 127) {
		COM_EROM_ACCESS_FLOAT(&EROM_Temp_NTC1_MIN_ReCon, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_Temp_NTC1_MAX_ReCon
	if (EEPROM_Adr == 128) {
		COM_EROM_ACCESS_FLOAT(&EROM_Temp_NTC1_MAX_ReCon, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	//----------- NTC2 ----------
	// WRITE float EROM_Temp_NTC2_MIN_DisCon
	if (EEPROM_Adr == 130) {
		COM_EROM_ACCESS_FLOAT(&EROM_Temp_NTC2_MIN_DisCon, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_Temp_NTC2_MAX_DisCon
	if (EEPROM_Adr == 131) {
		COM_EROM_ACCESS_FLOAT(&EROM_Temp_NTC2_MAX_DisCon, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_Temp_NTC2_MIN_ReCon
	if (EEPROM_Adr == 132) {
		COM_EROM_ACCESS_FLOAT(&EROM_Temp_NTC2_MIN_ReCon, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_Temp_NTC2_MAX_ReCon
	if (EEPROM_Adr == 133) {
		COM_EROM_ACCESS_FLOAT(&EROM_Temp_NTC2_MAX_ReCon, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// Bias Voltage limit Variables
	//---------------------------
	// WRITE float EROM_BiasV_Range_Min
	if (EEPROM_Adr == 135) {
		COM_EROM_ACCESS_FLOAT(&EROM_BiasV_Range_Min, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_BiasV_Range_Max
	if (EEPROM_Adr == 136) {
		COM_EROM_ACCESS_FLOAT(&EROM_BiasV_Range_Max, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// IV measurement control variables
	//---------------------------
	// WRITE uint8 EROM_IV_Meas_Points
	if (EEPROM_Adr == 140) {
		COM_EROM_ACCESS_UINT8(&EROM_IV_Meas_Points, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_IV_Mode
	if (EEPROM_Adr == 141) {
		COM_EROM_ACCESS_UINT8(&EROM_IV_Mode, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_IV_Cos_Max_Phase
	if (EEPROM_Adr == 142) {
		COM_EROM_ACCESS_FLOAT(&EROM_IV_Cos_Max_Phase, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_IV_Voc_Overshoot_Fact
	if (EEPROM_Adr == 143) {
		COM_EROM_ACCESS_FLOAT(&EROM_IV_Voc_Overshoot_Fact, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_IV_Point_Set_Delay_ms
	if (EEPROM_Adr == 144) {
		COM_EROM_ACCESS_UINT16(&EROM_IV_Point_Set_Delay_ms, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_IV_Point_Meas_Nu_AVR_Sets
	if (EEPROM_Adr == 145) {
		COM_EROM_ACCESS_UINT8(&EROM_IV_Point_Meas_Nu_AVR_Sets, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8 EROM_IV_Point_Meas_Nu_AVR_VoltCurr
	if (EEPROM_Adr == 146) {
		COM_EROM_ACCESS_UINT8(&EROM_IV_Point_Meas_Nu_AVR_VoltCurr, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// Maximum power point Tracker control variables
	//---------------------------
	// WRITE float EROM_LOAD_MPPT_Max_Step_CND
	if (EEPROM_Adr == 150) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_MPPT_Max_Step_CND, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_MPPT_Min_Step_CND
	if (EEPROM_Adr == 151) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_MPPT_Min_Step_CND, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_MPPT_Step_Increase_Fact
	if (EEPROM_Adr == 152) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_MPPT_Step_Increase_Fact, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_MPPT_Step_Reduction_Fact
	if (EEPROM_Adr == 153) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_MPPT_Step_Reduction_Fact, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_MPPT_Tolerance_CND
	if (EEPROM_Adr == 154) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_MPPT_Tolerance_CND, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_MPPT_Timer_Update_Delay
	if (EEPROM_Adr == 155) {
		COM_EROM_ACCESS_UINT16(&EROM_LOAD_MPPT_Timer_Update_Delay, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// Current Tracker control variables
	//---------------------------
	// WRITE float EROM_LOAD_CurrT_Max_Step_CND
	if (EEPROM_Adr == 160) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_CurrT_Max_Step_CND, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_CurrT_Min_Step_CND
	if (EEPROM_Adr == 161) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_CurrT_Min_Step_CND, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_CurrT_Step_Increase_Fact
	if (EEPROM_Adr == 162) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_CurrT_Step_Increase_Fact, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_CurrT_Step_Reduction_Fact
	if (EEPROM_Adr == 163) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_CurrT_Step_Reduction_Fact, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_LOAD_CurrT_Tolerance_CND
	if (EEPROM_Adr == 164) {
		COM_EROM_ACCESS_FLOAT(&EROM_LOAD_CurrT_Tolerance_CND, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// start-up mode control Variables
	//---------------------------
	// WRITE uint8_t EROM_SysControl
	if (EEPROM_Adr == 165) {
		COM_EROM_ACCESS_UINT8(&EROM_SysControl, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE uint8_t EROM_PV_Mode_CTR
	if (EEPROM_Adr == 166) {
		COM_EROM_ACCESS_UINT8(&EROM_PV_Mode_CTR, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_PV_Volt_Setpoint
	if (EEPROM_Adr == 167) {
		COM_EROM_ACCESS_FLOAT(&EROM_PV_Volt_Setpoint, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_PV_Curr_Setpoint
	if (EEPROM_Adr == 168) {
		 COM_EROM_ACCESS_FLOAT(&EROM_PV_Curr_Setpoint, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	
	//###########################
	// Fan control Variables
	//---------------------------
	// WRITE uint8_t EROM_FAN_CTR_Mode
	if (EEPROM_Adr == 175) {
		COM_EROM_ACCESS_UINT8(&EROM_FAN_CTR_Mode, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_FAN_NTC1_T_ON
	if (EEPROM_Adr == 176) {
		COM_EROM_ACCESS_FLOAT(&EROM_FAN_NTC1_T_ON, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_FAN_NTC1_T_OFF
	if (EEPROM_Adr == 177) {
		COM_EROM_ACCESS_FLOAT(&EROM_FAN_NTC1_T_OFF, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_FAN_NTC2_T_ON
	if (EEPROM_Adr == 178) {
		COM_EROM_ACCESS_FLOAT(&EROM_FAN_NTC2_T_ON, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_FAN_NTC2_T_OFF
	if (EEPROM_Adr == 179) {
		COM_EROM_ACCESS_FLOAT(&EROM_FAN_NTC2_T_OFF, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_FAN_Power_ON
	if (EEPROM_Adr == 180) {
		COM_EROM_ACCESS_FLOAT(&EROM_FAN_Power_ON, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_FAN_Power_OFF
	if (EEPROM_Adr == 181) {
		COM_EROM_ACCESS_FLOAT(&EROM_FAN_Power_OFF, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE unit_16 EROM_FAN_Switch_Timer_Max
	if (EEPROM_Adr == 182) {
		COM_EROM_ACCESS_UINT16(&EROM_FAN_Switch_Timer_Max, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}
	// WRITE float EROM_TEMP_MAX_Heat_NTC
	if (EEPROM_Adr == 183) {
		COM_EROM_ACCESS_FLOAT(&EROM_TEMP_MAX_Heat_NTC, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}	
	// WRITE uint_8 EROM_TEMP_Heat_dissipation_devices
	if (EEPROM_Adr == 184) {
		COM_EROM_ACCESS_UINT8(&EROM_TEMP_Heat_dissipation_devices, EEPROM_Write, EEPROM_Val_Char, EEPROM_Command);
		goto EEPROM_WRITE_EEROM_END;
	}			
	AddEnd_String(EEPROM_Command, "?"); // return ? as error if not found

	EEPROM_WRITE_EEROM_END:;

}

// ##########################################################################################
//-------------------------------------------------------------------------------------------
// FLOAT Support function to load / write EEROM data
void COM_EROM_ACCESS_FLOAT(float * EromVariable, uint8_t Write, char *ValueString, char *ReplyString){
	/* Optimized function to access the EEPROM via com input - read & write access - single float*/
	float TempFloat;
	
	if (Write == 1) {
		TempFloat = StringToFloat(ValueString);		// Convert value string
		eeprom_write_block(&TempFloat, EromVariable, sizeof(float));
	}
	eeprom_read_block(&TempFloat, EromVariable, sizeof(float));
	FloatToString(ValueString, TempFloat);		// Write float to string
	AddEnd_String(ReplyString, ValueString);
}
//-------------------------------------------------------------------------------------------
// UINT8 Support function to load / write EEROM data
void COM_EROM_ACCESS_UINT8(uint8_t * EromVariable, uint8_t Write, char *ValueString, char *ReplyString){
	/* Optimized function to access the EEPROM via com input - read & write access - UINT8*/
	uint8_t TempUI8;
	int32_t TempI32;
	
	if (Write == 1) {
		TempI32 = atol(ValueString); // convert string to long
		if(TempI32 > 255) TempUI8 = 255; // basic boundaries
		else if(TempI32 < 0) TempUI8 = 0;
		else TempUI8 = (uint8_t) TempI32; // convert the U8 if OK
		eeprom_write_byte (EromVariable,  TempUI8);
	}
	TempUI8 = eeprom_read_byte(EromVariable);
	itoa(TempUI8, ValueString, 10);
	AddEnd_String(ReplyString, ValueString);
}
//-------------------------------------------------------------------------------------------
// UINT16 Support function to load / write EEROM data
void COM_EROM_ACCESS_UINT16(uint16_t * EromVariable, uint8_t Write, char *ValueString, char *ReplyString){
	/* Optimized function to access the EEPROM via com input - read & write access - UINT16*/
	uint16_t TempU16;
	int32_t TempI32;
	
	if (Write == 1) {
		TempI32 = atol(ValueString); // convert string to long
		if(TempI32 > 65535) TempU16 = 65535; // basic boundaries
		else if(TempI32 < 0) TempU16 = 0;
		else TempU16 = (uint16_t) TempI32; // convert the U16 if OK
		eeprom_write_block(&TempU16, EromVariable, sizeof(uint16_t));
	}
	eeprom_read_block(&TempU16, EromVariable, sizeof(uint16_t));
	ltoa((int32_t) TempU16, ValueString, 10);
	AddEnd_String(ReplyString, ValueString);
}


//-------------------------------------------------------------------------------------------
// Write FLOAT to EEPROM
void EEPROM_WRITE_FLOAT(float value,float * EEMEMvariable){
	eeprom_write_block(&value, EEMEMvariable, sizeof(float));
}

//-------------------------------------------------------------------------------------------
// Read FLOAT from EEPROM
float EEPROM_READ_FLOAT(float * EEMEMvariable){
	float temp;
	eeprom_read_block(&temp, EEMEMvariable, sizeof(float));
	return(temp);
} 

//-------------------------------------------------------------------------------------------
// Write UINT8 to EEPROM
void EEPROM_WRITE_UINT8(uint8_t value,uint8_t * EEMEMvariable){
	eeprom_write_byte (EEMEMvariable,  value);
}

//-------------------------------------------------------------------------------------------
// Read UINT8 from EEPROM
uint8_t EEPROM_READ_UINT8(uint8_t * EEMEMvariable){
	uint8_t temp;
	temp = eeprom_read_byte(EEMEMvariable);
	return(temp);
} 

//-------------------------------------------------------------------------------------------
// Write INT16 to EEPROM
void EEPROM_WRITE_INT16(int16_t value,int16_t * EEMEMvariable){
	eeprom_write_block(&value, EEMEMvariable, sizeof(int16_t));
}

//-------------------------------------------------------------------------------------------
// Read INT16 from EEPROM
int16_t EEPROM_READ_INT16(int16_t * EEMEMvariable){
	int16_t temp;
	eeprom_read_block(&temp, EEMEMvariable, sizeof(int16_t));
	return(temp);
}

//-------------------------------------------------------------------------------------------
// Write UINT16 to EEPROM
void EEPROM_WRITE_UINT16(uint16_t value,uint16_t * EEMEMvariable){
	eeprom_write_block(&value, EEMEMvariable, sizeof(uint16_t));
}

//-------------------------------------------------------------------------------------------
// Read UINT16 from EEPROM
uint16_t EEPROM_READ_UINT16(uint16_t * EEMEMvariable){
	uint16_t temp;
	eeprom_read_block(&temp, EEMEMvariable, sizeof(uint16_t));
	return(temp);
}

//-------------------------------------------------------------------------------------------
// Write INT32 to EEPROM
void EEPROM_WRITE_INT32(int32_t value,int32_t * EEMEMvariable){
	eeprom_write_block(&value, EEMEMvariable, sizeof(int32_t));
}

//-------------------------------------------------------------------------------------------
// Read INT32 from EEPROM
int32_t EEPROM_READ_INT32(int32_t * EEMEMvariable){
	int32_t temp;
	eeprom_read_block(&temp, EEMEMvariable, sizeof(int32_t));
	return(temp);
}

//-------------------------------------------------------------------------------------------
// Write String to EEPROM
void EEPROM_WRITE_STRING(char * Value,char * EEMEMvariable,uint8_t Length){
	eeprom_write_block(Value, EEMEMvariable, Length);
}

//-------------------------------------------------------------------------------------------
// Read String from EEPROM
void EEPROM_READ_STRING(volatile char * ReturnVar, char * EEMEMvariable, uint8_t Length){
	char temp[Length];
	eeprom_read_block(temp, EEMEMvariable, Length);
	Copy_String(ReturnVar, temp);
}