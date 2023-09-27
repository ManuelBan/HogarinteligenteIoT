
Importante antes de correr el código, Majo y Manu del futuro
El código de python lo dejaré comentado hasta abajo del programa, en caso de que perdamos el archivo .py, solo será copiar y pegar en algun IDE
El programa no puede imprimir cosas en el serial print del arduino porque no deja correr en paralelo el serial de python y el del arduino.
La pantalla LCD es MUY importante, pues solo así podrán ver las variables que hay en arduino
El programa solo puede ser corrido en arduino MEGA, (Arduino UNO no soporta el programa)
Cargar el programa en el arduino antes de ejecutar el código de python
La conexion utilizada para el LCD es la de esta pagina: https://naylampmechatronics.com/blog/34_tutorial-lcd-conectando-tu-arduino-a-un-lcd1602-y-lcd2004.html 
el programa está configurado con esas conexiones.
El primer dato que muestra el LCD es la temperatura del DTH, el segundo la temperatura de internet
El tercer dato es la humedad del DHT, y el cuertop la humedad de la pagina de internet
Los dos ultimos datos es el PWM y angulo de apertura de la ventana
