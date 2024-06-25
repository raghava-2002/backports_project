#ifndef MAC_ADDRESS_H
#define MAC_ADDRESS_H

#include <linux/skbuff.h>
#include <net/mac80211.h>

void generate_mac_add_sta(struct sk_buff *skb, struct sta_info *sta, long long int current_tp, unsigned char *r_mac);
void generate_mac_add_ap(struct ieee80211_local *local, struct sk_buff *skb, struct sta_info *sta, long long int current_tp, unsigned char *r_mac);

void test_func(struct ieee80211_local *local, struct sk_buff *skb, struct sta_info *sta, int flag_addr, long long int current_tp);


#endif // MAC_ADDRESS_H
