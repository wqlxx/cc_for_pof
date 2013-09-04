INCLUDEDIR := -I$(PWD)/include 
CFLAGS += $(INCLUDEDIR)
LOCAL_OBJS = cc_main.o
.PHONY : all
all : $(LOCAL_OBJS) MV
$(LOCAL_OBJS) :%.o:%.c
	$(CC) $(CFLAGS) -g -c $^ -o $@
MV :
	mv ./$(LOCAL_OBJS) $(OBJS_DIR) 

