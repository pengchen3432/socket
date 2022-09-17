#include <stdio.h>
static int a = 1;
void mystep_cb(void)
{
    printf( "回调函数\n");
    return;
}
int number_cb(){
    return a;
}

void main()
{

    set_step(mystep_cb);
    set_number(number_cb);
    while ( 1 ) {
        run();
        a++;

    }
    return ;
}