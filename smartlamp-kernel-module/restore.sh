sudo modprobe cp210x
sudo modprobe usbserial

sudo rmmod probe.ko
sudo rmmod serial.ko
sudo rmmod serial_write.ko
sudo rmmod smartlamp.ko
sudo rmmod sysfs.ko
