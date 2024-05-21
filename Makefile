BUILD_DIR := ./src/
SIZE ?= 512
COLOR := \n\033[1;32m
NO_COLOR := \033[00m\n
DIVIDER := ................

all: dmp test

.PHONY: dmp
dmp:
	@echo "${COLOR}${DIVIDER}MAKING MODULE${DIVIDER}${NO_COLOR}"
	make -C ${BUILD_DIR}
	@echo "${COLOR}${DIVIDER}INSTALLING MODULE${DIVIDER}${NO_COLOR}"
	insmod ${BUILD_DIR}dmp.ko

.PHONY: test
test:
	make create_devices
	@echo "${COLOR}${DIVIDER}TESTING DEVICES${DIVIDER}${NO_COLOR}"
	dd if=/dev/random of=/dev/mapper/dmp1 bs=4k count=1
	dd of=/dev/null if=/dev/mapper/dmp1 bs=4k count=1
	@echo "${COLOR}${DIVIDER}SHOW STATISTICS${DIVIDER}${NO_COLOR}"
	cat /sys/module/dmp/stat/volumes
	make clean_devices

.PHONY: create_devices
create_devices:
	@echo "${COLOR}${DIVIDER}CREATING DEVICES${DIVIDER}${NO_COLOR}"
	dmsetup create zero1 --table "0 ${SIZE} zero"
	dmsetup create dmp1 --table "0 ${SIZE} dmp /dev/mapper/zero1"

.PHONY: clean_devices
clean_devices:
	@echo "${COLOR}${DIVIDER}REMOVING DEVICES${DIVIDER}${NO_COLOR}"
	dmsetup remove dmp1
	dmsetup remove zero1

.PHONY: clean
clean:
	@echo "${COLOR}${DIVIDER}REMOVING MODULE${DIVIDER}${NO_COLOR}"
	rmmod dmp
	@echo "${COLOR}${DIVIDER}CLEANING BUILD DIRECTORY${DIVIDER}${NO_COLOR}"
	make -C ${BUILD_DIR} clean
