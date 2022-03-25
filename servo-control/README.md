# Servo Control using Slider Bar

MCPWM RC Servo Control Example is [here](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_servo_control).   

![servo-control](https://user-images.githubusercontent.com/6020549/135770904-915448e9-3ad1-40dd-be85-d91a06e1a0c6.jpg)

# Software requiment
- esp-idf ver4.1 or later.   
'tcpip_adapter_init' has been deprecated.


# Installation
```
git clone https://github.com/nopnop2002/esp-idf-pwm-slider
cd esp-idf-pwm-slider/servo-control
idf.py set-target esp32
idf.py menuconfig
idf.py flash monitor
```


# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/135744943-ca704fba-2786-4238-88bd-5fb5b7aae8f1.jpg)
![config-app](https://user-images.githubusercontent.com/6020549/135771137-57c04e8d-9e26-4550-b2df-19beafe5eb93.jpg)

## Wifi Setting

![config-wifi-1](https://user-images.githubusercontent.com/6020549/135744955-36149a83-d887-4271-8cae-b90cf188dda6.jpg)

You can use the mDNS hostname instead of the IP address.   
- esp-idf V4.3 or earlier   
 You will need to manually change the mDNS strict mode according to [this](https://github.com/espressif/esp-idf/issues/6190) instruction.   
- esp-idf V4.4 or later  
 If you set CONFIG_MDNS_STRICT_MODE = y in sdkconfig.default, the firmware will be built with MDNS_STRICT_MODE = 1.

![config-wifi-2](https://user-images.githubusercontent.com/6020549/135744972-d83fdc41-c472-46be-8a55-dd04d88e47e6.jpg)

You can use static IP.   
![config-wifi-3](https://user-images.githubusercontent.com/6020549/135744976-4a1c626d-3e93-498f-9062-a91914676567.jpg)

## HTTP Server Setting
![config-http](https://user-images.githubusercontent.com/6020549/135744994-eb863ae8-32f8-4082-a73e-49c1516ce16f.jpg)

## SERVO Setting
![config-servo](https://user-images.githubusercontent.com/6020549/135771153-b26c7102-2561-435e-a8e9-86680edbae83.jpg)

# How to use
Connect the Servo motor according to the SERVO Setting.   
I used SG90 Micro Servo Motor.   
Open your brouser, and put address in address bar.   
You can use the mDNS hostname instead of the IP address.   
Default mDNS name is esp32-server.local.   
When you operate the slider, the servo motor will rotate.   

# Wireing  

|SERVO||ESP32|
|:-:|:-:|:-:|
|PWM|--|GPIO18(*1)|
|GND|--|GND|
|VCC|--|3.3V(*2)|

(*1) You can change using menuconfig.   

(*2) Depending on the servo motor, 5V may be required.   
