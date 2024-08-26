// mac_randomizer.h

#ifndef MAC_RANDOMIZER_H
#define MAC_RANDOMIZER_H

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

#include "mesh.h"
#include "wep.h"
#include "wpa.h"
#include "wme.h"
#include "rate.h"

//rathan header files
#include "rathan_tables/mac_translation_table.h"
#include "rathan_tables/mac_pair_station.h"
#include "rathan_debug.h"
#include "mac_add_gen.h"

extern bool RND_MAC;    //  random MAC address generation logic bool edit this in .c file to enable or disable

extern bool RND_KERN;   //  random MAC address generation logic bool to run mac address change by the kernel time period
extern bool RND_AP;     //  random MAC address generation logic bool by AP intiated trigger


struct custom_packet_payload {
    u8 mac_validity_period; // Time period for MAC validity in seconds
    u32 mac_generation_seed; // Seed for MAC address generation
    char message[64]; // Optional text message or additional data
}; // Custom packet payload data

// Time period for random MAC address generation (in seconds)
//Incase of kernel time period based random mac address generation this is used every where hardcoded in the kernal
//Incase of AP intiated trigger based random mac address generation this is used only in the AP case
#define RND_TP 15

void handle_random_mac(struct ieee80211_tx_data *tx);
void mac_addr_change_hdr_tx(struct sk_buff_head *skbs, struct ieee80211_vif *vif);
void mac_addr_change_hdr_rx (struct ieee80211_local *local, struct ieee80211_hdr *hdr);
struct sk_buff *construct_custom_packet(struct ieee80211_vif *vif);

#endif // MAC_RANDOMIZER_H