import Adafruit_BBIO.GPIO as GPIO
import Adafruit_BBIO.PWM as PWM

#GPIO.setup("P8_10", GPIO.OUT)
#GPIO.output("P8_10", GPIO.HIGH)
#GPIO.cleanup()

PWM.start("P9_14", 50)

