import RPi.GPIO as GPIO
import time
from subprocess import call

GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN, pull_up_down=GPIO.PUD_UP)

counter = 0

while True:
    input_state = GPIO.input(18)
    if input_state == False:
        counter += 1
        print('Button Pressed')
        time.sleep(0.25)
    elif counter == 1:
        print("kill sox");
        call(["killall", "sox"])
        time.sleep(0.25)
        counter = 0
    else:
        counter = 0
        time.sleep(0.25)

    if counter == 8:
        print('shutdown')
        call(["killall", "mpradio"])
        call(["killall", "sox"])
        call(["shutdown", "-h", "now"])
        time.sleep(1)

