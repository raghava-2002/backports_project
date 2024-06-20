#ifndef RATHAN_DEBUG_H
#define RATHAN_DEBUG_H

enum rathan_instance_type {
    rathan_INSTANCE_UNKNOWN,
    rathan_INSTANCE_STA,
    rathan_INSTANCE_AP
};


void print_packet_header(struct sk_buff *skb);
enum rathan_instance_type which_instance (struct ieee80211_local *local);

#endif 
