#define REG_SIZE 	1						                            //size of 1 register is 1 byte
#define NUM_CTRL_REGS 	1 						                        //number of control registers
#define NUM_STT_REGS 	5						                        //number of status registers
#define NUM_DATA_REGS 	256 						                    //number of data registers
#define NUM_DEV_REGS 	(NUM_CTRL_REGS + NUM_STT_REGS + NUM_DATA_REGS)	//number of registers
#define ENABLE 1
#define DISABLE 0



/****************** Define custom config: START ******************/
/* Select one mechanism of below options to protect critical resource */
#define USE_ATOMIC      DISABLE
#define USE_MUTEX       ENABLE
#define USE_SPINLOCK    DISABLE
#define USE_SEMAPHORE   DISABLE

/* Use timer to trigger event */
#define USE_TIMER       DISABLE



/****************** Define custom config: END ******************/



/****************** Define control register: START ******************/
/*
* CONTROL_ACCESS_REG:
* - contains bits which control read/write data regs 
* - init value: 0x03
* - meaning of bits:
* 	+ bit0:
*		0: not allowed to read from data regs
*		1: allowed to read from data regs
* 	+ bit1:
*		0: not allowed to write into data regs
*		1: allowed to write into data regs
* 	+ bit2~7: available
*/
#define CONTROL_ACCESS_REG 0

#define CTRL_READ_DATA_BIT (1 << 0)
#define CTRL_WRITE_DATA_BIT (1 << 1)
/****************** Define control register: END ******************/



/****************** Define status register: START ******************/
/*
* [READ_COUNT_H_REG:READ_COUNT_L_REG]:
* - init value: 0x0000
* - read from data regs successfully, inc 1 unit
*/
#define READ_COUNT_H_REG 0
#define READ_COUNT_L_REG 1

/*
* [WRITE_COUNT_H_REG:WRITE_COUNT_L_REG]:
* - init value: 0x0000
* - write into data regs successfully, inc 1 unit
*/
#define WRITE_COUNT_H_REG 2
#define WRITE_COUNT_L_REG 3

/*
* DEVICE_STATUS_REG:
* - init value: 0x03
* - meaning of bits:
* 	+ bit0:
*		0: data regs are not ready to read
*		1: data regs are ready to read
* 	+ bit1:
*		0: data regs are not ready to write
*		1: data regs are ready to write
* 	+ bit2:
*		0: when data regs are deleted, bit will be reset to 0
*		1: when all data regs are written, bit will be set to 1
* 	+ bit3~7: available
*/
#define DEVICE_STATUS_REG 4

#define STT_READ_ACCESS_BIT (1 << 0)
#define STT_WRITE_ACCESS_BIT (1 << 1)
#define STT_DATAREGS_OVERFLOW_BIT (1 << 2)

#define READY 1
#define NOT_READY 0
#define OVERFLOW 1
#define NOT_OVERFLOW 0
/****************** Define status register: END ******************/
