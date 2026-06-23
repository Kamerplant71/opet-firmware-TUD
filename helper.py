from OPET_control import OPETBus, OPET
from serial import Serial

# ---- Edit these to match your setup ----
opet_port_name = 'COM7'  # serial port connected to the OPET bus
opet_address = 1          # jumper-configured address (0-31)
# -----------------------------------------

opet_port = Serial(
    opet_port_name,
    baudrate=200000,
    timeout=4
)

opet_bus = OPETBus(opet_port)
opet = OPET(opet_bus, opet_address)

print("Identification:") 
print(opet.identification)

# SysConfig
opet.send_verify("EEROM:WRITE\t2\t5\n") 
print(opet.send_verify("EEROM:READ?\t2\n")) 

# Sensor type
opet.send_verify("EEROM:WRITE\t3\t3\n") # Set to MAX31856  
print(opet.send_verify("EEROM:READ?\t3\n")) 

# Max temperature
opet.send_verify("EEROM:WRITE\t183\t70\n")  # in degrees C 
print(opet.send_verify("EEROM:READ?\t183\n")) 

# Number of devices
opet.send_verify("EEROM:WRITE\t184\t2\n")  
print(opet.send_verify("EEROM:READ?\t184\n")) 


# Reset OPET
opet.send_verify("*RST\n") 
