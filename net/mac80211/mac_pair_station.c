#include <linux/kernel.h>
#include <linux/slab.h>


#include "rathan_tables/mac_pair_station.h"

#define TABLE_SIZE 10
bool it_is_sta = false;
EXPORT_SYMBOL(it_is_sta);

struct mac_pair *s_translation_table[TABLE_SIZE] = { NULL }; // Global translation hash table
EXPORT_SYMBOL(s_translation_table);

// Hash function that combines the bytes of a MAC address
unsigned int s_hash_function(const unsigned char *mac) {
    unsigned int hash = 0;
	int i;
    for (i = 0; i < ETH_ALEN; i++) {
        hash = (hash << 5) + mac[i]; // Rotate hash and add next byte
    }
    return hash % TABLE_SIZE; // Modulo to fit within table size
}


// Function to insert a new entry into the hash table
void s_insert_entry(const unsigned char *base_mac, const unsigned char *random_mac) {
    unsigned int index = s_hash_function(random_mac);
    struct mac_pair *new_entry = kmalloc(sizeof(struct mac_pair), GFP_KERNEL);
    if (new_entry == NULL) {
        // Handle memory allocation failure
        printk(KERN_DEBUG "Mac pair: Memory allocation failed.\n");
        return;
    }
    memcpy(new_entry->s_base_mac, base_mac, ETH_ALEN);
    memcpy(new_entry->s_random_mac, random_mac, ETH_ALEN);
    //printk(KERN_DEBUG "Mac pair: Inserting entry with base MAC: %pM, random MAC: %pM\n", new_entry->s_base_mac, new_entry->s_random_mac);
    new_entry->next = s_translation_table[index];
    s_translation_table[index] = new_entry;
}

void s_update_entry_by_base(const unsigned char *base_mac, const unsigned char *new_random_mac) {
    struct mac_pair *entry = s_search_by_base_mac(base_mac);
    if (entry != NULL) {
        // Entry with the specified base MAC address found, update its random MAC address
        memcpy(entry->s_random_mac, new_random_mac, ETH_ALEN);
    } else {
        printk(KERN_DEBUG "Mac pair: Entry with base MAC address not found.\n");
    }
}

void s_update_entry_by_random(const unsigned char *random_mac, const unsigned char *new_base_mac) {
    struct mac_pair *entry = s_search_by_random_mac(random_mac);
    if (entry != NULL) {
        // Entry with the specified random MAC address found, update its base MAC address
        memcpy(entry->s_base_mac, new_base_mac, ETH_ALEN);
    } else {
        printk(KERN_DEBUG "Mac pair: Entry with random MAC address not found.\n");
    }
}

// Function to perform a reverse search to find the base MAC address associated with a randomized MAC address returns entry
struct mac_pair *s_search_by_random_mac(const unsigned char *random_mac) {
    // Iterate over all entries in the translation table
	unsigned int index;
    for (index = 0; index < TABLE_SIZE; ++index) {
        struct mac_pair *entry = s_translation_table[index];
        while (entry != NULL) {
            if (memcmp(entry->s_random_mac, random_mac, ETH_ALEN) == 0) {
                // Found the entry with the specified base MAC address
                return entry;
            }
            entry = entry->next;
        }
    }

    // Entry with the specified base MAC address not found
    return NULL;
}
EXPORT_SYMBOL(s_search_by_random_mac);

// Function to search for a randomized MAC address with base MAC address returns the entry
struct mac_pair *s_search_by_base_mac(const unsigned char *base_mac) {
    // Iterate over all entries in the translation table
	unsigned int index;
    //printk(KERN_DEBUG "Mac pair: Searching for base MAC address\n");
    for (index = 0; index < TABLE_SIZE; ++index) {
        struct mac_pair *entry = s_translation_table[index];
        while (entry != NULL) {
            if (memcmp(entry->s_base_mac, base_mac, ETH_ALEN) == 0) {
                // Found the entry with the specified base MAC address
                return entry;
            }
            entry = entry->next;
        }
    }

    // Entry with the specified base MAC address not found
    return NULL;
}

void print_mac_pair_table(void) {
    int i;
    printk(KERN_DEBUG "Mac pair: MAC pair table:\n");
    for (i = 0; i < TABLE_SIZE; ++i) {
        struct mac_pair *entry = s_translation_table[i];
        while (entry != NULL) {
            printk(KERN_DEBUG "Base MAC: %pM, Random MAC: %pM\n", entry->s_base_mac, entry->s_random_mac);
            entry = entry->next;
        }
    }
}
EXPORT_SYMBOL(print_mac_pair_table);

// Function to delete an entry from the hash table
void s_delete_entry(const unsigned char *random_mac) {
    unsigned int index = s_hash_function(random_mac);
    struct mac_pair *prev_entry = NULL;
    struct mac_pair *entry = s_translation_table[index];
    
    // Traverse the linked list at the index
    while (entry != NULL) {
        if (memcmp(entry->s_random_mac, random_mac, ETH_ALEN) == 0) {
            // Found the entry with the specified random MAC address
            if (prev_entry == NULL) {
                // Entry is the first node in the linked list
                s_translation_table[index] = entry->next;
            } else {
                // Entry is not the first node, update the previous node's next pointer
                prev_entry->next = entry->next;
            }
            
            // Free the memory allocated for the entry
            kfree(entry);
            
            return;
        }
        
        prev_entry = entry;
        entry = entry->next;
    }
    
    printk(KERN_DEBUG "Mac pair: Entry with random MAC address not found, nothing to delete entry.\n");
}