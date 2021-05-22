include $(TOPDIR)/config.mk

CFLAGS +=-I$(TOPDIR)/uip/uip/

LIB= $(obj)dhcpc.o

COBJS-y += dhcpc.o

COBJS:= $(COBJS-y)
SRCS:= $(COBJS:.o=.c)
OBJS:= $(addprefix $(obj),$(COBJS))

$(LIB):LIB$(obj).depend $(OBJS)
		$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
.depend: Makefile $(OBJS:.o=.c)
			$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

