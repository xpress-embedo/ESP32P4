import struct

# Configuration values
access_point_name = "ESP32_AP"
ip_address = "192.168.0.1"
serial_number = "SN0123456789"

# Fixed sizes for each field (in bytes)
AP_NAME_SIZE = 32
IP_ADDR_SIZE = 16
SERIAL_SIZE = 32

# Pad or truncate each field
def pad(value, size):
    return value.encode('utf-8')[:size].ljust(size, b'\x00')

# Create binary blob
binary_data = b""
binary_data += pad(access_point_name, AP_NAME_SIZE)
binary_data += pad(ip_address, IP_ADDR_SIZE)
binary_data += pad(serial_number, SERIAL_SIZE)

# Save to file
with open("config.bin", "wb") as f:
    f.write(binary_data)

print("Binary config written to config.bin")
