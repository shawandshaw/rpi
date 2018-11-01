#ifndef _PID_H_
#define _PID_H_

typedef struct _pid {
	float SetSpeed;
	float ActualSpeed;
	float err;
	float err_last;
	float Kp, Ki, Kd;
	float voltage;
	float integral;
	float umax;
	float umin;
}Pid;


class Pid_control
{
public:

	void PID_init();
	float PID_realize(float speed);

private:
	int index;
	Pid pid;
};
#endif