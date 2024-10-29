The bluetooth mode on the RPI4 is a Broadcom Chip (BCM43455). It holds the controller layer for the bluetooth stack.
Firmware needs to be loaded onto the chip after the main BCM2711 is powered up. This software is obtained from
the BCM43450.hcd file from the Bluez Repo. https://github.com/RPi-Distro/bluez-firmware/tree/master/broadcom