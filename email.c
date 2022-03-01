#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
static int number_of_warnings = 0;

int submit_warning_to_email(int level, char *content, char *time) {
    char buf[1024] = {0};
    int fd = open("./mail_body", O_RDWR | O_APPEND);
    number_of_warnings++;
    if ( fd < 0 ) {
        syslog(LOG_ERR, "%s\n", strerror(errno));
        return -1;
    }
    snprintf(buf, sizeof(buf), "%d\t%s\t%s\n",level, content, time);
    if ( write(fd, buf, strlen(buf)) != strlen(buf) ) {
        syslog(LOG_ERR, "write file err:%s\n", strerror(errno));
        return -1;
    }
    if ( close(fd) < 0) {
        syslog(LOG_ERR, "close file err:%s\n", strerror(errno));
        return -1;
    }
    return 1;
}
int mail_body_init() {
    char label[36] = "level\tcontent\ttime\n";
    int fd = creat("./mail_body", 0664);
    if (fd < 0) {
        syslog(LOG_ERR, "create file err:%s\n", strerror(errno));
        return -1;
    }
    if (write(fd, label, strlen(label)) != strlen(label)) {
        syslog(LOG_ERR, "write file err:%s\n", strerror(errno));
        return -1;
    }
    if (close(fd) < 0) {
        syslog(LOG_ERR, "close file err:%s\n", strerror(errno));
        return -1;
    } 
    return 1;
}
void detection_warning() {
    int ret;
    char cmd[256] = {0};
    if ( (ret = mail_body_init()) < 0) {
        return;
    }
    while (1) {
        if (number_of_warnings > 0) {
            snprintf(cmd, sizeof(cmd), "sh -x /usr/sbin/email_template.sh %d", number_of_warnings);
            system(cmd);
            number_of_warnings = 0;
            if ( mail_body_init() < 0 ) {
                return;
            }
        }
        sleep(60);
    }
}

int main()
{
    int ret;
    ret = mail_body_init();
    printf("ret = %d\n", ret);
    ret = submit_warning_to_email(1, "Router GWN7062_hz (00:0B:82:A6:43:5C) memory usage reaches 60", "16:14");
    printf("ret = %d\n", ret);
}
