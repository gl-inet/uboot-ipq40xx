include $(TOPDIR)/config.mk

LIB= $(obj)dhcpd.o

COBJS-y += dhcpd.o

COBJS:= $(COBJS-y)
SRCS:= $(COBJS:.o=.c)
OBJS:= $(addprefix $(obj),$(COBJS))

$(LIB):LIB$(obj).depend $(OBJS)
		$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target

include $(SRCTREE)/rules.mk

sinclude $(obj).dependi
