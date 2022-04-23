# esp-idf-pwm-slider
Slider Bar Control using ESP-IDF.   
ESP-IDF contains a lot of example code, but there is no example to create FORM on the WEB and input data from RANGE object.   
This project reads value from the RANGE object on the WEB and controls LEDs and so on.   
No library other than ESP-IDF is required to read the value from the WEB page.   

I watched [this](https://www.youtube.com/watch?v=s-NFdMXA0H4&t=167s) video.   

You can control these device from your smartphone.   

- LED Control   
![led-control](https://user-images.githubusercontent.com/6020549/135770897-e17f9b45-86ee-4c94-aa90-eb4b1ce1305d.jpg)

- Servo Control   
![servo-control](https://user-images.githubusercontent.com/6020549/135770904-915448e9-3ad1-40dd-be85-d91a06e1a0c6.jpg)

- RGB Control   
![rgb-control](https://user-images.githubusercontent.com/6020549/135774278-5a4021da-3f9e-4300-a872-4d82cfc2a53d.jpg)

- Digital potentiometer   
Components of the DS3502 is [here](https://github.com/UncleRus/esp-idf-lib/tree/master/components/ds3502).   
Components of the Three-Wire Digitally Controlled Potentiometer is [here](https://github.com/nopnop2002/esp-idf-x9c103).   

- Digital Anlog Converter   
Components of the PCF8591 is [here](https://github.com/UncleRus/esp-idf-lib/tree/master/components/pcf8591).   
Components of the MCP4725 is [here](https://github.com/UncleRus/esp-idf-lib/tree/master/components/mcp4725).   

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

# Reference
https://github.com/nopnop2002/esp-idf-web-form

