# DS18B20_Kernel_module
A kernel module and user-space interface for the DS18B20 temperature sensor, for a university project.

## Changing device name
To locate the correct temperature sensor device:
```bash
ls /sys/bus/w1/devices/

28-000010ab45bb  w1_bus_master1
# 28-XXXXXXXXXXXX is the name of the deive, change in user-space
```
## Compile and load into kernel
In the repo directory:
```bash
make
```
Load module:
```bash
sudo insmod tempmon.ko
```
Ensure the module is loaded:
```bash
dmesg | tail
```
## Compile user-space application
Make sure the pigpio library is installed. Then compile user the lpigpio linker:
```bash
gcc user_space.c -o temp_app -lpigpio -lrt -pthread
```
Run the application:
```bash
sudo ./temp_app <lower_temp> <upper_temp>
```
## To remove the kernel module
```bash
sudo rmmod tempmon
```


