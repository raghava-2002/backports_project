#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "mac_pair_station.h"

//static struct mac_pair station_mac_pair = {(0), (0)}; // Define a single MAC pair, initially empty

struct mac_pair *station_mac_pair = NULL;
spinlock_t mac_pair_lock;


void set_mac_pair(const unsigned char *base_mac, const unsigned char *random_mac) {
    unsigned long flags;
    spin_lock_irqsave(&mac_pair_lock, flags);
    // Allocate memory for station_mac_pair if it is not already allocated
    if (!station_mac_pair) {
        station_mac_pair = kmalloc(sizeof(struct mac_pair), GFP_KERNEL);
        if (!station_mac_pair) {
            printk(KERN_ALERT "Rathan: Failed to allocate memory for MAC pair\n");
            return;
        }
        memset(station_mac_pair, 0, sizeof(struct mac_pair)); // Initialize to zero
    } 
     
    memcpy(station_mac_pair->s_base_mac, base_mac, ETH_ALEN);
    memcpy(station_mac_pair->current_random_mac, random_mac, ETH_ALEN);
    spin_unlock_irqrestore(&mac_pair_lock, flags);
    printk(KERN_DEBUG "Rathan: MAC pair set - Base MAC: %pM, Random MAC: %pM\n", station_mac_pair->s_base_mac, station_mac_pair->current_random_mac);
}

void get_mac_pair(struct mac_pair *pair) {
    unsigned long flags;

    spin_lock_irqsave(&mac_pair_lock, flags);

    if (station_mac_pair) {
        memcpy(pair->s_base_mac, station_mac_pair->s_base_mac, ETH_ALEN);
        memcpy(pair->current_random_mac, station_mac_pair->current_random_mac, ETH_ALEN);
    }

    spin_unlock_irqrestore(&mac_pair_lock, flags);
}



void update_current_random_mac(const unsigned char *base_mac, const unsigned char *new_random_mac) {
    unsigned long flags;
    spin_lock_irqsave(&mac_pair_lock, flags);
    if (station_mac_pair && memcmp(station_mac_pair->s_base_mac, base_mac, ETH_ALEN) == 0) {
        memcpy(station_mac_pair->current_random_mac, new_random_mac, ETH_ALEN);
    }else {
        printk(KERN_DEBUG "Rathan: pair update Entry with base MAC address not found.\n");
    }
    spin_unlock_irqrestore(&mac_pair_lock, flags);
}

void print_mac_pair(const struct mac_pair *pair) {
    if (pair) {
        printk(KERN_DEBUG "Rathan: MAC Pair - Base MAC: %pM, Random MAC: %pM\n", pair->s_base_mac, pair->current_random_mac);
    }
}