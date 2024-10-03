#include <linux/kernel.h>
#include <linux/slab.h>


#include "rathan_tables/mac_pair_station.h"
#include "rathan_tables/mac_translation_table.h"

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

struct sock *wmd_nl_sk = NULL;  // Define the Netlink socket here
EXPORT_SYMBOL(wmd_nl_sk);       // Export the symbol

void wmd_netlink_receive(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    unsigned char *raw_data;
    unsigned char random_mac[ETH_ALEN];
    struct mac_pair *s_entry;
    struct mac_translation_entry *ap_entry; // Entry for station in the AP table

    int res, i;
    int data_len;

    //printk(KERN_DEBUG "Mac pair: Received Netlink message from user-space\n");

    // Step 1: Extract the Netlink message header
    nlh = nlmsg_hdr(skb);

    if (!nlh) {
        printk(KERN_ERR "Mac pair: Failed to extract Netlink header\n");
        return;
    }

    // Step 2: Get the raw data from the Netlink message payload
    raw_data = nlmsg_data(nlh);
    data_len = nlmsg_len(nlh);

    if (data_len < (8 + ETH_ALEN)) {
        printk(KERN_ERR "Mac pair: Invalid Netlink message length: %d\n", data_len);
        return;
    }

    // Log the raw data for debugging
    //printk(KERN_DEBUG "Mac pair: Raw data received from user-space:\n");
    /* for (i = 0; i < data_len; i++) {
        printk(KERN_DEBUG "%02x ", raw_data[i]);
    }
    printk(KERN_DEBUG "\n"); */

    // Step 3: Manually offset to extract the correct MAC address (offset by 8 bytes)
    memcpy(random_mac, raw_data + 8, ETH_ALEN);

    // Log the extracted random MAC
    //printk(KERN_DEBUG "Mac pair: Extracted random MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",random_mac[0], random_mac[1], random_mac[2],random_mac[3], random_mac[4], random_mac[5]);

    // Step 4: Search the MAC pair table for the associated base MAC
    s_entry = s_search_by_random_mac(random_mac);
    

    // Allocate a new Netlink message to send the response
    skb_out = nlmsg_new(ETH_ALEN, 0);
    if (!skb_out) {
        printk(KERN_ERR "Mac pair: Failed to allocate new skb\n");
        return;
    }

    // Prepare the Netlink message
    nlh = nlmsg_put(skb_out, 0, nlh->nlmsg_seq, NLMSG_DONE, ETH_ALEN, 0);
    //printk(KERN_DEBUG "Mac pair: Received message with seq_num: %u\n", nlh->nlmsg_seq);


    if (!nlh) {
        printk(KERN_ERR "Mac pair: Failed to create Netlink header for response\n");
        kfree_skb(skb_out);
        return;
    }

    NETLINK_CB(skb_out).dst_group = 0;  // Unicast message (i.e., send directly to wmediumd)

    if (s_entry) {
        // If a base MAC is found, copy the base MAC into the response
        //printk(KERN_DEBUG "Mac pair: Found base MAC for random MAC sending %pM\n", s_entry->s_base_mac);
        memcpy(nlmsg_data(nlh), s_entry->s_base_mac, ETH_ALEN);  // Copy the base MAC into the message
    } else {
        // If no base MAC is found in the MAC pair table,
        //search in the MAC translation table also to get the base MAC
        ap_entry = search_by_random_mac(random_mac);
        if (ap_entry) {
            //printk(KERN_DEBUG "AP mac pair: Found base MAC for random MAC in AP table sending %pM\n", ap_entry->base_mac);
            memcpy(nlmsg_data(nlh), ap_entry->base_mac, ETH_ALEN);  // Copy the base MAC into the message
        } else {
            //send the random MAC back as is
            //printk(KERN_DEBUG "Mac pair: No base MAC found, returning original random MAC\n");
            memcpy(nlmsg_data(nlh), random_mac, ETH_ALEN);  // Send the original random MAC back
        }
        
    }

    // Send the response back to wmediumd (user-space)
    res = nlmsg_unicast(wmd_nl_sk, skb_out, NETLINK_CB(skb).portid);
    if (res < 0) {
        printk(KERN_ERR "Mac pair: Error while sending response to user-space: %d\n", res);
        kfree_skb(skb_out);  // Free the skb if sending fails
    } else {
        //printk(KERN_DEBUG "Mac pair: Response sent successfully to user-space\n");
    }
}
EXPORT_SYMBOL(wmd_netlink_receive);

// Function to clean up the MAC pair table
void s_cleanup_translation_table(void) {
    int i;
    struct mac_pair *entry, *tmp;

    printk(KERN_DEBUG "Cleaning up MAC translation table\n");

    for (i = 0; i < TABLE_SIZE; ++i) {
        entry = s_translation_table[i];
        while (entry) {
            tmp = entry;
            entry = entry->next;
            kfree(tmp);
        }
        s_translation_table[i] = NULL;
    }
}
EXPORT_SYMBOL(s_cleanup_translation_table);