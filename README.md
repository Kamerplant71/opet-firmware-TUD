# OPET Firmware

OPET (Open PV Electrical Tool) is a programmable electronic load for long-term performance measurement of photovoltaic (PV) devices in the field or under controlled environmental conditions. This repository contains the firmware source code and prebuilt release binaries for the OPET microcontroller.

For related repositories, see:
- [OPET Hardware](https://github.com/NREL/opet-hardware) — PCB design and schematics
- [OPET Control Software](https://github.com/NREL/opet-control) — host-side control application
- [OPET Firmware](https://github.com/NatLabRockies/opet-firmware) - Original OPET firmware

This is an adaptation of the original repository of the OPET Firmware. Check thier documentation for basic functionality if needed.

# Adaptations

For the TUD monitoring station, the following modificatins to the firmware were made.

- Included functionality for the MAX31856 T-type Thermocoupler sensor
    - added functions `void TEMP_MAX31856_Setup()` and `float TEMP_MAX31856_Measure()` for setting up and measuring the sensor in `MPPT_PCB_MCU__IO.c`. Sensor can be configured in the `main.h` by setting PCBconfig_TEMP_Sensor_Type to `Temp_Sensor__MAX31856`.
- Added Temperature variable to the IV curve data response
- Added MPPT_PCB_MCU__TEMP.c/h for monitoring the temperature for up to 8 ADC's (ADC121C021) which monitor 8 NTCs. 
- Added new communication command `TEMP?` to monitor the temperature of the onboard NTC's and if configured the PV temperature and the temperature of each heat monitoring device. The reply will be in the form of `TEMP?\t<NTC1>\t<NTC2>\t<RTD>\t<T0>\t<T1>\t<T2>\t<T3>\t<T4>\t<T5>\t<T6>\t<T7>\n`
- If heat monitoring device is installed, it gives an error and shuts down the power in case of over temperature or if not enough devices give temperature readings. Errors can be read from the statusbyte. 
    - Bit 4 of `SysStatus_B` is set in case of overtemperature
    - Bit 5 of `SysStatus_B` is set in there are less readings then expected.
- Two new eerom addresses for the temperature monitoring:  at `183`, the max temperature stored for the device and at `184` the number of monitoring devices can be configured.


