#Copyright (c) 2015, mleaf mleaf90@gmail.com or 350983773@qq.com
#All rights reserved.
# include ../config.mk
include $(TOPDIR)/config.mk
include $(OBJTREE)/include/autoconf.mk

CFLAGS +=-Iunix/ -Iuip/   -Iapps/dhcpc -Iapps/dhcpd

LIB	=libuip.o

OS_OBJS  = unix/main.o unix/clock-arch.o

DHCPC_OBJS = apps/dhcpc/dhcpc.o
DHCPD_OBJS = apps/dhcpd/dhcpd.o
UIP_OBJS =uip/uip.o  uip/uip_arp.o uip/uiplib.o   uip/timer.o \
	   uip/uip-neighbor.o


#LIB	=$(obj)libuip.o
#COBJS-y += unix/main.o unix/clock-arch.o
#COBJS-y += apps/dhcpc/dhcpc.o
#COBJS-y += apps/dhcpd/dhcpd.o
#COBJS-y += uip/uip.o uip/uip_arp.o uip/uiplib.o   uip/timer.o   uip/uip-neighbor.o

#COBJS:= $(COBJS-y)
#SRCS:= $(COBJS:.o=.c)
#OBJS:= $(addprefix $(obj),$(COBJS))

OBJS = $(UIP_OBJS) $(DHCPD_OBJS) $(OS_OBJS)

$(LIB): $(START) $(OBJS)
		    $(AR) crv $@ $(OBJS)
.depend: Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

#$(LIB):$(obj).depend $(OBJS)
#	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
#include $(SRCTREE)/rules.mk
sinclude $(obj).depend

#########################################################################


