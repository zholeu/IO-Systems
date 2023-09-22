#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h> 
#include <linux/in.h>
#include <net/arp.h>
#include <linux/ip.h>
#include <linux/udp.h>

static char* link = "enp0s3";
module_param(link, charp, 0);

static char* ifname = "lab3";

static struct net_device_stats stats;

static struct net_device *child = NULL;
struct priv {
    struct net_device *parent;
};

struct ip_addreses_pair {
    unsigned int saddr;
    unsigned int daddr;
};

#define BUFFER_SIZE 40
static struct ip_addreses_pair res_buffer[BUFFER_SIZE];
static size_t res_end = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0) 
  #define HAVE_PROC_OPS 
#endif 

static struct proc_dir_entry* our_proc_file;
#define PROCFS_NAME "var2"

size_t strlen(const char* str) {
  size_t i = 0;
  while (str[i] != '\0') i++;
  return i;
}

static size_t i = 0;

static ssize_t procfile_read(struct file *filePointer, char __user *buffer, 
                             size_t buffer_length, loff_t *offset) 
{ 
    pr_info("Procfile read\n");
    char mid_buffer[100];
    struct ip_addreses_pair* ip = &res_buffer[i];
    sprintf(mid_buffer, "Captured IP packet, saddr: %d.%d.%d.%d\ndaddr: %d.%d.%d.%d\n\0", 
        ntohl(ip->saddr) >> 24, (ntohl(ip->saddr) >> 16) & 0x00FF,
        (ntohl(ip->saddr) >> 8) & 0x0000FF, (ntohl(ip->saddr)) & 0x000000FF,
        ntohl(ip->daddr) >> 24, (ntohl(ip->daddr) >> 16) & 0x00FF,
        (ntohl(ip->daddr) >> 8) & 0x0000FF, (ntohl(ip->daddr)) & 0x000000FF
    );
    size_t mid_buf_len = strlen(mid_buffer);
    if (copy_to_user(buffer, mid_buffer, mid_buf_len)) {
        return 0;
    } else {
        *offset += mid_buf_len;
    }
    i = i + 1;
    if (res_end - i == 0) {
        i = 0;
        return 0;
    }
    pr_info("write to proc file %d pair from %d pairs", i, res_end);
    return mid_buf_len;
}

#ifdef HAVE_PROC_OPS 
static const struct proc_ops proc_file_fops = { 
    .proc_read = procfile_read, 
}; 
#else 
static const struct file_operations proc_file_fops = { 
    .read = procfile_read, 
}; 
#endif 

void copy_str(char* buffer, char* str, size_t offset, int size){
  size_t i;
  for(i = 0; i < size; i++){
    buffer[i + offset] = str[i];
  }
}

static char check_frame(struct sk_buff *skb, unsigned char data_shift) {
    if (skb->protocol == htons(ETH_P_IP)){
        struct iphdr *ip = (struct iphdr *)skb_network_header(skb);
        if(ip->version == 4){

            printk("Captured IP packet, saddr: %d.%d.%d.%d\n",
                    ntohl(ip->saddr) >> 24, (ntohl(ip->saddr) >> 16) & 0x00FF,
                    (ntohl(ip->saddr) >> 8) & 0x0000FF, (ntohl(ip->saddr)) & 0x000000FF);
            printk("daddr: %d.%d.%d.%d\n",
                    ntohl(ip->daddr) >> 24, (ntohl(ip->daddr) >> 16) & 0x00FF,
                    (ntohl(ip->daddr) >> 8) & 0x0000FF, (ntohl(ip->daddr)) & 0x000000FF);
            if (res_end == BUFFER_SIZE - 1) {
                res_end = 0;
            }
            res_buffer[res_end].saddr = ip->saddr;
            res_buffer[res_end].daddr = ip->daddr;
            res_end += 1;
            return 1;
        }
    }
    return 0;
}

static rx_handler_result_t handle_frame(struct sk_buff **pskb) {
    
        if (check_frame(*pskb, 0)) {
            stats.rx_packets++;
            stats.rx_bytes += (*pskb)->len;
        }
        (*pskb)->dev = child;
        return RX_HANDLER_ANOTHER;
} 

static int open(struct net_device *dev) {
    netif_start_queue(dev);
    printk(KERN_INFO "%s: device opened", dev->name);
    return 0; 
} 

static int stop(struct net_device *dev) {
    netif_stop_queue(dev);
    printk(KERN_INFO "%s: device closed", dev->name);
    return 0; 
} 

static netdev_tx_t start_xmit(struct sk_buff *skb, struct net_device *dev) {
    struct priv *priv = netdev_priv(dev);

    if (check_frame(skb, 14)) {
        stats.tx_packets++;
        stats.tx_bytes += skb->len;
    }

    if (priv->parent) {
        skb->dev = priv->parent;
        skb->priority = 1;
        dev_queue_xmit(skb);
        return 0;
    }
    return NETDEV_TX_OK;
}

static struct net_device_stats *get_stats(struct net_device *dev) {
    return &stats;
} 

static struct net_device_ops net_device_ops = {
    .ndo_open = open,
    .ndo_stop = stop,
    .ndo_get_stats = get_stats,
    .ndo_start_xmit = start_xmit
};

static void setup(struct net_device *dev) {
    int i;
    ether_setup(dev);
    memset(netdev_priv(dev), 0, sizeof(struct priv));
    dev->netdev_ops = &net_device_ops;

    //fill in the MAC address
    for (i = 0; i < ETH_ALEN; i++)
        dev->dev_addr[i] = (char)i;
} 

int __init vni_init(void) {
    int err = 0;
    struct priv *priv;
    child = alloc_netdev(sizeof(struct priv), ifname, NET_NAME_UNKNOWN, setup);
    if (child == NULL) {
        printk(KERN_ERR "%s: allocate error", THIS_MODULE->name);
        return -ENOMEM;
    }
    priv = netdev_priv(child);
    priv->parent = __dev_get_by_name(&init_net, link); //parent interface
    if (!priv->parent) {
        printk(KERN_ERR "%s: no such net: %s", THIS_MODULE->name, link);
        free_netdev(child);
        return -ENODEV;
    }
    if (priv->parent->type != ARPHRD_ETHER && priv->parent->type != ARPHRD_LOOPBACK) {
        printk(KERN_ERR "%s: illegal net type", THIS_MODULE->name); 
        free_netdev(child);
        return -EINVAL;
    }

    //copy IP, MAC and other information
    memcpy(child->dev_addr, priv->parent->dev_addr, ETH_ALEN);
    memcpy(child->broadcast, priv->parent->broadcast, ETH_ALEN);
    
    if ((err = dev_alloc_name(child, child->name))) {
        printk(KERN_ERR "%s: allocate name, error %i", THIS_MODULE->name, err);
        free_netdev(child);
        return -EIO;
    }

    register_netdev(child);
    rtnl_lock();
    netdev_rx_handler_register(priv->parent, &handle_frame, NULL);
    rtnl_unlock();

    // procfs init
    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops); 
    if (NULL == our_proc_file) { 
        proc_remove(our_proc_file); 
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME); 
        return -ENOMEM; 
    } 
 
    printk(KERN_INFO "Module %s loaded", THIS_MODULE->name);
    printk(KERN_INFO "%s: create link %s", THIS_MODULE->name, child->name);
    printk(KERN_INFO "%s: registered rx handler for %s", THIS_MODULE->name, priv->parent->name);
    pr_info("/proc/%s created\n", PROCFS_NAME); 
    return 0; 
}

void __exit vni_exit(void) {
    struct priv *priv = netdev_priv(child);
    if (priv->parent) {
        rtnl_lock();
        netdev_rx_handler_unregister(priv->parent);
        rtnl_unlock();
        printk(KERN_INFO "%s: unregister rx handler for %s", THIS_MODULE->name, priv->parent->name);
    }
    unregister_netdev(child);
    free_netdev(child);

    //procfs destory
    proc_remove(our_proc_file); 

    printk(KERN_INFO "Module %s unloaded", THIS_MODULE->name); 
} 

module_init(vni_init);
module_exit(vni_exit);

MODULE_AUTHOR("Author");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Description");
