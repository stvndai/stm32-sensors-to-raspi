import smbus2
import time

I2C_BUS = 1
STM32_ADDR = 0x28

bus = smbus2.SMBus(I2C_BUS)

def read_dht_data():
    try:
        data = bus.read_i2c_block_data(STM32_ADDR, 0x00, 5)
        humidity_int, humidity_dec, temp_int, temp_dec, checksum = data

        expected_checksum = (humidity_int + humidity_dec + temp_int + temp_dec) & 0xFF
        valid = (checksum == expected_checksum)

        return {
            "humidity": humidity_int + humidity_dec / 10,
            "temperature": temp_int + temp_dec / 10,
            "checksum_valid": valid,
        }
    except OSError as e:
        print(f"I2C read error: {e}")
        return None

if __name__ == "__main__":
    while True:
        reading = read_dht_data()
        if reading:
            status = "OK" if reading["checksum_valid"] else "CHECKSUM FAIL"
            print(f"Temp: {reading['temperature']:.1f}C  Humidity: {reading['humidity']:.1f}%  [{status}]")
        time.sleep(1)