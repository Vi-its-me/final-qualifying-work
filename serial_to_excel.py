import serial
import time
from openpyxl import Workbook

got_begin = False
workbook = Workbook()
worksheet = workbook.active
print_condition_flag = False
serial_object = serial.Serial()
port_number = 5
receiving_terminated = False
begin_found_flag = False
# исключение для вызова в случае, если не получили
class Did_not_begin(Exception):     # "стартовую" строку
    def __init__(self, text):
        self.txt = text
# функция для печати в таблицу, начиная с первой её строки
def print_to_spreadsheet(row, list):
    column = 1
    for element in list:
        worksheet.cell(row=row, column=column, value=element)
        column += 1
def find_empty_row(worksheet, column_letter = 'A'):
    row_number = 1
    while row_number != 30:
        if worksheet[column_letter + str(row_number)].value is None:
            return row_number
        row_number += 1
def find_average(column_number):
    count = 0
    total = 0
    for row_number in range(2, 11):
        if worksheet.cell(row=row_number, column=column_number).value is None:
            continue
        elif worksheet.cell(row=row_number, column=column_number).value == "404":
            continue
        total += int(worksheet.cell(row=row_number, column=column_number).value)
        count += 1
    return total / count
# Попытка открыть порт, пока тот не будет открыт
while not serial_object.is_open:
    try:
        serial_object = serial.Serial(port="COM"+str(port_number), baudrate=115200, timeout=20)
    except serial.SerialException:
        if not print_condition_flag:
            print("Port serial is not opened", end="")
            print_condition_flag = True
        if print_condition_flag:
            print(".", end="")
        time.sleep(1)
print("Port opened")
# заполнить первую строку называниями столбцов
print_list = ["№ Замера", "Уровень сигнала", "Время прохождения сигнала", "Расстояние от уровня сигнала", "Расстояние от времени прохождения сигнала", "Усредненное значение расстояния", "Фактическое расстояние"]
print_to_spreadsheet(1, print_list)
# Заполнить таблицу данными, полученными от NodeMCU
worksheet_row = 2
try:
    while True:
        serial_string = serial_object.readline().decode()
        if serial_string == "end\n":
            if not begin_found_flag:
                raise Did_not_begin("begin not found")
            break  # Переносом строки может стать "\r\n"!
        elif begin_found_flag:
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
        elif serial_string.find("begin\n") != -1:
            begin_found_flag = True
    # Подсчитать среднее значение в столбцах и напечатать
    print_to_spreadsheet(find_empty_row(worksheet), ["Среднее арифметическое:", find_average(2), find_average(3)])
except Did_not_begin as how:
    print(how)
except serial.SerialException:
    print(f"Nothing got from COM{port_number}")
else:
    workbook.save(input("Enter the distance in meters: ") + "m" + ".xlsx")
finally:
    serial_object.close()
