#ifndef MAC_PAIR_STATION_H
#define MAC_PAIR_STATION_H

#include <linux/spinlock.h>
#define ETH_ALEN 6

struct mac_pair {
    unsigned char s_base_mac[ETH_ALEN];
    unsigned char current_random_mac[ETH_ALEN];
};

extern struct mac_pair *station_mac_pair;
extern spinlock_t mac_pair_lock;

void set_mac_pair(const unsigned char *base_mac, const unsigned char *random_mac);
void get_mac_pair(struct mac_pair *pair);
void update_current_random_mac(const unsigned char *base_mac, const unsigned char *new_random_mac);
void print_mac_pair(const struct mac_pair *pair);

#endif // MAC_PAIR_H
