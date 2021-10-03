# esp-idf-pwm-slider
PWM Slider Bar Control using ESP-IDF.   
ESP-IDF contains a lot of example code, but there is no example to create FORM on the WEB and input data from FORM.   
No library other than ESP-IDF is required to read the data from the WEB page.   

I watched [this](https://www.youtube.com/watch?v=s-NFdMXA0H4&t=167s) video.   

LED PWM control example is [here](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/ledc/ledc_basic).   


![pwm-slider](https://user-images.githubusercontent.com/6020549/135744931-c5a9deb7-0347-4635-be0c-46d2c79a42f8.jpg)

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-pwm-slider
cd esp-idf-pwm-slider
idf.py set-target esp32
idf.py menuconfig
idf.py flash monitor
```


# Configuration
Set the following items using menuconfig.

![config-main](https://user-images.githubusercontent.com/6020549/135744943-ca704fba-2786-4238-88bd-5fb5b7aae8f1.jpg)
![config-app](https://user-images.githubusercontent.com/6020549/135744945-34686d90-e0c6-4520-8e66-5a4fb05f0cbe.jpg)

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
![config-pwm](https://user-images.githubusercontent.com/6020549/135745004-cfc7631c-49ad-4751-bc39-181dadf412db.jpg)

# How to use
Connect the LED according to the GPIO Setting.   
Open your brouser, and put address in address bar.   
You can use the mDNS hostname instead of the IP address.   
Default mDNS name is esp32-server.local.   
The brightness of the LED changes when the Slider is operated.   

# WEB Page
You can change the web page.   
The WEB page is stored in the html folder.   

# How to browse image data using built-in http server   
Even if there are image files in SPIFFS, the esp-idf http server does not support this:   
```
httpd_resp_sendstr_chunk(req, "<img src=\"/spiffs/picture.png\">");
```

You need to convert the image file to base64 string.   
```
httpd_resp_sendstr_chunk(req, "<img src=\"data:image/png;base64,");
httpd_resp_sendstr_chunk(req, (char *)BASE64_ENCODE_STRING);
httpd_resp_sendstr_chunk(req, "\">");
```

Images in png format are stored in the image folder.   
Images in base64 format are stored in the html folder.   
I converted using the base64 command.   
```
$ base64 image/ESP-LOGO.png > html/ESP-LOGO.txt
```
