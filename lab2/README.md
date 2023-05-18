# Лабораторная работа 2

**Название:** "Разработка драйверов блочных устройств"

**Цель работы:** Получить знания и навыки разработки драйверов блочных устройств для операционной системы Linux.

## Описание функциональности драйвера

Создается два первичных раздела и один расширенный с размерами 10, 20 и 20 Мб соотвественно. Расширенный разделяется на два логических раздела, оба размером 10 Мб.

## Инструкция по сборке

```bash
make
```

## Инструкция пользователя

```bash
insmod io2.ko
```

## Примеры использования

Пример просмотра разделов диска:

```bash
sudo fdisk /dev/vramdisk -l
```
Disk /dev/vramdisk: 50 MiB, 52428800 bytes, 102400 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0x36e5756d

Пример создания файловых систем, на созданном диске:
```bash
al@al-VirtualBox:~/Desktop/o$ sudo mkfs.vfat /dev/vramdisk1
```
mkfs.fat 4.2 (2021-01-31)
```bash
al@al-VirtualBox:~/Desktop/o$ sudo mkfs.vfat /dev/vramdisk2
```
mkfs.fat 4.2 (2021-01-31)
```bash
al@al-VirtualBox:~/Desktop/o$ sudo mkfs.vfat /dev/vramdisk3
```
mkfs.fat 4.2 (2021-01-31)
mkfs.vfat: Attempting to create a too small or a too large filesystem
```bash
al@al-VirtualBox:~/Desktop/o$ sudo mkfs.vfat /dev/vramdisk5
```
mkfs.fat 4.2 (2021-01-31)
```bash
al@al-VirtualBox:~/Desktop/o$ sudo mkfs.vfat /dev/vramdisk6
```
mkfs.fat 4.2 (2021-01-31)

Получаем информацию о диске:
```bash
lsblk
```
NAME    	MAJ:MIN RM  SIZE RO TYPE MOUNTPOINTS
sda       	8:0	0   20G  0 disk
├─sda1    	8:1	0	1M  0 part
├─sda2    	8:2	0  513M  0 part /boot/efi
└─sda3    	8:3	0 19,5G  0 part /
sr0      	11:0	1 1024M  0 rom  
vramdisk	252:0	0   50M  0 disk
├─vramdisk1 252:1	0   10M  0 part
├─vramdisk2 252:2	0   20M  0 part
├─vramdisk3 252:3	0	1K  0 part
├─vramdisk5 252:5	0   10M  0 part
└─vramdisk6 252:6	0   10M  0 part
