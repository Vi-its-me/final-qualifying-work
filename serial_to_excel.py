import serial
import time
from openpyxl import Workbook
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
workbook = Workbook()
worksheet = workbook.active
worksheet_row = 1
try:
    while True:
        serial_string = serial_object.readline().decode()
        if serial_string == "end\n":
            break  # Переносом строки может стать ""\r\n"!
        worksheet_column = 1
        previous_iterator = 0
        iterator = 0
        if iterator:
            previous_iterator = iterator
        for char in serial_string:
            if char == "\t" or char == "\n":
                worksheet.cell(row=worksheet_row, column=worksheet_column).value = serial_string[previous_iterator:iterator]
                worksheet_column += 1
                previous_iterator = iterator + 1
            iterator += 1
        worksheet_row += 1
        # перейти на другую строку
except serial.SerialException:
    print(f"Nothing got from COM{port_number}")
workbook.save(input("Enter the distance in meters: ") + "m" + ".xlsx")
serial_object.close()
