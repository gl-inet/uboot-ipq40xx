## compile

clone uboot code
```
git clone https://github.com/gl-inet/uboot-ipq40xx.git
```
clone sdk, ie. toolchain to compile uboot:
```
git clone https://github.com/gl-inet-builder/openwrt-sdk-ipq806x-qsdk53
```
edit toplevel Makefile:
change STAGING_DIR varable to point to openwrt-sdk-ipq806x/staging_dir you just clone.
```
make
```
the uboot binary will be:
bin/openwrt-ipq40xx-u-boot-stripped.elf


## using uboot web:

1. connect lan and PC by ethernet, your PC's ip should be 192.168.1.x
2. push reset button and power on, about 10 seconds later release the button, until wifi led stop
  flashing and the middle led light on.
3. using browser:
  access http://192.168.1.1 to flash firmware.
  access http://192.168.1.1/uboot.html to flash uboot.

## tftp upgrade by uboot cmd line:

connect by serial port, use keyboard input 
```
gl
```
to get into uboot cmd line.

setup tftpd server at ip 192.168.1.2, put **firmware.bin**(change name according to board, will be prompted by issue the following command) at tftpd root directory.
flash firmware(factory firmware or openwrt firmware) by:
```
run lf
```

setup tftpd server at ip 192.168.1.2, put **openwrt-ipq40xx-u-boot-stripped.elf**(change name according to board, will be prompted by issue the following command) at tftpd root directory.
flash uboot by:
```
run lu
```

## automatic tftp upgrade:
This function is disabled by default, turn it on by
```
setenv tftp_upgrade 1
saveenv

```
1. setup tftpd server at ip 192.168.1.2, put **firmware.bin**(change name according to board, will be prompted by issue 'run lf' command) at tftpd root directory.
2. connect the device and tfppd server, when the device power on, it will detect tftpd 
server to do flashing automatically.


