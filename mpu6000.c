#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "global.h"
#include "mpu6000.h"
#include "RCC_Config.h"
#include "LED_Config.h"
#include "SPI.h"
#include "usart.h"


int16_t gyro_pitch, gyro_roll, gyro_yaw, acc_pitch, acc_roll, acc_yaw, temp;
int16_t gyro_x, gyro_y, gyro_z, acc_x, acc_y, acc_z;
static int32_t gyro_x_cal, gyro_y_cal, gyro_z_cal, cal_int;

bool isPresentMPU6000(void) {
	
	uint8_t res;
	
	ENABLE_MPU6000;
	res = SPITransfer(MPU6000_SPI, MPU6000_WHOAMI | 0x80);
	res = SPITransfer(MPU6000_SPI, 0x00);
	DISABLE_MPU6000;
	
	//return (res == 0x68);
	if (res == 0x68) {
		return 1;
	}
	
	return 0;
	
}


void initMPU6000(void) {
	
	// Device reset
	ENABLE_MPU6000;
	SPITransfer(MPU6000_SPI, MPU6000_PWR_MGMT_1);
	SPITransfer(MPU6000_SPI, BIT_H_RESET);
	DISABLE_MPU6000;
	
	// wait a bit
	delayMilliseconds(150);

	ENABLE_MPU6000;
	SPITransfer(MPU6000_SPI, 0x68);
	SPITransfer(MPU6000_SPI, 0x07);
	DISABLE_MPU6000;
	
	delayMilliseconds(150);
	
	// clock source PLL with Z axis gyro reference
	ENABLE_MPU6000;
	SPITransfer(MPU6000_SPI, MPU6000_PWR_MGMT_1);
	SPITransfer(MPU6000_SPI, MPU_CLK_SEL_PLLGYROZ);
	DISABLE_MPU6000;
	
	// wait a bit
	delayMilliseconds(10);
	
	// Disable I2C interface
	ENABLE_MPU6000;
	SPITransfer(MPU6000_SPI, MPU6000_USER_CTRL);
	SPITransfer(MPU6000_SPI, BIT_I2C_IF_DIS);
	DISABLE_MPU6000;
	
	// wait a bit
	delayMilliseconds(10);
	
	
	ENABLE_MPU6000;
  SPITransfer(MPU6000_SPI, MPU6000_PWR_MGMT_2);
  SPITransfer(MPU6000_SPI, 0x00);
  DISABLE_MPU6000;
	
	// wait a bit
	delayMilliseconds(10);
	
	// Accel sample rate 1000Hz, gyro sample rate 8000Hz
	ENABLE_MPU6000;
	SPITransfer(MPU6000_SPI, MPU6000_SMPLRT_DIV);
	SPITransfer(MPU6000_SPI, 0x00);
	DISABLE_MPU6000;
	
	// wait a bit
	delayMilliseconds(10);
	
	
	// Accel and gyro LPF set to ~43Hz
	ENABLE_MPU6000;
	SPITransfer(MPU6000_SPI, MPU6000_CONFIG);
	SPITransfer(MPU6000_SPI, BITS_DLPF_CFG_42HZ);
	DISABLE_MPU6000;
	
	// wait a bit
	delayMilliseconds(10);
	
	
	// Accel +/- 8G full scale
	ENABLE_MPU6000;
	SPITransfer(MPU6000_SPI, MPU6000_ACCEL_CONFIG);
	SPITransfer(MPU6000_SPI, BITS_FS_8G);
	DISABLE_MPU6000;
	
	// wait a bit
	delayMilliseconds(10);
	
	// Gyro +/- 500DPS full scale
	ENABLE_MPU6000;
	SPITransfer(MPU6000_SPI, MPU6000_GYRO_CONFIG);
	SPITransfer(MPU6000_SPI, BITS_FS_500DPS);
	DISABLE_MPU6000;
	
	
	// Set SPI clock to 12.5Mhz
	//SetSPIClockTo(MPU6000_SPI, 8);
	
	delayMilliseconds(100);
	
}


void readMPU6000(void) {
	
	uint8_t H, L;
	
	ENABLE_MPU6000;
	
	SPITransfer(MPU6000_SPI, 0x3B | 0x80);
	
	// Accel X
	H = SPITransfer(MPU6000_SPI, 0x00);
	L = SPITransfer(MPU6000_SPI, 0x00);
	acc_x = (int16_t) (H << 8) | L;
	
	// accel y
	H = SPITransfer(MPU6000_SPI, 0x00);
	L = SPITransfer(MPU6000_SPI, 0x00);
	acc_y = (int16_t) (H << 8) | L;
	
	// accel z
	H = SPITransfer(MPU6000_SPI, 0x00);
	L = SPITransfer(MPU6000_SPI, 0x00);
	acc_z = (int16_t) (H << 8) | L;
	
	// temperature
	H = SPITransfer(MPU6000_SPI, 0x00);
	L = SPITransfer(MPU6000_SPI, 0x00);
	temp = (int16_t) (H << 8) | L;
	
	// gyro x
	H = SPITransfer(MPU6000_SPI, 0x00);
	L = SPITransfer(MPU6000_SPI, 0x00);
	gyro_x = (int16_t) (H << 8) | L;
	
	// gyro y
	H = SPITransfer(MPU6000_SPI, 0x00);
	L = SPITransfer(MPU6000_SPI, 0x00);
	gyro_y = (int16_t) (H << 8) | L;
	
	// gyro z
	H = SPITransfer(MPU6000_SPI, 0x00);
	L = SPITransfer(MPU6000_SPI, 0x00);
	gyro_z = (int16_t) (H << 8) | L;
	
	DISABLE_MPU6000;
	
#ifdef USE_COMP_FILTER
	acc_pitch *= -1;
	acc_roll *= -1;
	gyro_yaw *= -1;
	gyro_roll *= -1;
#endif
	
	if (cal_int == 2000) {
		gyro_x -= gyro_x_cal;
		gyro_y -= gyro_y_cal;
		gyro_z -= gyro_z_cal;
	}
	
}


void calibrateMPU6000(void) {
	
	cal_int = 0;
	gyro_x_cal = 0;
	gyro_y_cal = 0;
	gyro_z_cal = 0;
	
	// take 5000 reading samples and average them
	for (cal_int = 0; cal_int < 2000; cal_int += 1) {
		if (cal_int % 25 == 0) {
			LED_TOGGLE;
		}
		
		readMPU6000();
		
		gyro_x_cal += gyro_x;
		gyro_y_cal += gyro_y;
		gyro_z_cal += gyro_z;
		
		delayMilliseconds(4);
	}
	
	//GPIOC->ODR |= (1 << 13) | (1 << 14);
	LED_OFF;
	
	//averaging 5000 samples
	gyro_x_cal /= 2000;
	gyro_y_cal /= 2000;
	gyro_z_cal /= 2000;
	
	printMsg("%d %d %d %d\n", gyro_x_cal, gyro_y_cal, gyro_z_cal, cal_int);
	
}
