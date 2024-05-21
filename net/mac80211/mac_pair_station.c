#include <linux/kernel.h>
#include <linux/slab.h>

#include "mac_pair_station.h"

static struct mac_pair station_mac_pair = {{0}, {0}}; // Define a single MAC pair, initially empty

void set_mac_pair(const unsigned char *base_mac, const unsigned char *random_mac) {
    memcpy(station_mac_pair.s_base_mac, base_mac, ETH_ALEN);
    memcpy(station_mac_pair.current_random_mac, random_mac, ETH_ALEN);
}

const struct mac_pair* get_mac_pair(void) {
    return &station_mac_pair;
}

const struct mac_pair* search_by_s_base_mac(const unsigned char *base_mac) {
    if (memcmp(station_mac_pair.s_base_mac, base_mac, ETH_ALEN) == 0) {
        return &station_mac_pair;
    } else {
        printk(KERN_DEBUG "Rathan: pair b Entry with base MAC address not found.\n");
        return NULL;
    }
}

const struct mac_pair* search_by_current_random_mac(const unsigned char *random_mac) {
    if (memcmp(station_mac_pair.current_random_mac, random_mac, ETH_ALEN) == 0) {
        return &station_mac_pair;
    } else {
        printk(KERN_DEBUG "Rathan: pair r Entry with random MAC address not found.\n");
        return NULL;
    }
}

void update_current_random_mac(const unsigned char *base_mac, const unsigned char *new_random_mac) {
    if (memcmp(station_mac_pair.s_base_mac, base_mac, ETH_ALEN) == 0) {
        memcpy(station_mac_pair.current_random_mac, new_random_mac, ETH_ALEN);
    }else {
        printk(KERN_DEBUG "Rathan: pair update Entry with base MAC address not found.\n");
    }
}

void print_mac_pair(const struct mac_pair *pair) {
    printk(KERN_DEBUG "Rathan: MAC Pair - Base MAC: %pM, Random MAC: %pM\n", pair->s_base_mac, pair->current_random_mac);
}