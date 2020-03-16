import requests
import datetime
import csv
import time

url='http://192.168.43.173'

def GetHum():
    return requests.get(url + '/humidity')

def GetTemp():
    return requests.get(url + '/temperature')

def pushToCsv(tem, hum, current_time, now):
    with open('array.csv', 'a') as bigdata:
        writer= csv.writer(bigdata, delimiter=';')
        writer.writerow([tem, hum, current_time, now])

while True:
    try:
        now = datetime.datetime.now()
        current_time = now.strftime("%H:%M:%S")
        tem=GetTemp().text
        hum=GetHum().text

        print("Temperature: " + tem)
        print("Rel. Humidity: " + hum)
        print("Current Time =", current_time)
        print(now)
        pushToCsv(tem, hum, current_time, now)
        time.sleep(5)
    except:
        print("GET failed, sleeping")
        time.sleep(5)
