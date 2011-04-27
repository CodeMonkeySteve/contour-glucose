CC = gcc
LD = ld
CFLAGS = -Wall -O2 -g

GLUCOSE_OBJS = main.o

ALL_OBJS = $(GLUCOSE_OBJS)
ALL_DEBS = $(shell echo " "$(ALL_OBJS) | sed -e "s,[^ ]*\.a,,g" -e	\
	"s,\([^ ]*\)/\([^ /]*\)\.o,\1/.\2.o.d,g" -e "s, \([^		\
	/]*\)\.o, .\1.o.d,g" )

ifeq ($(V),1)
	Q		=
	QUIET_CC	=
	QUIET_LINK	=
else
	Q		= @
	QUIET_CC	= @echo "       CC " $@;
	QUIET_LINK	= @echo "     LINK " $@;
endif

all: glucose

glucose: $(GLUCOSE_OBJS)
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -vf glucose *~ *.o .*.d

.c.o:
	$(QUIET_CC)$(CC) -MMD -MF .$@.d $(CFLAGS) -c $< -o $@
	$(Q)cp .$@.d .$@.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < .$@.d >> .$@.P; \
            mv .$@.P .$@.d

TAGS:
	@echo -e "\tTAGS\t"
	@etags *.[ch]

.PHONY: all clean TAGS

-include $(ALL_DEBS)
