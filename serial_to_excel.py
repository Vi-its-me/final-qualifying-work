import serial
import time
from openpyxl import Workbook
from openpyxl.utils import get_column_letter

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
    for index, element in enumerate(list, 1):
        if isinstance(element, str):
            worksheet.column_dimensions[get_column_letter(index)].width = len(element) + 1
        worksheet.cell(row=row, column=index, value=element)
def find_empty_row(worksheet, column_letter = 'A'):
    row_number = 1
    while row_number != 30:
        if worksheet[column_letter + str(row_number)].value is None:
            return row_number
        row_number += 1
def find_average(column_number):
    count = 0
    total = 0
    for row_number in range(2, 12):
        if worksheet.cell(row=row_number, column=column_number).value is None:
            continue
        elif worksheet.cell(row=row_number, column=column_number).value == "404":
            continue
        total += int(worksheet.cell(row=row_number, column=column_number).value)
        count += 1
    return total / count
def determine_and_insert_distance(signal_amount_column, distance_column):
    for current_row in range(2, 12):
        if int(worksheet.cell(row=current_row, column=signal_amount_column).value) >= -50:
            distance = (abs(int(worksheet.cell(row=current_row, column=signal_amount_column).value)) + (-10.09)) / 43.893
            worksheet.cell(row=current_row, column=distance_column).value = distance
        elif int(worksheet.cell(row=current_row, column=signal_amount_column).value) < -50:
            distance = abs((abs(int(worksheet.cell(row=current_row, column=signal_amount_column).value)) + (-54.698)) / 0.457)
            worksheet.cell(row=current_row, column=distance_column).value = distance
def merge_cells_and_insert(column_number, to_insert):
    row_number = 2
    worksheet.merge_cells(start_row=row_number, end_row=row_number + 9, start_column=column_number, end_column=column_number)
    worksheet.cell(row=row_number, column=column_number).value = to_insert
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
print_list = ["№ Замера", "Уровень сигнала", "Время прохождения сигнала", "Расстояние от уровня сигнала", "Усредненное значение расстояния", "Фактическое расстояние"]
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
    # Подсчитать среднее значение в каждой строке и напечатать
    determine_and_insert_distance(2, 4)
    # Объединить все строки в заданном столбце и подсчитать моду всех значений
    merge_cells_and_insert(5, "=MODE(ROUND(D2:D11, 2))")
except Did_not_begin as how:
    print(how)
except serial.SerialException:
    print(f"Nothing got from COM{port_number}")
else:
    actual_distance = input("Enter actual distance in meters: ")
    # В последнем столбце объединяем все строки и помещаем значение фактического расстояния
    merge_cells_and_insert(6, actual_distance)
    workbook.save(actual_distance + "m" + ".xlsx")
finally:
    serial_object.close()
