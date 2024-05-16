#include <linux/kernel.h>
#include <linux/slab.h>

#include "mac_translation_table.h"

#define TABLE_SIZE 100

struct mac_translation_entry *translation_table[TABLE_SIZE] = { NULL }; // Global translation hash table


// Hash function that combines the bytes of a MAC address
unsigned int hash_function(const unsigned char *mac) {
    unsigned int hash = 0;
	int i;
    for (i = 0; i < ETH_ALEN; i++) {
        hash = (hash << 5) + mac[i]; // Rotate hash and add next byte
    }
    return hash % TABLE_SIZE; // Modulo to fit within table size
}


// Function to insert a new entry into the hash table
void insert_entry(const unsigned char *base_mac, const unsigned char *random_mac) {
    unsigned int index = hash_function(random_mac);
    struct mac_translation_entry *new_entry = kmalloc(sizeof(struct mac_translation_entry), GFP_KERNEL);
    if (new_entry == NULL) {
        // Handle memory allocation failure
        return;
    }
    memcpy(new_entry->base_mac, base_mac, ETH_ALEN);
    memcpy(new_entry->random_mac, random_mac, ETH_ALEN);
    new_entry->next = translation_table[index];
    translation_table[index] = new_entry;
}

void update_entry_by_base(const unsigned char *base_mac, const unsigned char *new_random_mac) {
    struct mac_translation_entry *entry = search_by_base_mac(base_mac);
    if (entry != NULL) {
        // Entry with the specified base MAC address found, update its random MAC address
        memcpy(entry->random_mac, new_random_mac, ETH_ALEN);
    } else {
        printk(KERN_DEBUG "Rathan: Entry with base MAC address not found.\n");
    }
}

void update_entry_by_random(const unsigned char *random_mac, const unsigned char *new_base_mac) {
    struct mac_translation_entry *entry = search_by_random_mac(random_mac);
    if (entry != NULL) {
        // Entry with the specified random MAC address found, update its base MAC address
        memcpy(entry->base_mac, new_base_mac, ETH_ALEN);
    } else {
        printk(KERN_DEBUG "Rathan: Entry with random MAC address not found.\n");
    }
}

// Function to perform a reverse search to find the base MAC address associated with a randomized MAC address returns entry
struct mac_translation_entry *search_by_random_mac(const unsigned char *random_mac) {
    // Iterate over all entries in the translation table
	unsigned int index;
    for (index = 0; index < TABLE_SIZE; ++index) {
        struct mac_translation_entry *entry = translation_table[index];
        while (entry != NULL) {
            if (memcmp(entry->random_mac, random_mac, ETH_ALEN) == 0) {
                // Found the entry with the specified base MAC address
                return entry;
            }
            entry = entry->next;
        }
    }

    // Entry with the specified base MAC address not found
    return NULL;
}

// Function to search for a randomized MAC address with base MAC address returns the entry
struct mac_translation_entry *search_by_base_mac(const unsigned char *base_mac) {
    // Iterate over all entries in the translation table
	unsigned int index;
    for (index = 0; index < TABLE_SIZE; ++index) {
        struct mac_translation_entry *entry = translation_table[index];
        while (entry != NULL) {
            if (memcmp(entry->base_mac, base_mac, ETH_ALEN) == 0) {
                // Found the entry with the specified base MAC address
                return entry;
            }
            entry = entry->next;
        }
    }

    // Entry with the specified base MAC address not found
    return NULL;
}
