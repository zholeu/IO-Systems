sudo insmod calc.ko
sudo chmod 777 /dev/var2
sudo chmod 777 /proc/var2
sudo echo "5+6" > /dev/var2
sudo echo "11-2" > /dev/var2
sudo echo "10*2" > /dev/var2
sudo echo "100/5" > /dev/var2
sudo echo "8/0" > /dev/var2
sudo cat /proc/var2
sudo cat /dev/var2
sudo dmesg | tail -10

