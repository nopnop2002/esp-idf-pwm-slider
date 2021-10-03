# LED Control using Slider Bar

LEDC Basic Example is [here](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/ledc/ledc_basic).   

![led-control](https://user-images.githubusercontent.com/6020549/135770897-e17f9b45-86ee-4c94-aa90-eb4b1ce1305d.jpg)

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-pwm-slider
cd esp-idf-pwm-slider/led-control
idf.py set-target esp32
idf.py menuconfig
idf.py flash monitor
```


# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/135744943-ca704fba-2786-4238-88bd-5fb5b7aae8f1.jpg)
![config-ap](https://user-images.githubusercontent.com/6020549/135746374-6b88c724-c210-4bfd-9c23-efe7494f97e5.jpg)

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

## GPIO Setting
![config-gpio](https://user-images.githubusercontent.com/6020549/135746385-462b7133-5376-43f5-b207-c8ad77881c7a.jpg)

# How to use
Connect the LED according to the GPIO Setting.   
Open your brouser, and put address in address bar.   
You can use the mDNS hostname instead of the IP address.   
Default mDNS name is esp32-server.local.   
The brightness of the LED changes when the Slider is operated.   

