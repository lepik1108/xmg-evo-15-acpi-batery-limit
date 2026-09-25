obj-m += xmg_ec_battery.o

KDIR ?= /lib/modules/$(shell uname -r)/build
BUILD_DIR ?= $(PWD)/build

all:
	@mkdir -p $(BUILD_DIR)
	@ln -sf $(PWD)/xmg_ec_battery.c $(BUILD_DIR)/xmg_ec_battery.c
	@echo "obj-m += xmg_ec_battery.o" > $(BUILD_DIR)/Kbuild
	make -C $(KDIR) M=$(BUILD_DIR) LLVM=1 CC=clang modules

clean:
	rm -rf $(BUILD_DIR)

