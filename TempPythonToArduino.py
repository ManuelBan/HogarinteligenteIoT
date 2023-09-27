from bs4 import BeautifulSoup
import requests
from re import split

import serial
from struct import *
import sys
import time
import random
import ast


try:
    ser=serial.Serial(baudrate='115200', timeout=.5, port='com5')
except:
    print('Port open error')

a=0

website = 'https://www.tutiempo.net/lagos-de-moreno.html?datos=por-horas'
result = requests.get(website) #pide html de la pagina
content = result.text

soup = BeautifulSoup(content, 'lxml')

box = soup.find('div', class_='thh')

textfile = open("a_file.txt", "w") #escribe datos en el .txt

allinfo = box.find_all('td')

datos = list()

for i in allinfo:
    datos.append(i.text)      #acomoda los datos en una list()
print(datos)

newdatos1 = [s.replace(" km/h", "") for s in datos] #limpia impurezas de texto
newdatos2 = [s.replace("°", "") for s in newdatos1]
newdatos3 = [s.replace("%", "") for s in newdatos2]
newdatos4 = [s.replace(" mm", "") for s in newdatos3]
newdatos5 = newdatos4 #saves values of newdatos4 at this moment

indexes_to_be_removed = [0,1, 6, 7, 8, 14, 15, 21, 22, 28, 29, 35, 36, 42, 43, 49, 50, 56, 57, 63, 64] #quita palabras de arreglo

for idx in sorted(indexes_to_be_removed, reverse = True):
    del newdatos4[idx]

enteros = [float(s) for s in newdatos4] #los convierte a float
enteros = [round(s) for s in enteros] #redondea los datos
enteros = [int(s) for s in enteros] #los convierte a enteros
print(enteros)

####################################################

time.sleep(5)#no delete!
while True:
    try:
        ser.write(pack ('15h',enteros[0],enteros[1],enteros[2],enteros[3],enteros[4],enteros[5],enteros[6],enteros[7],enteros[8],
                        enteros[9],enteros[10],enteros[11],enteros[12],enteros[13],random.randint(0,100)))#the 15h is 15 element, and h is an int type data
                                                                    #random test, that whether data is updated
        time.sleep(1)#delay 2s
        dat=ser.readline()#read a line data
        
        if dat!=b''and dat!=b'\r\n':
            try:                #convert in list type the readed data
                dats=str(dat)
                dat1=dats.replace("b","")
                dat2=dat1.replace("'",'')
                dat3=dat2[:-4]
                list_=ast.literal_eval(dat3) #list_ value can you use in program
                print(dat3)
            except:
                print('Error in corvert, readed: ', dats)
        time.sleep(.05)
    except KeyboardInterrupt:
        break
    except:
        print(str(sys.exc_info())) #print error
        break



