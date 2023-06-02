# Project: Virtual Character Driver

# Reference: 
    https://vimentor.com/vi/lesson/linux-device-driver

# Instructiom:
    - To upload kernel module: 
        + Open Terminal contains file vchar_driver.ko
        + Login with su
        + Enter cmd: insmod vchar_driver.ko
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