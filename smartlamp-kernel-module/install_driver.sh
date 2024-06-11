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

sudo rmmod $1.ko
sudo insmod $1.ko
lsmod | grep $1



