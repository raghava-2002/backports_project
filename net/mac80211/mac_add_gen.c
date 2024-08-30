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
#include "rathan_tables/mac_translation_table.h"

#include "rathan_tables/mac_pair_station.h"

#include "mac_add_gen.h"

#include "rathan_debug.h"
#include "mac_randomizer.h"



bool ccmp_reset = true; // this is the flag to reset the ccmp counter (pn) for the station/AP used only in the mac80211 subsystem




//this function for kernal based time period random mac address generation
// function is for all stations associated with the AP to generate random mac address and update the table

void generate_mac_add_ap_all(struct ieee80211_local *local, long long int current_tp) {
    struct crypto_shash *shash;
    struct shash_desc *shash_desc;
    struct ieee80211_key *key, *g_key;
    unsigned char hash[20];  // Buffer for the hash
    char *data;
    int total_size = ETH_ALEN + 16 + sizeof(current_tp);
    bool gen; // Flag is used in case of no PTK for station but it is authorized (in between sending key to the packets to the station) 
    struct sta_info *sta;
    u64 rndpn64;

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
        //printk(KERN_DEBUG "Processing station base mac: %pM", dest_mac_addr);

        key = rcu_dereference(sta->ptk[sta->ptk_idx]);
        if (key) {
            memcpy(data, key->conf.key, key->conf.keylen);
            //printk(KERN_DEBUG "PTK for station: %*ph", key->conf.keylen, data);
            gen = true;
        } else {
            //printk(KERN_DEBUG "No PTK for station: %pM", dest_mac_addr);
            gen = false;
        }

        //this is worest case seniario where sta is not authorized but we are updating the table
        if (sta->sta_state != IEEE80211_STA_AUTHORIZED) {
            gen = false;
        }

        if (!gen) {
            memcpy(r_mac, dest_mac_addr, ETH_ALEN); // Use SA as the base MAC address
            //printk(KERN_DEBUG "Using base MAC for random mac : %pM", dest_mac_addr);
        } else {
            memcpy(data + 16, dest_mac_addr, ETH_ALEN); // Use SA as the base MAC address
            //printk(KERN_DEBUG "Using base MAC: %pM", dest_mac_addr);

            memcpy(data + ETH_ALEN + 16, &current_tp, sizeof(current_tp));
            //printk(KERN_DEBUG "Current timestamp: %lld", current_tp);

            // Compute the hash
            crypto_shash_digest(shash_desc, data, total_size, hash);

            // Adjust the first byte of the hash to make it a valid MAC address
            hash[0] = (hash[0] & 0xFC) | 0x02;  // Set bit-0 to 0 and bit-1 to 1

            // Copy the generated MAC address to r_mac
            memcpy(r_mac, hash, ETH_ALEN);

            printk(KERN_DEBUG "Ap: random MAC: %pM", r_mac);
        }

        //sequence reset
        sta->sdata->sequence_number = 0;
        //printk(KERN_DEBUG "new address generated ");

        //here we try to reset ccmp counter (pn) for the station (hard reset for time sake)
        //rndpn64 = 0x123456789ABCULL;
        if(ccmp_reset){
            //rndpn64 = 0x000000000123ULL;
            //atomic64_add(0x000001000000ULL, &key->conf.tx_pn);
            //atomic64_set(&key->conf.tx_pn, rndpn64);
            
            rndpn64 = generate_pn(current_tp);
            //printk(KERN_DEBUG "Test PN: %llu\n", rndpn64);
            //we are reseting the pn for the station only for the unicast packets
            atomic64_set(&key->conf.tx_pn, rndpn64);

            //we need to rest the counter for the multicast packets also
            g_key = rcu_dereference(sta->sdata->default_multicast_key);
            if(g_key){
                atomic64_set(&g_key->conf.tx_pn, rndpn64);
            }
            

        }
        

        // Update the MAT table with the generated MAC address (update if entry exists or insert new entry for the new station)
        update_entry_by_base(dest_mac_addr, r_mac);
    }

    rcu_read_unlock();
    // Clean up
    kfree(shash_desc);
    crypto_free_shash(shash);
    kfree(data);
}


//this is the new function which generates new random mac address for the station and updates the mac pair table

void generate_mac_add_sta(struct sta_info *sta, long long int current_tp) {
    struct crypto_shash *shash;
    struct shash_desc *shash_desc;
    struct ieee80211_key *key;
    //struct ieee80211_key *g_key;
    unsigned char hash[20];  // Buffer for the hash
    char *data;
    int total_size = ETH_ALEN + 16 + sizeof(current_tp);
    const u8 *interface_mac_addr;
    //struct ieee80211_tx_info *info;
    //struct ieee80211_sub_if_data *sdata;
    unsigned char r_mac[ETH_ALEN];  // Buffer for the generated random MAC address
    u64 rndpn64;



    //info = IEEE80211_SKB_CB(skb);
    //sdata = vif_to_sdata(info->control.vif);
    interface_mac_addr = sta->sdata->vif.addr;

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

    if (sta && (key = rcu_dereference(sta->ptk[sta->ptk_idx]))) {
        // Copy the PTK key to data, after the MAC address
        memcpy(data, key->conf.key, key->conf.keylen);
        //printk(KERN_DEBUG "sta in key: %*ph", key->conf.keylen, data);

        // Copy the MAC address to data
        memcpy(data + 16, interface_mac_addr, ETH_ALEN); // Use DA as the base MAC address
        //printk(KERN_DEBUG "sta in base mac: %pM", interface_mac_addr);

        // Copy current_tp adjacent to the PTK key
        memcpy(data + ETH_ALEN + key->conf.keylen, &current_tp, sizeof(current_tp));
        //printk(KERN_DEBUG "sta in current_tp %lld", current_tp);
    } else {
        printk(KERN_DEBUG "Rathan: key is null ");
    }
    rcu_read_unlock();

    // Compute the hash
    crypto_shash_digest(shash_desc, data, total_size, hash);

    // Adjust the first byte of the hash to make it a valid MAC address
    hash[0] = (hash[0] & 0xFC) | 0x02;  // Set bit-0 to 0 and bit-1 to 1

    // Copy the generated MAC address to r_mac
    memcpy(r_mac, hash, ETH_ALEN);
    printk(KERN_DEBUG "STA: sta rand mac %pM", r_mac);

    //sequence reset
    sta->sdata->sequence_number = 0;
    //print_current_pn(key);

    //here we try to reset ccmp counter (pn) for the station (hard reset for time sake)
    rndpn64 = 0x123456789ABCULL;
    
    if(ccmp_reset){
        //rndpn64 = 0x000000000123ULL;
        //atomic64_add(0x000001000000ULL, &key->conf.tx_pn);
 
        rndpn64 = generate_pn(current_tp);
        //printk(KERN_DEBUG "Test PN: %llu\n", rndpn64);
        atomic64_set(&key->conf.tx_pn, rndpn64);
        
        //atomic64_set(&key->conf.tx_pn, rndpn64);
        //we need to rest the counter for the multicast packets also
        /* g_key = rcu_dereference(sta->sdata->default_multicast_key);
        if(g_key){
            atomic64_set(&g_key->conf.tx_pn, rndpn64);
        }else{
            printk(KERN_DEBUG "STA: Multicast key is null\n");
        } */
    }
    



    // Update the MAC pair table (update if entry exists or insert new entry for the new station)
    s_update_entry_by_base(interface_mac_addr, r_mac); 

    // Clean up
    kfree(shash_desc);
    crypto_free_shash(shash);
    kfree(data);
}

//reseting the ccmp counter after every interval


static inline int calculate_l(int bitrate, int avg_frame_size) {
    // Bitrate * RND_TP / avg_frame_size is an integer division
    int result = bitrate * RND_TP / avg_frame_size;
    
    // Calculate log2 using bitwise operation
    int log_result = 0;
    while (result >>= 1) log_result++;
    
    // Add 1 to simulate the ceiling function
    return log_result + 1;
}

static inline int calculate_h(int l) {
    return 48 - l;
}


//return u64
u64 generate_pn(long long int seed) {
    u64 pn;
    u64 pn_h, pn_l;
    int l, h;
    int bitrate = 94000; // Example bitrate in kbps
    int avg_frame_size = 1500; // Example average frame size in bytes
    
    // Calculate l and h
    l = calculate_l(bitrate, avg_frame_size);
    h = calculate_h(l);
    //printk(KERN_DEBUG "h l: %d %d\n", h, l);
    h=24;
    l=24;
    // Calculate PN-H and PN-L
    pn_h = (seed) % (1ULL << h);
    pn_l = 0;  // Reset PN-L to 0
    
    // Combine PN-H and PN-L into a 48-bit PN
    pn = (pn_h << l) | pn_l;

    // Print debugging information
    /* printk(KERN_DEBUG "Bitrate: %d, Avg Frame Size: %d\n", bitrate, avg_frame_size);
    printk(KERN_DEBUG "Calculated l: %d\n", l);
    printk(KERN_DEBUG "Calculated h: %d\n", h);
    printk(KERN_DEBUG "PN-H: %llu\n", pn_h);
    printk(KERN_DEBUG "PN-L: %lluu\n", pn_l);
    printk(KERN_DEBUG "Generated PN: %016llx\n", pn); */

    return pn;
}

