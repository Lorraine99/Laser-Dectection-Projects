#include "gpiolib_addr.h"

#include "gpiolib_reg.h"



#include <stdlib.h>

#include <stdio.h>

#include <stdint.h>

#include <time.h>



//initializing

GPIO_Handle initializeGPIO() {

	GPIO_Handle gpio;

	gpio = gpiolib_init_gpio();

	if(gpio == NULL) {

		perror("Could not initialize GPIO");

	}

	return gpio;

}



//change pin values, set output

void setOutput(GPIO_Handle gpio, int pinNumber) {

	if(gpio == NULL) {

		printf("The GPIO has not been initialized properly \n");

		return;

	}

	

	if(pinNumber < 2 || pinNumber > 27) {

		printf("Not a valid pinNumber \n");

		return;

	}

	

	//create register number

	int registerNum = pinNumber / 10;

	

	//create number of bits shifted (int)

	int bitShift = (pinNumber % 10) * 3;

	

	uint32_t sel_reg = gpiolib_read_reg(gpio, GPFSEL(registerNum));

	sel_reg |= 1 << bitShift;

	gpio_write_reg(gpio, GPFSEL(1), sel_reg);

}

//4 and 22

#define LASER1_PIN_NUM 4

#define LASER2_PIN_NUM 17

int laserDiodeStatus(GPIO_Handle gpio, int diodeNumber) {

	if(gpio == NULL) {

		return -1;

	}

	

	if(diodeNumber == 1) {

		uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));

		//if diode is shined

		if(level_reg & (1 << LASER1_PIN_NUM)) {

			return 1;

		} else {

			return 0;

		}

	} else if (diodeNumber == 2) {

		uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));

		if(level_reg & (1 << LASER2_PIN_NUM)) {

			return 1;

		} else {

			return 0;

		}

	} else {

		return -1;

	}

}



//turn on and off

void outputOff(GPIO_Handle gpio, int pinNumber) {

	gpiolib_write_reg(gpio, GPCLR(0), 1 << pinNumber);

}

void outputOn(GPIO_Handle gpio, int pinNumber) {

	gpiolib_write_reg(gpio, GPSET(0), 1 << pinNumber);

}





int stateMachine(const int timeLimit) {

	enum State {START, DONE, GOT_LEFT, GOT_RIGHT, NONE, BOTH};

	

	enum State s = START;

	

	int in = 0;

	int out = 0;

	int fromL = 0;

	int fromR = 0;

	int pin_state_L;

	int pin_state_R;

	int numL = 0;

	int numR = 0;

	

	GPIO_Handle gpio = initializeGPIO();

	/* setOutput(gipo,17); */

	

	time_t startTime = time(NULL);

	

	while((time(NULL) - startTime) < timeLimit) {

		switch(s) {

			

			case START:

				//checking pin_state for left and right photo diodes. 

				pin_state_L = laserDiodeStatus(gpio, 1);

				pin_state_R = laserDiodeStatus(gpio, 2);

				//initializing booleans

				fromL = 0;

				fromR = 0;

				if(pin_state_L) {

					fromL = 1;

					numL++;

					s = GOT_LEFT;

				} else if(pin_state_R) {

					numR++;

					fromR = 1;

					s = GOT_RIGHT;

				} else {

					return -1;

				}

				break;

				

			case GOT_LEFT:

				//checking pin_states

				pin_state_L = laserDiodeStatus(gpio, 1);

				pin_state_R = laserDiodeStatus(gpio, 2);

				if(pin_state_R) {

					numR++;

					s = BOTH;

				} else if (fromL&&!pin_state_L) {

					s = NONE;

				} else if (fromR&&!pin_state_L) {

					s = NONE;

					out++;

				} else {

					return -1;

				}

				break;

				

			case GOT_RIGHT:

				//checking pin states

				pin_state_L = laserDiodeStatus(gpio, 1);

				pin_state_R = laserDiodeStatus(gpio, 2);

				if(pin_state_L) {

					numL++;

					s = BOTH;

				} else if (fromR&&!pin_state_R) {

					s = NONE;

				} else if (fromL&&!pin_state_R) {

					s = NONE;

					in++;

				} else {

					return -1;

				}

				break;

				

			case BOTH:

				//checking pin states

				pin_state_L = laserDiodeStatus(gpio, 1);

				pin_state_R = laserDiodeStatus(gpio, 2);

				if(fromL&&!pin_state_L) {

					s = GOT_RIGHT;

				} else if (fromL&&!pin_state_R) {

					s = GOT_LEFT;

				} else if (fromR&&!pin_state_L) {

					s = GOT_RIGHT;

				} else if (fromR&&!pin_state_R) {

					s = GOT_LEFT;

				} else {

					return -1;

				}

				break;

				

			case NONE:

				s = START;

				break;

				

			

			default:

				break;

		}

	}

}

int main(const int argc, const char* const argv[]) {

	if(argc < 2) {

		printf("Error, no time given: exitting\n");

		return -1;

	}

	int timeLimit= atoi(argv[1]);

}