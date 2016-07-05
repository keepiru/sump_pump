# sump_pump
simple Arduino-based sump pump controller

This is an Arduino sketch which senses water level and controls a pump.
The purpose is to empty the condensate out of the bottom of my air
conditioner before it spills on the floor.

Water is sensed by leaving a ground wire and a sense wire in the bottom of
the condensate pan.  When they get covered with water the pump turns on to
empty it out.  My pump (a $10 miniature peristaltic pump from Amazon) is
safe to run dry, so this program keeps it running for a few minutes to
finish slurping the last of the water out of the pan and purge the lines
with air.

I'm using an Adafruit motor shield v1.2 to power the pump.  When using 5v
power, the H-bridge drops too much voltage if I connect to both sides of
output.  Instead, I connect the pump to one side of the output and ground.
I also have to feed power directly into the 5v rail because the USB port
drops too much juice.

Alternatively you could use a separate slightly-higher-voltage supply for
the motor shield, or replace the whole shield with discrete transistor and
flyback diode, or use a v2 motor shield which has better H-bridges with
less voltage drop.
