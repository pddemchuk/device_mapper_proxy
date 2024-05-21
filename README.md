# device_mapper_proxy
> Tested on Ubuntu 24.04 with kernel release 6.8.0-31-generic, gcc 13.2.0, make 4.3

This is a linux kernel module that creates virtual block devices on top of an existing device mapper and monitors operation statistics.

## dependencies

```
$ apt-get install build-essential linux-headers-`uname -r`
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

Alternativly, you can:

- Install module
```
$ sudo make dmp
```

- Test module
```
$ sudo make test
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

Creating a test block device
```
$ sudo dmsetup create zero1 --table "0 $size zero"
```
Note: $size - произвольный размер устройства.

Creating a proxy block device
```
$ sudo dmsetup create dmp1 --table "0 $size dmp /dev/mapper/zero1"
```
Note: $size - размер устройства /dev/mapper/zero1.

Statistics:
```
cat /sys/module/dmp/stat/volumes
```

Delete device:
```
$ sudo dmsetup remove dmp1
```