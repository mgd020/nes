CC := cc
CPPFLAGS :=
CFLAGS :=
INCLUDES :=

LDFLAGS := -g -Wall
LDLIBS :=

MAIN := ./build/nes
BUILD_DIR := ./build
SRC_DIR := ./src

SRCS := $(shell find $(SRC_DIR) -name '*.c' -a ! -name 'test*')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

all: $(MAIN)

# main?
$(MAIN): $(OBJS) 
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^

# all tests
tests: test_*

# compile and run a test
test_%: $(BUILD_DIR)/test_%
	-$(BUILD_DIR)/$@

# make test exe from object file with same name
$(BUILD_DIR)/test_%: $(BUILD_DIR)/test_%.o
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^

# make object file from src file with same name
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -MMD -MP $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# include build/*.d deps
-include $(DEPS)

# object dependencies
$(BUILD_DIR)/test_cpu: $(patsubst %,$(BUILD_DIR)/%.o, bus ram test util cpu)

# remove build dir
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# ensure build dir exists
$(shell mkdir -p $(BUILD_DIR))
