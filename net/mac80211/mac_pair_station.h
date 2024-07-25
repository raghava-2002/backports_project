#ifndef MAC_PAIR_STATION_H
#define MAC_PAIR_STATION_H

#include <linux/ieee80211.h>
//#define ETH_ALEN 6

struct mac_pair {
    unsigned char s_base_mac[ETH_ALEN];
    unsigned char s_random_mac[ETH_ALEN];
    struct mac_pair *next;
};




//defintions for the functions to use translation table 
void s_insert_entry(const unsigned char *base_mac, const unsigned char *random_mac);
void s_update_entry_by_base(const unsigned char *base_mac, const unsigned char *new_random_mac);
void s_update_entry_by_random(const unsigned char *random_mac, const unsigned char *new_base_mac);
struct mac_pair *s_search_by_random_mac(const unsigned char *random_mac);
struct mac_pair *s_search_by_base_mac(const unsigned char *base_mac);
void print_mac_pair_table(void);  // Declaration of the print function
void s_delete_entry(const unsigned char *random_mac);

#endif // MAC_PAIR_H
