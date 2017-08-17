CC=mips-openwrt-linux-gcc
#CFLAGS+=-g
CFLAGS+=-Wall
LDFLAGS+=-ljpeg
LDFLAGS+=-L/home/rolo/se/openwrt/staging_dir/target-mips_34kc_musl-1.1.16/usr/lib

PROG=cliente_v4l2
SRCS=utils
INCS=-I. -I$(SRCS) -I/home/rolo/se/openwrt/staging_dir/target-mips_34kc_musl-1.1.16/usr/include

OBJS =  $(PROG).o \
        $(SRCS)/YUYVtoJPEG.o \
        $(SRCS)/socket.o

.PHONY: all clean
$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS)  $(INCS) $<  -o $@

all: $(PROG)

clean:
	rm -f $(OBJS) $(PROG)

