INCLUDEDIR := $(PWD)/include
LOCAL_SRCS = $(wildcard *.c)
LOCAL_OBJS = $(patsubst %.c,%.o,$(LOCAL_SRCS))

.PHONY : all
all : $(LOCAL_OBJS) MV
$(LOCAL_OBJS) : $(LOCAL_SRCS)
	$(CC) $(CFLAGS) -g -c $^ -I$(INCLUDEDIR)
MV :
	mv ./$(LOCAL_OBJS) $(OBJS_DIR)

clean :
	rm *.o

