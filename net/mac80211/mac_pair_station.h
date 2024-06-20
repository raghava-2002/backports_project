#ifndef MAC_PAIR_STATION_H
#define MAC_PAIR_STATION_H


#define ETH_ALEN 6

struct mac_pair {
    unsigned char s_base_mac[ETH_ALEN];
    unsigned char current_random_mac[ETH_ALEN];
};

extern struct mac_pair *station_mac_pair;


void set_mac_pair(const unsigned char *base_mac, const unsigned char *random_mac);
const struct mac_pair* get_mac_pair(void);
void update_current_random_mac(const unsigned char *base_mac, const unsigned char *new_random_mac);
void print_mac_pair(void);

#endif // MAC_PAIR_H
