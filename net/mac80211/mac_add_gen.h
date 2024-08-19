#ifndef MAC_ADDRESS_H
#define MAC_ADDRESS_H

#include <linux/skbuff.h>
#include <net/mac80211.h>
#include <linux/ieee80211.h>

extern bool send_custom_packet;

void generate_mac_add_sta(struct sta_info *sta, long long int current_tp);
void generate_mac_add_ap_all(struct ieee80211_local *local, long long int current_tp);

//ccmp reseting functions
u64 generate_pn(void);
//int calculate_l(int bitrate, int avg_frame_size);
//int calculate_h(int l);


#endif // MAC_ADDRESS_H
