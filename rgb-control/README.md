# RGB Control using Slider Bar

LEDC Basic Example is [here](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/ledc/ledc_basic).   

![rgb-control](https://user-images.githubusercontent.com/6020549/135774278-5a4021da-3f9e-4300-a872-4d82cfc2a53d.jpg)


# Installation
```
git clone https://github.com/nopnop2002/esp-idf-pwm-slider
cd esp-idf-pwm-slider/rgb-control
idf.py set-target {esp32/esp32s2/esp32s3/esp32c2/esp32c3/esp32c6}
idf.py menuconfig
idf.py flash monitor
```


# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/135744943-ca704fba-2786-4238-88bd-5fb5b7aae8f1.jpg)
![config-app](https://user-images.githubusercontent.com/6020549/135774085-5e06b6a9-aaad-43e3-908e-17d2da166425.jpg)

## Wifi Setting

![config-wifi-1](https://user-images.githubusercontent.com/6020549/135744955-36149a83-d887-4271-8cae-b90cf188dda6.jpg)

You can connect using the mDNS hostname instead of the IP address.   

![config-wifi-2](https://user-images.githubusercontent.com/6020549/135744972-d83fdc41-c472-46be-8a55-dd04d88e47e6.jpg)

You can use static IP.   
![config-wifi-3](https://user-images.githubusercontent.com/6020549/135744976-4a1c626d-3e93-498f-9062-a91914676567.jpg)

## HTTP Server Setting
![config-http](https://user-images.githubusercontent.com/6020549/135744994-eb863ae8-32f8-4082-a73e-49c1516ce16f.jpg)

## GPIO Setting
![config-gpio](https://user-images.githubusercontent.com/6020549/135774095-9719625e-2c7e-4f73-b4ba-3d61cdb37e1e.jpg)

# How to use
Connect the __Full-color LED__ according to the GPIO Setting.   
Full-color LEDs have a cathode common and an anode common.   
Open your brouser, and put address in address bar.   
You can use the mDNS hostname instead of the IP address.   
Default mDNS name is esp32-server.local.   
The brightness of the LED changes when the Slider is operated.   

