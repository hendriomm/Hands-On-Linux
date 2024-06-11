sudo rmmod probe.ko
sudo rmmod serial.ko
sudo rmmod serial_write.ko
sudo rmmod smartlamp.ko
sudo rmmod sysfs.ko

sudo rmmod serial
sudo rmmod sysfs

sudo rmmod usbserial

sudo rmmod cp210x
make
sudo rmmod serial.ko
sudo rmmod serial_write.ko
sudo insmod serial_write.ko
lsmod | grep serial
