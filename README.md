# Project: Virtual Character Driver

# Reference: 
    https://vimentor.com/vi/lesson/linux-device-driver

# Instruction:
    - To upload kernel module: 
        + Open Terminal contains file vchar_driver.ko
        + Login with su
        + Enter cmd: insmod vchar_driver.ko
        + Enter cmd: chmod 666 /dev/vchar_dev -> for user to interact with device file
    - To run application:
        + Open Terminal in App folder
        + Enter cmd: ./app
    - To remove kernel module:
        + Open Terminal contains file vchar_driver.ko
        + Login with su
        + Enter cmd: rmmod vchar_driver.ko
    - To test strace:
        + Open Terminal in Strace folder
        + Enter cmd: strace ./testStrace
    - To create race condition:
        + Open terminal in demo_RaceCondition folder
		+ Enter cmd: ./concurrency.sh
        + Check data in critical resource by entering cmd: ./display_CR
