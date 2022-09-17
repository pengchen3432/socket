// 编译成功动态库之后，运行的时候动态库有指定路径去搜索，不是说编译程就能运行，具体见下面连接
//  https://www.cnblogs.com/zhengmeifu/archive/2010/03/02/linux-gcc_compile_header_file_and_lib_path.html
#include <stdio.h>

#include <unistd.h>
typedef void (*callback)();
callback step;
int (*get_number)() = NULL;
void run()
{
    int c;
    if ( step ) 
        step();
    if ( get_number )
    {
        printf("number=%d\n", get_number());
    }
    sleep(1);
}       
void set_step(callback p)
{
    step = p;
    printf("my_step init success\n");
}
void set_number(int (*p)()){
    get_number = p;
}