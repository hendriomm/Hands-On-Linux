#sudo rmmod probe.ko 2> /dev/null
#sudo rmmod serial.ko 2> /dev/null
#sudo rmmod serial_write.ko 2> /dev/null
#sudo rmmod smartlamp.ko 2> /dev/null
#sudo rmmod sysfs.ko 2> /dev/null

sudo rmmod serial 2> /dev/null
sudo rmmod sysfs 2> /dev/null

sudo rmmod usbserial 2> /dev/null

sudo rmmod cp210x 2> /dev/null
make all
sudo insmod $1.ko
lsmod | grep $1
