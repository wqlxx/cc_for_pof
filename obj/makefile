LOCAL_OBJ := $(wildcard *.o)  
CFLAGS = -o  
BIN = cc_main

  
$(BIN) : $(LOCAL_OBJ)  
	$(CC) $(CFLAGS) $@ $^
	-mv $(BIN) $(BIN_DIR) 

clean :
	rm *.o
