.SUFFIXES : .c .o

BASE = $(shell pwd)

Q_		  ?= 

CC        := gcc
MKDIR     := mkdir -p
CP 		  := cp

CFLAGS    := -O2 -Wall -fPIC
DEFINES   := -DDEBUG
LDFLAGS   := -lpthread

OUT_DIR   := obj
TARGET    := client

INCLUDES  := $(BASE)/.. $(BASE)
SRC_DIRS  := $(BASE)
SRCS      := main.c

#common
DEFINES   += -D_GNU_SOURCE
INCLUDES  += $(BASE)/../common
SRC_DIRS  += $(BASE)/../common
SRCS      += ipc.c \
             ipc_client.c \
             ipc_parcel.c \
             ipc_server.c \
             ipc_task.c

APP       := $(OUT_DIR)/$(TARGET)
APP_OBJS  := $(SRCS:%.c=$(OUT_DIR)/%.o) 
APP_FLAGS := $(DEFINES) $(addprefix -I, $(INCLUDES))


vpath %.c   $(SRC_DIRS)

.PHONY: all clean

all: $(OUT_DIR) $(APP)

$(APP): $(APP_OBJS)
	@echo "[Linking... $(notdir $(APP))]"
	$(Q_)$(CC) -o $(APP) $(LDFLAGS) $(APP_OBJS) $(LIBS)
	
$(OUT_DIR):
	$(Q_)$(MKDIR) $(OUT_DIR)
	
$(OUT_DIR)/%.o : %.c 
	@echo "[Compile... $(notdir $<)]" 
	$(Q_)$(CC) $(CFLAGS) $(APP_FLAGS) -c $< -o $@ 

clean:
	@echo "[Clean... all objs]"
	$(Q_)-$(RM) -rf $(OUT_DIR)

