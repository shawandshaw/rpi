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
	controlLeft(FORWARD,20);
	controlRight(FORWARD,20);
	delay(3000);

	//Stop
	stopLeft();
	stopRight();

	return 0;
}
