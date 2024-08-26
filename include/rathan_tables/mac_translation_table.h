#ifndef MAC_TRANSLATION_TABLE_H
#define MAC_TRANSLATION_TABLE_H

#include <linux/ieee80211.h>
//#define ETH_ALEN 6

struct mac_translation_entry {
    unsigned char base_mac[ETH_ALEN];
    unsigned char random_mac[ETH_ALEN];
    struct mac_translation_entry *next;
};

extern struct mac_translation_entry *translation_table[]; // Declare the translation table as extern


//defintions for the functions to use translation table 
void insert_entry(const unsigned char *base_mac, const unsigned char *random_mac);
void update_entry_by_base(const unsigned char *base_mac, const unsigned char *new_random_mac);
void update_entry_by_random(const unsigned char *random_mac, const unsigned char *new_base_mac);
struct mac_translation_entry *search_by_random_mac(const unsigned char *random_mac);
struct mac_translation_entry *search_by_base_mac(const unsigned char *base_mac);
void print_mac_translation_table(void);  // Declaration of the print function
void delete_entry(const unsigned char *random_mac);


#endif /* MAC_TRANSLATION_TABLE_H */