# device_mapper_proxy

This is a linux kernel module that creates virtual block devices on top of an existing device mapper and monitors operation statistics.

> Tested on Ubuntu 24.04 with kernel release 6.8.0-31-generic, gcc 13.2.0, make 4.3.
## prepare

```
$ sudo apt install build-essential linux-headers-`uname -r`
```

## makefile-based build

Install module and test it:

```
$ sudo make
```

Remove module and clean build directory:

```
$ sudo make clean
```

Alternatively, you can:

- Install module
```
$ sudo make dmp
```

- Test module
```
$ sudo make test
```

- Create test device
```
$ sudo make create_devices
```

- Remove test device
```
$ sudo make clean_devices
```

## manual build

Build module:

```
$ cd ./src && make
```

Install module:

```
$ sudo insmod dmp.ko
```

Remove module:

```
$ sudo rmmod dmp
```

Clean artifacts:

```
$ make clean
```

## usage

Creating a block device
```
$ sudo dmsetup create zero1 --table "0 $size zero"
$ sudo dmsetup create dmp1 --table "0 $size dmp /dev/mapper/zero1"
```

Example of operations:
```
$ sudo dd if=/dev/random of=/dev/mapper/dmp1 bs=4k count=1
$ sudo dd of=/dev/null if=/dev/mapper/dmp1 bs=4k count=1
```

Statistics:
```
cat /sys/module/dmp/stat/volumes
```

Deleting devices:
```
$ sudo dmsetup remove dmp1
$ sudo dmsetup remove zero1
```