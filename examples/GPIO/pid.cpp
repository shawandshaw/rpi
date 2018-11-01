#include <iostream>
#include "pid.h"

using namespace std;

void  Pid_control::PID_init()
{
	pid.SetSpeed = 0.0;
	pid.ActualSpeed = 0.0;
	pid.err = 0.0;
	pid.err_last = 0.0;
	pid.voltage = 0.0;
	pid.integral = 0.0;
	pid.Kp = 0.4;
	pid.Ki = 0.2;
	pid.Kd = 0.2;
	pid.umax = 400;
	pid.umin = -200;
}

float Pid_control::PID_realize(float speed) {
	float index;
	pid.SetSpeed = speed;
	pid.err = pid.SetSpeed - pid.ActualSpeed;

	if (abs(pid.err) > 200) //波动性标准分析
	{
		index = 0.0;
	}
	else if (abs(pid.err) < 180) {
		index = 1.0;
		pid.integral += pid.err;
	}
	else {
		index = (200 - abs(pid.err)) / 20;
		pid.integral += pid.err;
	}
	pid.voltage = pid.Kp*pid.err + index * pid.Ki*pid.integral + pid.Kd*(pid.err - pid.err_last);

	pid.err_last = pid.err;
	pid.ActualSpeed = pid.voltage*1.0;
	return pid.ActualSpeed;
}