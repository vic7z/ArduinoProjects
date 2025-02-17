# ArduinoProjects

## Arduino RC boat
Easy to make cheap RC boat and a 6 channel RC transmitter with Small OLED that can display realtime telemetry Data such as boats power , current draw, motor Driver temperature and batter voltage


![image](https://github.com/user-attachments/assets/2c324700-b937-4a87-bab6-96020da7f90f)
![image](https://github.com/user-attachments/assets/727c7a76-f8b4-46b2-9188-518be0cb5f2e)
![image](https://github.com/user-attachments/assets/1cd235c6-99f4-4f70-9f02-b489cb316cc2)



1. TX set up

   Code for the transmitter can be found under [/RC Boat/Transmitter](https://github.com/vic7z/ArduinoProjects/tree/master/RC%20Boat/Transmitter)

   Parts
   
   - Arduino pro mini ( or Arduino Nano i like the small form factor of the pro mini)
   - 0.96" OLED SSD1306 ( I prefer the i2c version - less wiring)
   -  2 x cheap Joystick with pushbutton
  
    Charging circuit
   - 1500mAh small lipo
   - tp4056 lipo charger
  
Note : if using Arduino pro mini make sure to buy a USB to URAT converter like FT232RL/CP2102
   

   ### Wiring
   So bad at creating wirining diagram so this :(
   
   | Arduino Pro Mini  | Nrf24L01  | SSD1306 | Right JoyStick | left Joystick
   | ------------- | ------------- | ------------- | ------------- |------------- |
   |11   | MOSI  | | | |
   |12   | MISO  | | | |
   |13   | SCK  | | | |
   |7   | CE  | | | |
   |8   | CSN  | | | |
   |A4   |   | SDA | | |
   |A5  |   | SCL | | |
   |A0  |   |  |x-axis | |
   |A1  |   |  |y-axis | |
   |9  |   |  |button | |
   |A2  |   | | | x-axis|
   |A3  |   | | |  y-axis|
   |3  |   |  | | button|
