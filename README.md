# R-Heating

**R-Heating** is a C++ program meant to be run on Arduino UNO. This program does control the heating system of a house using relays and temperature sensors.

## 🚊 Usage
 >  Program starts with a splash screen
 ![enter image description here](https://github.com/barbierb/Arduino-RHeating/blob/main/docs/splash.PNG?raw=true)

 
> Prints all component states, stops if one is disconnected or malfunctioning
![Blockquote](https://github.com/barbierb/Arduino-RHeating/blob/main/docs/checks.PNG?raw=true)

> Main screen, showing all temperatures and relays states
![enter image description here](https://github.com/barbierb/Arduino-RHeating/blob/main/docs/main.PNG?raw=true)

> If you long-press the middle button, you can modify the wanted temperature for the underfloor/room heating.
![enter image description here](https://github.com/barbierb/Arduino-RHeating/blob/main/docs/rtlimit.PNG?raw=true)

> If you press the middle button again, you will be able to modify the high temperature storage limit
![enter image description here](https://github.com/barbierb/Arduino-RHeating/blob/main/docs/htlimit.PNG?raw=true)

> If you press the middle button again, you will be able to modify the low temperature storage limit
![enter image description here](https://github.com/barbierb/Arduino-RHeating/blob/main/docs/ltlimit.PNG?raw=true)

> If you don't press anything while in config screens, it will come back to main one in 15 seconds

## ⚙️ Circuit

## 🖼️ Pictures & Videos

## 👨‍💻 Modifying the program
In order to modify the program you will need to follow all those following steps. If you have any issue during this procedure, please open an issue [here](https://github.com/barbierb/Arduino-RHeating/issues/)
 1. Install the [Arduino IDE](https://www.arduino.cc/en/software) on your computer
 2. Download all of this project's files by using the **CODE** button or clicking [here](https://github.com/barbierb/Arduino-RHeating/archive/refs/heads/main.zip)
 3. Unzip the downloaded content on your computer
 4. With the Arduino IDE, open the **main.ino** file
 5. Plug the Arduino UNO on the computer using a USB cable, on the editor check the right port where the Arduino is connected to
 6. Modify the program
 7. Using the **Verify** button on the editor in order to check if you made error or not
 8. Click the Upload button to compile & send the program to the board
 9. The program should now be modified 😁
