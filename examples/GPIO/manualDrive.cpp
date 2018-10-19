#include<iostream>
#include <stdlib.h>
#include <signal.h>
#include "GPIOlib.h"

using namespace std;
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
    int ch = 0;

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

	int speed=0;
	int abs_speed=0;
	int direction=FORWARD;
	int angle=0;
	turnTo(angle);

    while(1)
    {
		fflush(stdin);
        ch = getchar();
        if (ch)
        {
            printf("key = %d(%c)\n\r", ch, ch);
            switch (ch)
            {
                case 'w':{
					if(speed<80)speed++;
					if(speed>0)direction=FORWARD;
					else abs_speed=speed;
					cout<<direction<<endl;
					cout<<speed<<endl;
					controlLeft(FORWARD,abs_speed);
					controlRight(FORWARD,abs_speed);
					delay(500);
					break;
				}
                    
                case 's':{
					if(speed>-80)speed--;
					if(speed<0){
						direction=BACKWARD;
						abs_speed=-speed;
					}
					cout<<direction<<endl;
					cout<<speed<<endl;
					controlLeft(BACKWARD,abs_speed);
					controlRight(BACKWARD,abs_speed);
					delay(500);
					break;
				}
                    
                case 'a':{
					if(angle>-45)angle--;
					cout<<angle<<endl;
					turnTo(angle);
					delay(500);
					break;
				}
                case 'd':{
					if(angle<45)angle++;
					cout<<angle<<endl;
					turnTo(angle);
					delay(500);
					break;
				}
            }
        }           
    }
}