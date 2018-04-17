## How to Compile

```
修改顶层Makefile的STAGING_DIR和TOOLPATH，然后执行make.
```



## Restore Uboot Env

```
如果您的uboot不是这版本，在烧录这个版本的uboot后，您需要执行如下操作：
按住RESET键上电，并保持20s以上后松开即可。
```

## Start Httpd to upgrade

```
1.将设备的LAN口通过网线接到电脑，并设置电脑IP为192.168.1.2
2.按住RESET键上电，并保持5s以上后松开即可。
3.在浏览器输入：192.168.1.1（推荐Google浏览器）
```

