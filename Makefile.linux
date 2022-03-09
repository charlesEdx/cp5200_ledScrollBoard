

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++


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


C_SRCS		= led_cp5200.c test_ledCP5200.c
C_OBJS		= $(C_SRCS:.c=.o)

EXEC_TEST	= test_led

OBJS_ALL		= $(C_OBJS)
EXEC_ALL 		= $(EXEC_TEST)
DEPS_ALL		= $(OBJS_ALL:.o=.d)


all: $(EXEC_ALL)

$(EXEC_TEST): $(OBJS_ALL)
	@(echo "")
	@(echo ">>>>> Building $@ ...")
	$(CXX) -o $@ $^ $(LDFLAGS)
	@(echo "..... Succeeded!!")
	@(echo "")



# install:
# 	./install_falldt.sh

clean:
	@rm -f $(EXEC_ALL)
	@rm -f $(OBJS_ALL) $(DEPS_ALL)

.PHONY: clean all install $(EXEC_ALL)

sinclude $(DEPS_ALL)
