Download the esp idf 3.3 or greater

1) using the esp-idf powershell type the following (without the #>)
#> idf.py create some-project-name

2) open the newly created directory, copy and paste my source code from main/esp32_custom_pulse.c into your main/some-project-name.c and then save
3) type the following to set the tartet to esp32c3 esp32s3 or whatever...  If you type "idf.py set-target" without an actual target, then it lists all the options. 
#> idf.py set-target esp32
#> idf.py build
4) Figure out what COM port you're using (Arduino IDE is a handy tool for that)
5) type the following replacing the relevant COM port:
#> idf.py flash -p COM12
and if that works you should have pulses, you can also open the monitor to see if there are any errors.
#> idf.py moniitor -p COM12 
   


