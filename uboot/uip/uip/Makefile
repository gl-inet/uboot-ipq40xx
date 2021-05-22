include $(TOPDIR)/config.mk

LIB= $(obj)uip.o

COBJS-y +=uip.o   uip-neighbor.o    uip-split.o              psock.o           timer.o           uip_arp.o         uip-fw.o       uiplib.o

COBJS:= $(COBJS-y)
SRCS:= $(COBJS:.o=.c)
OBJS:= $(addprefix $(obj),$(COBJS))

$(LIB):LIB$(obj).depend $(OBJS)
		$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

