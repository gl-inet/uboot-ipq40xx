## 编译

首先clone uboot代码uboot-ipq40xx：
```
git clone https://github.com/gl-inet/uboot-ipq40xx.git
```
然后通过clone sdk来获得toolchain：
```
git clone https://github.com/gl-inet/openwrt-sdk-ipq806x.git
```
修改顶层Makefile的STAGING_DIR，指向openwrt-sdk-ipq806x/staging_dir，然后执行：
```
make
```
生成的uboot为：
bin/openwrt-ipq40xx-u-boot-stripped.elf


## web升级

1. 将设备的LAN口通过网线接到电脑，并设置电脑IP为192.168.1.x
2. 按住RESET键上电，并保持5s以上后松开即可。可通过观察wifi灯闪烁至熄灭中间灯亮来指示。
3. 在浏览器（推荐Google浏览器）输入：
访问http://192.168.1.1来升级固件
访问http://192.168.1.1/uboot.html来升级uboot


## tftp命令行升级

在tftpd服务器（ip为192.168.1.2）中放入用于升级的固件或uboot。
升级固件（出厂固件或openwrt固件），固件名字默认为firmware.bin
```
run lf
```

升级uboot，uboot名字默认为openwrt-ipq40xx-u-boot-stripped.elf
```
run lu
```

## 自动升级
在tftpd服务器（ip为192.168.1.2）中放入用于升级的固件，固件名字默认为firmware.bin

