#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/etherdevice.h>
#include <linux/bitmap.h>
#include <linux/rcupdate.h>
#include <linux/export.h>
#include <net/net_namespace.h>
#include <net/ieee80211_radiotap.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>
#include <net/codel.h>

#include <asm/unaligned.h>


#include "ieee80211_i.h"
#include "driver-ops.h"
#include "led.h"
#include "mesh.h"
#include "wep.h"
#include "wpa.h"
#include "wme.h"
#include "rate.h"
//here in AP instance we directly updating in the table but in sta instance we are not updating in the pair we will do that later 
#include "mac_translation_table.h"


#include "mac_add_gen.h"



#define ETH_ALEN 6
// i think i need to add more headers here then only it will work.


//the problem is the function is void it has to return the mac address
//change the return type to unsigned char* and return the r_mac



// modify this function generate mac based on the instance 
// here this function is for station instance so based on interface mac
/* void generate_mac_address(struct sk_buff *skb, struct sta_info *sta, int flag_addr, long long int current_tp, unsigned char *r_mac) {
    struct crypto_shash *shash;
    struct shash_desc *shash_desc;
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
    struct ieee80211_key *key;
    unsigned char hash[20];  // Buffer for the hash
    char *data;
    int total_size = ETH_ALEN + 16 + sizeof(current_tp);
    // Assuming total_size is the sum of the lengths of the MAC address, the PTK key, and current_tp

    data = kmalloc(total_size, GFP_KERNEL);
    if (!data) {
        printk(KERN_ERR "Failed to allocate data\n");
        return;
    }

    // Initialize the crypto hash
    shash = crypto_alloc_shash("sha1", 0, 0);
    if (IS_ERR(shash)) {
        printk(KERN_ERR "Failed to allocate crypto hash\n");
        kfree(data);
        return;
    }

    shash_desc = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(shash), GFP_KERNEL);
    if (!shash_desc) {
        printk(KERN_ERR "Failed to allocate shash_desc\n");
        crypto_free_shash(shash);
        kfree(data);
        return;
    }

    shash_desc->tfm = shash;

    if (sta && (key = rcu_dereference(sta->ptk[sta->ptk_idx]))) {
        printk(KERN_DEBUG "Rathan: ptk key %*ph", key->conf.keylen, key->conf.key);
        printk(KERN_DEBUG "Rathan: current time period %lld", current_tp);

        // Copy the MAC address to data
        if (flag_addr == 1) {
            memcpy(data, hdr->addr1, ETH_ALEN); // Use SA as the base MAC address
        } else {
            memcpy(data, hdr->addr2, ETH_ALEN); // Use DA as the base MAC address
        }

        printk(KERN_DEBUG "Rathan: %*ph", ETH_ALEN, data);
        // Copy the PTK key to data, after the MAC address
        memcpy(data + ETH_ALEN, key->conf.key, key->conf.keylen);

        // Copy current_tp adjacent to the PTK key
        memcpy(data + ETH_ALEN + key->conf.keylen, &current_tp, sizeof(current_tp));
    } else {
        printk(KERN_DEBUG "Rathan: key is null ");
    }

    // Compute the hash
    crypto_shash_digest(shash_desc, data, total_size, hash);

    // Adjust the first byte of the hash to make it a valid MAC address
    hash[0] = (hash[0] & 0xFC) | 0x02;  // Set bit-0 to 0 and bit-1 to 1

    // Copy the generated MAC address to r_mac
    memcpy(r_mac, hash, ETH_ALEN);

    // Clean up
    kfree(shash_desc);
    crypto_free_shash(shash);
    kfree(data);
} */




void generate_mac_add_sta(struct sk_buff *skb, struct sta_info *sta, long long int current_tp, unsigned char *r_mac) {
    struct crypto_shash *shash;
    struct shash_desc *shash_desc;
    //struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
    struct ieee80211_key *key;
    unsigned char hash[20];  // Buffer for the hash
    char *data;
    int total_size = ETH_ALEN + 16 + sizeof(current_tp);
    const u8 *interface_mac_addr;
    struct ieee80211_tx_info *info;
	struct ieee80211_sub_if_data *sdata;


    info = IEEE80211_SKB_CB(skb);
	sdata = vif_to_sdata(info->control.vif);
    interface_mac_addr = sdata->vif.addr;

    data = kmalloc(total_size, GFP_KERNEL);
    if (!data) {
        printk(KERN_ERR "Failed to allocate data\n");
        return;
    }

    // Initialize the crypto hash
    shash = crypto_alloc_shash("sha1", 0, 0);
    if (IS_ERR(shash)) {
        printk(KERN_ERR "Failed to allocate crypto hash\n");
        kfree(data);
        return;
    }

    shash_desc = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(shash), GFP_KERNEL);
    if (!shash_desc) {
        printk(KERN_ERR "Failed to allocate shash_desc\n");
        crypto_free_shash(shash);
        kfree(data);
        return;
    }

    shash_desc->tfm = shash;

    if (sta && (key = rcu_dereference(sta->ptk[sta->ptk_idx]))) {
        //printk(KERN_DEBUG "Rathan: ptk key %*ph", key->conf.keylen, key->conf.key);
        //printk(KERN_DEBUG "Rathan: current time period %lld", current_tp);

        //printk(KERN_DEBUG "Rathan: %*ph", ETH_ALEN, data);
        // Copy the PTK key to data, after the MAC address
        memcpy(data , key->conf.key, key->conf.keylen);
        printk(KERN_DEBUG "sta in key: %*ph", key->conf.keylen, data);

        // Copy the MAC address to data
        
        memcpy(data + 16, interface_mac_addr, ETH_ALEN); // Use DA as the base MAC address
        printk(KERN_DEBUG "sta in base mac: %pM", interface_mac_addr);
        

        

        // Copy current_tp adjacent to the PTK key
        memcpy(data + ETH_ALEN + key->conf.keylen, &current_tp, sizeof(current_tp));
        printk(KERN_DEBUG "sta in current_tp %lld", current_tp);
    } else {
        printk(KERN_DEBUG "Rathan: key is null ");
    }

    // Compute the hash
    crypto_shash_digest(shash_desc, data, total_size, hash);

    // Adjust the first byte of the hash to make it a valid MAC address
    hash[0] = (hash[0] & 0xFC) | 0x02;  // Set bit-0 to 0 and bit-1 to 1

    // Copy the generated MAC address to r_mac
    memcpy(r_mac, hash, ETH_ALEN);
    printk(KERN_DEBUG "Rathan: sta rand mac %pM", r_mac);

    // Clean up
    kfree(shash_desc);
    crypto_free_shash(shash);
    kfree(data);
}



// need to write a code to generate mac address based on the instance
// here this function is for Ap instance so based select the base mac address based on dest_mac_addr
// this function is only generate mac address for only single station


void generate_mac_add_ap(struct ieee80211_local *local, struct sk_buff *skb, struct sta_info *sta, long long int current_tp, unsigned char *r_mac) {
    struct crypto_shash *shash;
    struct shash_desc *shash_desc;
    //struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
    struct ieee80211_key *key;
    unsigned char hash[20];  // Buffer for the hash
    char *data;
    int total_size = ETH_ALEN + 16 + sizeof(current_tp);
    const u8 *dest_mac_addr;
    bool gen; // Flag is used in case of no ptk for station but it is authorized (in between sending key to the packets to the station) 

    data = kmalloc(total_size, GFP_KERNEL);
    if (!data) {
        printk(KERN_ERR "Failed to allocate data\n");
        return;
    }

    // Initialize the crypto hash
    shash = crypto_alloc_shash("sha1", 0, 0);
    if (IS_ERR(shash)) {
        printk(KERN_ERR "Failed to allocate crypto hash\n");
        kfree(data);
        return;
    }

    shash_desc = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(shash), GFP_KERNEL);
    if (!shash_desc) {
        printk(KERN_ERR "Failed to allocate shash_desc\n");
        crypto_free_shash(shash);
        kfree(data);
        return;
    }

    shash_desc->tfm = shash;

    if(sta){
        dest_mac_addr = sta->sta.addr;
        rcu_read_lock();

        list_for_each_entry(sta, &local->sta_list, list){
            if (memcmp(sta->sta.addr, dest_mac_addr, ETH_ALEN) == 0) {
                // Station found, retrieve the PTK
                printk(KERN_DEBUG "Station found ");
                key = rcu_dereference(sta->ptk[sta->ptk_idx]);
                if (key) {
                    //printk(KERN_DEBUG "inter %pM dest %pM\n", interface_mac_addr, dest_mac_addr);
                    
                    //printk(KERN_DEBUG "PTK for station %*ph\n", key->conf.keylen, key->conf.key);
                    memcpy(data , key->conf.key, key->conf.keylen);
                    printk(KERN_DEBUG "Ap in key: %*ph", key->conf.keylen, data);
                    gen = true;
                } else {
                printk(KERN_DEBUG "No PTK for station ");
                gen = false;
                }
                break;
            }else{
                printk(KERN_DEBUG "Station not found ");
            }
        }
       rcu_read_unlock();
           // Copy the MAC address to data
        
        
    } 

    if (gen == false) {
        printk(KERN_DEBUG "No PTK for station ");
        
        memcpy(r_mac , dest_mac_addr, ETH_ALEN); // Use SA as the base MAC address
        printk(KERN_DEBUG "Ap in base mac: %pM", dest_mac_addr);
        
        
    }

    if(gen == true){


        
        memcpy(data + 16 , dest_mac_addr, ETH_ALEN); // Use SA as the base MAC address
        printk(KERN_DEBUG "Ap in base mac: %pM", dest_mac_addr);
        

            //printk(KERN_DEBUG "Rathan: %*ph", ETH_ALEN, data);
            // Copy the PTK key to data, after the MAC address
        // memcpy(data + ETH_ALEN, key->conf.key, key->conf.keylen);

            // Copy current_tp adjacent to the PTK key
        memcpy(data + ETH_ALEN + 16, &current_tp, sizeof(current_tp));
        printk(KERN_DEBUG "Ap in current_tp %lld", current_tp);

        // Compute the hash
        crypto_shash_digest(shash_desc, data, total_size, hash);

        // Adjust the first byte of the hash to make it a valid MAC address
        hash[0] = (hash[0] & 0xFC) | 0x02;  // Set bit-0 to 0 and bit-1 to 1

        // Copy the generated MAC address to r_mac
        memcpy(r_mac, hash, ETH_ALEN);

        printk(KERN_DEBUG "Rathan:ap rand mac %pM", r_mac);

    }

    // Clean up
    kfree(shash_desc);
    crypto_free_shash(shash);
    kfree(data);
}



void test_func(struct ieee80211_local *local, struct sk_buff *skb, struct sta_info *sta, int flag_addr, long long int current_tp)
{

    if(local == NULL){
        printk(KERN_DEBUG "local is null");
    }else{
        printk(KERN_DEBUG "local is not null");
    }
}



// this is similar to the above function but this function is for all stations associated with the AP

void generate_mac_add_ap_all(struct ieee80211_local *local, long long int current_tp) {
    struct crypto_shash *shash;
    struct shash_desc *shash_desc;
    struct ieee80211_key *key;
    unsigned char hash[20];  // Buffer for the hash
    char *data;
    int total_size = ETH_ALEN + 16 + sizeof(current_tp);
    bool gen; // Flag is used in case of no PTK for station but it is authorized (in between sending key to the packets to the station) 
    struct sta_info *sta;

    data = kmalloc(total_size, GFP_KERNEL);
    if (!data) {
        printk(KERN_ERR "Failed to allocate data\n");
        return;
    }

    // Initialize the crypto hash
    shash = crypto_alloc_shash("sha1", 0, 0);
    if (IS_ERR(shash)) {
        printk(KERN_ERR "Failed to allocate crypto hash\n");
        kfree(data);
        return;
    }

    shash_desc = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(shash), GFP_KERNEL);
    if (!shash_desc) {
        printk(KERN_ERR "Failed to allocate shash_desc\n");
        crypto_free_shash(shash);
        kfree(data);
        return;
    }

    shash_desc->tfm = shash;

    rcu_read_lock();

    list_for_each_entry_rcu(sta, &local->sta_list, list) {
        const u8 *dest_mac_addr = sta->sta.addr;
        unsigned char r_mac[ETH_ALEN];
        printk(KERN_DEBUG "Processing station base mac: %pM", dest_mac_addr);

        key = rcu_dereference(sta->ptk[sta->ptk_idx]);
        if (key) {
            memcpy(data, key->conf.key, key->conf.keylen);
            printk(KERN_DEBUG "PTK for station: %*ph", key->conf.keylen, data);
            gen = true;
        } else {
            printk(KERN_DEBUG "No PTK for station: %pM", dest_mac_addr);
            gen = false;
        }

        if (!gen) {
            memcpy(r_mac, dest_mac_addr, ETH_ALEN); // Use SA as the base MAC address
            printk(KERN_DEBUG "Using base MAC for random mac : %pM", dest_mac_addr);
        } else {
            memcpy(data + 16, dest_mac_addr, ETH_ALEN); // Use SA as the base MAC address
            //printk(KERN_DEBUG "Using base MAC: %pM", dest_mac_addr);

            memcpy(data + ETH_ALEN + 16, &current_tp, sizeof(current_tp));
            printk(KERN_DEBUG "Current timestamp: %lld", current_tp);

            // Compute the hash
            crypto_shash_digest(shash_desc, data, total_size, hash);

            // Adjust the first byte of the hash to make it a valid MAC address
            hash[0] = (hash[0] & 0xFC) | 0x02;  // Set bit-0 to 0 and bit-1 to 1

            // Copy the generated MAC address to r_mac
            memcpy(r_mac, hash, ETH_ALEN);

            printk(KERN_DEBUG "Generated random MAC: %pM", r_mac);
        }

        // Update the MAT table with the generated MAC address
        update_entry_by_base(dest_mac_addr, r_mac);
    }

    rcu_read_unlock();

    // Clean up
    kfree(shash_desc);
    crypto_free_shash(shash);
    kfree(data);
}
