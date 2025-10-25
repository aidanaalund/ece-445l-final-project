#include "Motor.h"
#include "PulseWidthMeasure.h"
#include <stdint.h>
#include "../inc/PLL.h"
#include "../inc/PWM.h"
#include "../inc/Timer1A.h"
#include "Buttons.h"

#define LENGTHFIXEDPOINT 60
#define MINPWM 10000
#define MAXPWM 59000
#define MAXHEIGHT 4

// Gains for tube 1 and tube 2 (OR MAYBE REGIONS OF THE TUBE)
static int32_t Kp1 = 1024;//2048
static int32_t Kp2 = 1;//1
static int32_t Ki1 = 1;//1
static int32_t Ki2 = 2;//2

static int32_t DesiredPosition = 0;
static int32_t LeftPosition = 0;
static int32_t RightPosition = 0;
static int32_t LeftU = 0;
static int32_t RightU = 0;
static int32_t LeftE = 0;
static int32_t RightE = 0;

static int32_t LeftBuf[100] = {0};
static int32_t RightBuf[100] = {0};
static int32_t AveLeftE = 0;
static int32_t LeftSum = 0;
static int32_t AveRightE = 0;
static int32_t RightSum = 0;

//Distance from sensor in cm
uint32_t oscillatingDistance [512] = {
30, 30, 30, 31, 31, 31, 31, 32, 32, 32, 32, 33, 33, 33, 33, 34,
34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 36, 37, 37, 37, 37, 37,
38, 38, 38, 38, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 41, 41,
41, 41, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 44, 44, 44,
44, 44, 44, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46,
47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 48,
48, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50,
50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
50, 50, 50, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 47, 47, 47, 47, 47, 47,
47, 46, 46, 46, 46, 46, 46, 46, 45, 45, 45, 45, 45, 45, 44, 44,
44, 44, 44, 44, 43, 43, 43, 43, 43, 42, 42, 42, 42, 42, 42, 41,
41, 41, 41, 40, 40, 40, 40, 40, 39, 39, 39, 39, 39, 38, 38, 38,
38, 37, 37, 37, 37, 37, 36, 36, 36, 36, 35, 35, 35, 35, 34, 34,
34, 34, 33, 33, 33, 33, 32, 32, 32, 32, 31, 31, 31, 31, 30, 30,
30, 30, 30, 29, 29, 29, 29, 28, 28, 28, 28, 27, 27, 27, 27, 26,
26, 26, 26, 25, 25, 25, 25, 24, 24, 24, 24, 23, 23, 23, 23, 23,
22, 22, 22, 22, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 19, 19,
19, 19, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 16, 16, 16,
16, 16, 16, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14,
13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12,
12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10,
10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13,
13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16,
16, 16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 19,
19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22,
22, 23, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25, 26, 26,
26, 26, 27, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30
};

uint32_t hover = 40;
uint32_t oscillation = 31;
uint32_t tubeMode = 1;

void MotorInit(){
	PWM0E_Init(60000,50000); // 667hz, 75% duty on PE4 fan1 left
	PWM0F_Init(60000,50000); // 667hz, 75% duty on PE5 fan2 right
	Timer1A_Init(&PIControlLoop,5000000,2); // samplingTime = 0.0625 is 2500000, half that is 1250000, 
}

//int bu = 0; //TO DELETE

void PIControlLoop(void){
	
	//Test Burak
//	int be = 9000 - GetRightPosition(); //10 to 120,000
//	int bp = (be * Kp1)/Kp2;
//	bu += (be * Ki1)/Ki2;
//	bu = bu < MINPWM ? MINPWM : bu;
//	bu = bu > MAXPWM ? MAXPWM : bu;
//	int bt = bp + bu;
//	bt = bt < MINPWM ? MINPWM : bt;
//	bt = bt > MAXPWM ? MAXPWM : bt;
//	PWM0F_Duty(bt);
//	return;
	//End Test Burak

	//tubeMode = getMode();
	//tubeMode = 1; // debug
	tubeMode = 2; // debug
	
	int32_t oldLeft = LeftPosition;
	int32_t oldRight = RightPosition;
	int32_t oldDesired = DesiredPosition;
	
	LeftPosition = ((((int32_t)GetLeftPosition() / 2320)*3) + oldLeft) / 4;
	RightPosition = ((((int32_t)GetRightPosition() / 2320)*3) + oldRight) / 4;
	//LeftPosition = (int32_t)GetLeftPosition() / 2320;
	//RightPosition = (int32_t)GetRightPosition() / 2320;
	
	if(LeftPosition > 60){LeftPosition = 60;}
	if(RightPosition > 60){RightPosition = 60;}
	
	// Determine ERROR
	//Mode0 -> oscillating seperately
	if (tubeMode == 0) { // TODO: oscillating mode is just a underdamped set of gains...
		//LeftE = (int32_t)() - (int32_t)(LENGTHFIXEDPOINT-LeftPosition);
		RightE = (int32_t)(oscillatingDistance[oscillation%512]) - (int32_t)(LENGTHFIXEDPOINT-RightPosition);
	} else if ( tubeMode ==1){
		LeftE = (int32_t)(hover) - (int32_t)(LENGTHFIXEDPOINT-LeftPosition);
		RightE = (int32_t)(hover) - (int32_t)(LENGTHFIXEDPOINT-RightPosition);
			
	} else {
		DesiredPosition = ((((int32_t)GetDesiredPosition() / 2320)*3) + oldDesired) / 4;
		if(DesiredPosition > 55){DesiredPosition = 55;}
		LeftE = (int32_t)(DesiredPosition) - (int32_t)(LENGTHFIXEDPOINT-LeftPosition);
		RightE = (int32_t)(DesiredPosition) - (int32_t)(LENGTHFIXEDPOINT-RightPosition);
	}

	// Proportional terms
	int32_t P1 = (Kp1*LeftE)/Kp2;
	int32_t P2 = (Kp1*RightE)/Kp2;

	if(P1 < -10000) P1 = -10000;         // Minimum P
	if(P1 > MAXPWM) P1 = MAXPWM;       
	if(P2 < -10000) P2 = -10000;         // Minimum P
	if(P2 >MAXPWM) P2 = MAXPWM;       

	// Integral terms from SUM(KiDt)   
	static int32_t I1 =  0;
	static int32_t I2 = 0;

	// sliding window average
	static int32_t idx = 0;

	// Remove oldest value
	LeftSum -= LeftBuf[idx];
	RightSum -= RightBuf[idx];

	// Add new value
	LeftBuf[idx] = LeftE;
	RightBuf[idx] = RightE;
	LeftSum += LeftE;
	RightSum += RightE;

	idx = (idx+1)%100;

	AveLeftE = LeftSum/100; 
	AveRightE = RightSum/100;

	I1 = I1 + (Ki1*LeftE)/Ki2;
	I2 = I2 + (Ki1*RightE)/Ki2;      
	
	if(I1 > MAXPWM){I1 = MAXPWM;} // TODO: retune these
	if(I1 < -4000){I1 = -4000;}
	if(I2 > MAXPWM){I2 = MAXPWM;}
	if(I2 < -4000){I2 = -4000;}
	
	LeftU = P1 + I1;
	RightU = P2 + I2;

	// TODO: tune these to avoid crashing or falling down fast.
	
	if(LeftU < MINPWM)  LeftU= MINPWM;           // Minimum PWM output
	if(LeftU > MAXPWM) LeftU= MAXPWM;         
	if(RightU < MINPWM)  RightU= MINPWM;           // Minimum PWM output
	if(RightU > MAXPWM) RightU= MAXPWM;         
	
	// TODO: check for ball too close to sensors, if so, turn off to avoid damaging the sensor.
	if(LeftPosition < 8){
		PWM0E_Duty(2*MINPWM);
	} else {
		PWM0E_Duty(LeftU);
	}
	
	if(RightPosition < 5){
		PWM0F_Duty(2*MINPWM);
	} else {
		PWM0F_Duty(RightU);
	}
	
}

int mainMotor(){
	PLL_Init(Bus80MHz);	// bus clock at 80 MHz

  while(1){
		
  }
}