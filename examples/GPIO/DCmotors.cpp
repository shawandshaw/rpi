#include "GPIOlib.h"
#include <signal.h>
#include <stdlib.h>
using namespace GPIO;


void on_exit(void)
{
    stopLeft();
	stopRight();
}
void signal_crash_handler(int sig) { 
	on_exit(); 
	exit(-1); 
} 
void signal_exit_handler(int sig) { 
	exit(0); 
}


int main()
{
	init();
	atexit(on_exit);
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);

    // ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    signal(SIGBUS, signal_crash_handler);     // 总线错误
    signal(SIGSEGV, signal_crash_handler);    // SIGSEGV，非法内存访问
    signal(SIGFPE, signal_crash_handler);       // SIGFPE，数学相关的异常，如被0除，浮点溢出，等等
    signal(SIGABRT, signal_crash_handler);

	//Move forward
	controlLeft(FORWARD,50);
	controlRight(FORWARD,50);
	delay(1000);

	//Stop
	stopLeft();
	stopRight();
	delay(1000);

	//Move backward
	controlLeft(BACKWARD,50);
	controlRight(BACKWARD,50);
	delay(1000);

	//Stop
	stopLeft();
	stopRight();
	delay(1000);

	//2 motors can work at different speeds.
	controlLeft(FORWARD,30);
	controlRight(FORWARD,40);
	delay(1000);

	//Stop
	stopLeft();
	stopRight();
	delay(1000);

	//Even directions can differ from each other.
	controlLeft(BACKWARD,35);
	controlRight(FORWARD,20);
	delay(1000);

	//Don't forget to stop all motors before exiting.
	stopLeft();
	stopRight();
	return 0;
}
