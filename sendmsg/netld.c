#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/types.h>
#include <net/sock.h>
#include <net/netlink.h>
#define MAX_PAYLOAD 1024 // maximum payload size
#define NETLINK_CHEN 28 //自定义的协议
struct sock* nl_sk = NULL;


void send_msg(
    int pid
)
{
    int ret;
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlhdr = NULL;
    char temp[] = "niubi";
    int len = NLMSG_SPACE(MAX_PAYLOAD);
    //分配一个sk_buff， 头 + data_len
    printk("pid = %d\n", pid);
    skb = nlmsg_new(len, GFP_ATOMIC);
    if (NULL == skb)
    {
        printk(KERN_ERR "netlink_send_msg: alloc_skb Error.\n");
        return ;
    }
    printk("11111111111\n");
    //头赋值
    nlhdr = nlmsg_put(skb, 0, 0, 0, MAX_PAYLOAD, 0);
    if (NULL == nlhdr)
    {
        printk(KERN_ERR "netlink_send_msg: nlmsg_put Error.\n");
        nlmsg_free(skb);
        return ;
    }
        printk("222222222222\n");
    //payload 赋值
    memcpy(nlmsg_data(nlhdr), temp, strlen(temp) + 1);
        printk("333333333333\n");
    //单播 向 user_pid 发送数据 user_pid在接收的回调函数中可获得
    ret = netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
    return ;
}

void recv_msg(
    struct sk_buff *__skb
)
{
    struct sk_buff *skb;
    struct nlmsghdr *nl;
    int pid;
    skb = skb_get(__skb);
    if ( skb->len >= NLMSG_SPACE(0) ) {
        nl = nlmsg_hdr(skb);
        printk("recvmsg:%s", (char*)NLMSG_DATA(nl));
        pid = nl->nlmsg_pid;
        send_msg(pid);
    }
}

static void netlink_exit(void)
{
    if(nl_sk != NULL){
        sock_release(nl_sk->sk_socket);
    }
    printk("my_net_link: self module exited\n");
}

static struct netlink_kernel_cfg cfg = {
    .input = recv_msg,
};

int netlink_init(void) {
    nl_sk = netlink_kernel_create(&init_net, NETLINK_CHEN, &cfg);
    if ( !nl_sk ) {
        printk( KERN_ERR "my netlink creat err\n" );
        return -1;
    }
    printk("create my nelink success\n");
    return 0;
}
module_init(netlink_init);
module_exit(netlink_exit);
MODULE_AUTHOR("pengchen");
MODULE_LICENSE("GPL");
