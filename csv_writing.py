import serial
import time
print_condition_flag = False
serial_object = serial.Serial()
port_number = 5
receiving_terminated = False
while not serial_object.is_open:
    try:
        serial_object = serial.Serial(port="COM"+str(port_number), baudrate=115200, timeout=20)
    except serial.SerialException:
        if not print_condition_flag:
            print("Port serial is not opened", end="")
            print_condition_flag = True
        print(".", end="")
        time.sleep(1)
print("\nOpened!")
try:
    while True:
        serial_string = serial_object.readline().decode()
        if serial_string == "end\n":
            break   # Переносом строки может стать ""\r\n"!
        string_to_column = "" # объявляем
        for char in serial_string:
            if char == "\t" or char == "\n":
                print(string_to_column + "\t|\t", end="")
                string_to_column = "" # обнуляем
                continue
            string_to_column += char
        print(end="\n")
except serial.SerialException:
    print(f"Nothing got from COM{port_number}")
serial_object.close()
