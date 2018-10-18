#include<iostream>
#include <stdlib.h>
#include "GPIOlib.h"

using namespace std;
using namespace GPIO;

#define TTY_PATH            "/dev/tty"
#define STTY_US             "stty raw -echo -F "
#define STTY_DEF            "stty -raw echo -F "

static int get_char();

static int get_char()
{
    fd_set rfds;
    struct timeval tv;
    int ch = 0;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 10; //设置等待超时时间

    //检测键盘是否有输入
    if (select(1, &rfds, NULL, NULL, &tv) > 0)
    {
        ch = getchar(); 
    }

    return ch;
}

int main()
{
    int ch = 0;
    system(STTY_US TTY_PATH);

	init();
	int speed=0;
	int abs_speed=0;
	int direction=FORWARD;
	int angle=0;
	turnTo(angle);

    while(1)
    {
		stopLeft();
		stopRight();
        ch = get_char();
        if (ch)
        {
            printf("key = %d(%c)\n\r", ch, ch);
            switch (ch)
            {
                case 3: //ctrl+c
					stopLeft();
					stopRight();
                    system(STTY_DEF TTY_PATH);
					return 0;
                case 'w':{
					if(speed<80)speed++;
					if(speed>0)direction=FORWARD;
					else abs_speed=speed;
					cout<<direction<<endl;
					cout<<speed<<endl;
					controlLeft(FORWARD,50);
					controlRight(FORWARD,50);
					delay(1000);
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
					controlLeft(BACKWARD,50);
					controlRight(BACKWARD,50);
					delay(1000);
					break;
				}
                    
                case 'a':{
					if(angle>-45)angle--;
					cout<<angle<<endl;
					turnTo(angle);
					break;
				}
                case 'd':{
					if(angle<45)angle++;
					cout<<angle<<endl;
					turnTo(angle);
					break;
				}
            }
        }           
    }
}