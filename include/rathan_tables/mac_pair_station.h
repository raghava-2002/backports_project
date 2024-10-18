#ifndef MAC_PAIR_STATION_H
#define MAC_PAIR_STATION_H

#include <linux/ieee80211.h>
#include_next <net/netlink.h>
//#define ETH_ALEN 6
#define NETLINK_USER 30  // Choose a unique protocol number for netlink communiocation should be less than 32

struct mac_pair {
    unsigned char s_base_mac[ETH_ALEN];
    unsigned char s_random_mac[ETH_ALEN];
    unsigned char s_old_random_mac[ETH_ALEN];
    struct mac_pair *next;
};

extern struct mac_pair *s_translation_table[]; // Declare the translation table as extern

extern struct sock *wmd_nl_sk; // Declare the netlink socket as extern


//defintions for the functions to use translation table 
void s_insert_entry(const unsigned char *base_mac, const unsigned char *random_mac);
void s_update_entry_by_base(const unsigned char *base_mac, const unsigned char *new_random_mac);
void s_update_entry_by_random(const unsigned char *random_mac, const unsigned char *new_base_mac);
struct mac_pair *s_search_by_random_mac(const unsigned char *random_mac);
struct mac_pair *s_search_by_base_mac(const unsigned char *base_mac);
void print_mac_pair_table(void);  // Declaration of the print function
void s_delete_entry(const unsigned char *random_mac);
//function to receive the random mac address from the userspace wmediumd and return the base mac address
void wmd_netlink_receive(struct sk_buff *skb);
void s_cleanup_translation_table(void);  // Declaration of the cleanup function

#endif // MAC_PAIR_H
