step sequencer project

prepping arduino ide...
- open arduino that teensyduino is installed to (arduino 1.0.5)
- click TOOLS drop down menu. go to BOARD and select TEENSY 2.0
- click TOOLS again. go to USB TYPE and select "Serial + Keyboard + Mouse + Joystick"
- click Tools again. CPU should be set to 16 mhz if isn't already
- click tools again. programmer should be set to AVR as ISP
- click file and go to preference. look to "Show Verbose output during" and checkk "upload"

when ready to upload...
- click the check mark. teensyloader should pop up in the backgruond. if it doesn't... bad
- click upload
- console will probably print looking for connection up to 49.
- will say done uploading, but this is a lie.
- look to teensyloader and wait for that to program. bar should appear and then fill up
- also look to the step sequencer it should reset itself. flash and then leds rush up to meet the position of pot
- if nothing happens then you'll have to hit the reset button on the teensy itself.
- teensy loader indicates this button. reach into case with finger and press it. then code should upload.
- kind of a pain, but might work?
