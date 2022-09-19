#include <string.h>
#include <stdio.h>
int main()
{
    char dst[] = "xiaohong 23 1998,xiaoming 24 1997,chen 25 1996";
    char *buf = dst;
    char *p[20];
    int in = 0;
    char *net_prt = NULL;
    char *pre_prt = NULL;
    while ( (p[in] = strtok_r(buf, ",", &net_prt)) != NULL) {
        // printf("net_prt = %p\n", p[in]);
        // printf("net_prt = %s\n", p[in]);
        // printf("net_prt = %p\n", net_prt);
        // printf("net_prt = %s\n", net_prt);
        
        buf = p[in];
        while ( (p[in] = strtok_r(buf, " ", &pre_prt)) != NULL ) {
            printf("%s\n", p[in]);
            in++;
            buf = NULL;

        }
    }
    // printf("dst=%p\n",dst);
    // printf("dst=%s\n",dst);
    // printf("buf=%p\n", buf);
    // printf("buf=%s\n", buf);
    // p = strtok(buf, ",");
    // printf("p = %p\n", p);
    // printf("p = %s\n", p);
    // p = strtok(NULL, ",");
    // printf("p = %p\n", p);
    // printf("p = %s\n", p);
}