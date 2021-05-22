


include $(TOPDIR)/config.mk

LIB= $(obj)
CFLAGE += -I$(TOPPDIR)/uip/uip/

SOBJS=clock-arch.o

COBJS-y +=clock-arch.o 

COBJS= $(COBJS-y)
SRCS 	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS:= $(addprefix $(obj),$(SOBJS) $(COBJS))

.depend: Makefile $(OBJS:.o=.c)
		     $(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@
$(LIB):LIB$(obj).depend $(OBJS)
		$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target

include $(SRCTREE)/rules.mk

sinclude $(obj).depend
