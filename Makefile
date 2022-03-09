
ifeq ($(BUILD_VS8AI), 1)
CUR_PATH ?= $(shell pwd)
ROOT_DIR ?= $(CUR_PATH)/../../../../../..
include $(ROOT_DIR)/source/Rules.mk

XPKG_CFLAGS	= -I$(PKG_INC)
XPKG_LDLIBS	= -L$(PKG_LIB)
endif


## Color
HL_RED := $(shell tput -Txterm setaf 1)
HL_NC  := $(shell tput -Txterm sgr0)


CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar


%.o:%.c
	@(echo "")
	@(echo ">>>>> [C] compiling $< ...")
	@$(CC) -MM -MF $(@:.o=.d) $(CFLAGS) $<
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo ""
	@echo ">>>>> [CPP] compiling $< ..."
	@$(CC) -MM -MF $(@:.o=.d) -std=c++98 $(CFLAGS) $<
	@$(CC) -std=c++98 $(CFLAGS) -c $< -o $@

#----------------------------------------------------------
#
#----------------------------------------------------------
C_SRCS		= led_cp5200.c
C_OBJS		= $(C_SRCS:.c=.o)

EXEC_TEST	= test_led

OBJS_ALL		= $(C_OBJS)
EXEC_ALL 		= $(EXEC_TEST)
DEPS_ALL		= $(OBJS_ALL:.o=.d)

LIB_CP5K2	= libcp5k2_led.a

all: $(LIB_CP5K2)

$(LIB_CP5K2): $(C_OBJS)
	@(echo "${HL_RED}*********************************${HL_NC}")
	@(echo "${HL_RED}** Building $@ ...${HL_NC}")
	@(echo "${HL_RED}*********************************${HL_NC}")
	@$(AR) rcs $@ $^

# $(EXEC_TEST): $(OBJS_ALL)
# 	@(echo "")
# 	@(echo ">>>>> Building $@ ...")
# 	$(CXX) -o $@ $^ $(LDFLAGS)
# 	@(echo "..... Succeeded!!")
# 	@(echo "")



clean:
	@rm -f $(EXEC_ALL)
	@rm -f $(LIB_CP5K2) ../$(LIB_CP5K2)
	@rm -f $(OBJS_ALL) $(DEPS_ALL)

.PHONY: clean all install $(EXEC_ALL)

sinclude $(DEPS_ALL)
