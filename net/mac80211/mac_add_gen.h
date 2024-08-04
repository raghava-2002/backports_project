#ifndef MAC_ADDRESS_H
#define MAC_ADDRESS_H

#include <linux/skbuff.h>
#include <net/mac80211.h>
#include <linux/ieee80211.h>

//void generate_mac_add_sta(struct sk_buff *skb, struct sta_info *sta, long long int current_tp, unsigned char *r_mac);
void generate_mac_add_sta(struct sta_info *sta, long long int current_tp);
//void generate_mac_add_sta(struct ieee80211_local *local, long long int current_tp);
void generate_mac_add_ap(struct ieee80211_local *local, struct sk_buff *skb, struct sta_info *sta, long long int current_tp, unsigned char *r_mac);
void generate_mac_add_ap_all(struct ieee80211_local *local, long long int current_tp);

void test_func(struct ieee80211_local *local, struct sk_buff *skb, struct sta_info *sta, int flag_addr, long long int current_tp);
void generate_mac_add_sta_rx(struct sk_buff *skb, struct sta_info *sta, long long int current_tp, const u8 *interface_mac_addr);

#endif // MAC_ADDRESS_H
