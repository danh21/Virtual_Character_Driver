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

- To upload kernel module: 
	+ Open Terminal in Module/
        + $ sudo insmod vchar_driver.ko
        + $ sudo chmod 666 /dev/vchar_dev  #for user to interact with device file
- To run application:
        + Open Terminal in App/
        + $ ./app
- To remove kernel module:
        + Open Terminal in Module/
        + $ sudo rmmod vchar_driver.ko
- To test strace:
        + Open Terminal in Strace/
        + $ strace ./testStrace
- To create race condition:
        + Open terminal in demo_RaceCondition/
	+ $ ./concurrency.sh
        + $ ./display_CR  #Check data in critical resource

