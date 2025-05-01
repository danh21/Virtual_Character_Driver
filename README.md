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
- demo_RaceCondition/: demo for race condition
- Strace/: demo for strace feature

### Usage

- Open Terminal in main folder.
- To upload kernel module and add permissions to device file: 
  - $ sudo insmod Module/vchar_driver.ko
  - $ sudo chmod 666 /dev/vchar_dev
- To run sample application:
  - $ ./App/app
- To remove kernel module:
  - $ sudo rmmod vchar_driver
- To test **strace** utility for debugging:
  - $ cd Strace/ ; make ; cd ..
  - $ strace Strace/testStrace
- To create race condition:
  - $ ./demo_RaceCondition/concurrency.sh
  - Check data in critical resource: $ ./demo_RaceCondition/display_CR

