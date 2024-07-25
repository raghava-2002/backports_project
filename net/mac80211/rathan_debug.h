#ifndef RATHAN_DEBUG_H
#define RATHAN_DEBUG_H

enum rathan_instance_type {
    rathan_INSTANCE_UNKNOWN,
    rathan_INSTANCE_STA,
    rathan_INSTANCE_AP
};

#define RND_TP 5

void print_packet_header(struct sk_buff *skb);
enum rathan_instance_type which_instance (struct ieee80211_local *local);
bool is_sta_authorized(struct ieee80211_sub_if_data *sdata, const u8 *addr);
//struct sta_info *local_to_sta_info(struct ieee80211_local *local, const u8 *addr);
struct sta_info *local_to_sta_info(struct ieee80211_local *local);
struct ieee80211_sub_if_data *get_ap_sdata(struct ieee80211_local *local);
//void testing (struct ieee80211_local *local);
//void get_ap_sdata(struct ieee80211_local *local);

#endif 
