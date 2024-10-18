#include <linux/kernel.h>
#include <linux/slab.h>

#include "rathan_tables/mac_translation_table.h"


#define TABLE_SIZE 100


struct mac_translation_entry *translation_table[TABLE_SIZE] = { NULL }; // Global translation hash table
EXPORT_SYMBOL(translation_table);

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
    memcpy(new_entry->base_mac, base_mac, ETH_ALEN); // Copy the base MAC address
    memcpy(new_entry->random_mac, random_mac, ETH_ALEN); // Copy the random MAC address
    memcpy(new_entry->old_random_mac, random_mac, ETH_ALEN); // it was first entry so old random mac is same as random mac
    new_entry->next = translation_table[index];
    translation_table[index] = new_entry;
}

void update_entry_by_base(const unsigned char *base_mac, const unsigned char *new_random_mac) {
    struct mac_translation_entry *entry = search_by_base_mac(base_mac);
    if (entry != NULL) {
        // Entry with the specified base MAC address found, update its random MAC address
        memcpy(entry->old_random_mac, entry->random_mac, ETH_ALEN); //store the old random mac address (from old time period)
        memcpy(entry->random_mac, new_random_mac, ETH_ALEN);
    } else {
        //printk(KERN_DEBUG "Rathan: MAT b Entry with base MAC address not found.\n");
        //if their is no entry with the base mac address then insert the new entry
        insert_entry(base_mac, new_random_mac);
        printk(KERN_DEBUG "Rathan: MAT b Entry with base MAC address not found. Inserted new entry.\n");
        //printk(KERN_DEBUG "Rathan: MAT b Entry with base MAC address not found. Inserted new entry.\n");
    }
}

//usually we donot use this function
void update_entry_by_random(const unsigned char *random_mac, const unsigned char *new_base_mac) {
    struct mac_translation_entry *entry = search_by_random_mac(random_mac);
    if (entry != NULL) {
        // Entry with the specified random MAC address found, update its base MAC address
        memcpy(entry->base_mac, new_base_mac, ETH_ALEN);
    } else {
        printk(KERN_DEBUG "Rathan: MAT r Entry with random MAC address not found.\n");
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

    //search for the old random mac address also just in case for race condition when the packet is received in the next time period from the previous time period
    for (index = 0; index < TABLE_SIZE; ++index) {
        struct mac_translation_entry *entry = translation_table[index];
        while (entry != NULL) {
            if (memcmp(entry->old_random_mac, random_mac, ETH_ALEN) == 0) {
                // Found the entry with the specified base MAC address
                printk(KERN_DEBUG "AP table: using old mac adddress\n");
                return entry;
            }
            entry = entry->next;
        }
    }
    // Entry with the specified base MAC address not found
    return NULL;
}
EXPORT_SYMBOL(search_by_random_mac);

// Function to search for a randomized MAC address with base MAC address returns the entry
struct mac_translation_entry *search_by_base_mac(const unsigned char *base_mac) {
    // Iterate over all entries in the translation table
	unsigned int index;
    for (index = 0; index < TABLE_SIZE; ++index) {
        struct mac_translation_entry *entry = translation_table[index];
        while (entry != NULL) {
            if (memcmp(entry->base_mac, base_mac, ETH_ALEN) == 0) {
                // Found the entry with the specified base MAC address
                //printk(KERN_DEBUG "Rathan: Found the entry with the specified base MAC address %pM\n", base_mac);
                return entry;
            }
            entry = entry->next;
        }
    }
    //printk(KERN_DEBUG "search by base MAC address not found.\n");
    // Entry with the specified base MAC address not found
    return NULL;
}

void print_mac_translation_table(void) {
    int i;
    printk(KERN_DEBUG "Rathan: Printing MAC translation table:\n");
    for (i = 0; i < TABLE_SIZE; ++i) {
        struct mac_translation_entry *entry = translation_table[i];
        while (entry != NULL) {
            printk(KERN_DEBUG "Rathan: Base MAC: %pM, Random MAC: %pM, old Random MAC: %pM\n", entry->base_mac, entry->random_mac, entry->old_random_mac);
            entry = entry->next;
        }
    }
}
EXPORT_SYMBOL(print_mac_translation_table);

// Function to delete an entry from the hash table
void delete_entry(const unsigned char *base_mac) {
    struct mac_translation_entry *prev_entry = NULL;
    unsigned int index;
    


    for (index = 0; index < TABLE_SIZE; ++index) {
        struct mac_translation_entry *cur = translation_table[index];
        prev_entry = NULL; // Reset prev_entry for each bucket
        while (cur != NULL) {
            if (memcmp(cur->base_mac, base_mac, ETH_ALEN) == 0) {
                // Found the entry, now delete it
                if (prev_entry == NULL) {
                    // Entry is the first node in the linked list
                    translation_table[index] = cur->next;
                } else {
                    // Entry is not the first node, update the previous node's next pointer
                    prev_entry->next = cur->next;
                }
                
                //printk(KERN_DEBUG "Rathan: MAT d Entry with base MAC %pM deleted.\n", base_mac);
                kfree(cur); // Free the memory allocated for the entry
                //print_mac_translation_table(); // Print updated table
                return;
            }
            prev_entry = cur;
            cur = cur->next;
        }
    }

    //check for the random mac address also in the table if yes delete it
    for (index = 0; index < TABLE_SIZE; ++index) {
        struct mac_translation_entry *cur = translation_table[index];
        prev_entry = NULL; // Reset prev_entry for each bucket
        while (cur != NULL) {
            if (memcmp(cur->random_mac, base_mac, ETH_ALEN) == 0) {
                // Found the entry, now delete it
                if (prev_entry == NULL) {
                    // Entry is the first node in the linked list
                    translation_table[index] = cur->next;
                } else {
                    // Entry is not the first node, update the previous node's next pointer
                    prev_entry->next = cur->next;
                }
                
                //printk(KERN_DEBUG "Rathan: MAT d Entry with base MAC %pM deleted.\n", base_mac);
                kfree(cur); // Free the memory allocated for the entry
                //print_mac_translation_table(); // Print updated table
                return;
            }
            prev_entry = cur;
            cur = cur->next;
        }
    }

    printk(KERN_DEBUG "MAT d Entry with base MAC %pM not found in the table during deletion.\n", base_mac);
    print_mac_translation_table(); // Print updated table
}


void cleanup_mac_translation_table(void) {
    int i;
    struct mac_translation_entry *entry, *tmp;
    printk(KERN_DEBUG "Cleaning up MAC translation table\n");
    for (i = 0; i < TABLE_SIZE; ++i) {
        entry = translation_table[i];
        while (entry) {
            tmp = entry;
            entry = entry->next;
            kfree(tmp);
        }
        translation_table[i] = NULL;
    }
}
EXPORT_SYMBOL(cleanup_mac_translation_table);