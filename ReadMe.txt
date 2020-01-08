v1:
放入内核 drivers/char
修改 drivers/char/Makefile，添加：
obj-y += leds_msm8909.o

为了在开机后安卓第三方应用直接访问/dev/led1文件需要更改以下几个文件：
a、原来的/work/SC20_R01/device/qcom/sepolicy/common/untrusted_app.te
新的区别为，为了自己编写的应用程序对LED1设备驱动文件有读写权限
末尾添加：
#add by andy:allow untrusted_app access to device
allow untrusted_app led_device:chr_file rw_file_perms;

b、原来的/work/SC20_R01/device/qcom/sepolicy/common/device.te
新的区别为，为了自己编写的应用程序对LED1设备驱动文件有读写权限
末尾添加：
#Definde led1 device add by andy
type led_device, dev_type;

b、原来的/work/SC20_R01/device/qcom/sepolicy/common/file_contexts
新的区别为，为了自己编写的应用程序对LED1设备驱动文件有读写权限
在dev node下面添加两行：
#add by andy led1 node
/dev/led1                                       u:object_r:led_device:s0

重新编译内核 make bootimage -j4