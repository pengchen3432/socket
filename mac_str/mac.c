#include <stdio.h>
#include <string.h>
#define MAC2STR(a) (a)[0],(a)[1],(a)[2],(a)[3],(a)[4],(a)[5]
#define MAC_STR_LEBN 18
#define COMPACT_MACSTR "%02x%02x%02x%02x%02x%02x"
int main()
{
    char mac[] = "1a1b1c";
    char str_mac[MAC_STR_LEBN] = {0};
    char regular[MAC_STR_LEBN] = {0};
    char test[16] = {0};
    snprintf( str_mac, MAC_STR_LEBN, COMPACT_MACSTR, MAC2STR(mac) );
    printf("%s\n",str_mac);
    sscanf(mac, "%*[1-9,a-b]%s", test);
    printf("test=%s\n", test);
}