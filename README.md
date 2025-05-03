# 📦 Project Name

> Virtual Character Driver

---

## 📚 Table of Contents

- [📦 Project Name](#-project-name)
  - [📚 Table of Contents](#-table-of-contents)
  - [📝 About](#-about)
  - [✨ Features](#-features)
  - [🚀 Getting Started](#-getting-started)
    - [Prerequisites](#prerequisites)
    - [Source](#source)
    - [Usage](#usage)

---

## 📝 About

> Develop virtual character driver to load to kernel, then use sample app to interact to driver.

---

## ✨ Features

- ✅ Create virtual character device
- ✅ User can interact to device file (open/close file, read/write file)
- ✅ Simulate race condition
- ✅ Test strace feature

---

## 🚀 Getting Started

### Prerequisites

- List software dependencies or system requirements here:
  - Linux kernel
  - Ubuntu
  - C Linux
  - Makefile

### Source

- Module/: virtual character driver development
- App/: sample app
- demo_RaceCondition/: demo for race condition, test sync scheme
- Strace/: demo for strace feature (utility to debug)

### Usage

- To upload kernel module and add permissions to device file: 
  - Open terminal at Module/
  - $ make
  - $ sudo insmod Module/vchar_driver.ko
  - $ sudo chmod 666 /dev/vchar_dev
- To run sample application:
  - Open terminal at App/
  - $ make
  - $ ./app
- To remove kernel module:
  - $ sudo rmmod vchar_driver
- To test **strace** utility for debugging:
  - Open terminal at Strace/
  - $ make
  - $ strace testStrace
- To create race condition:
  - Open terminal at demo_RaceCondition/
  - $ make
  - To increase critical resource data by 3 cores at same time: $ . concurrency.sh
  - To increase critical resource data by 1 core: $ ./change_CR
  - To check critical resource data: $ ./display_CR
  - To reset critical resource data to 0: $ ./reset_CR
