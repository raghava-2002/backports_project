#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <net/mac80211.h>


#include "ieee80211_i.h"
#include "sta_info.h"


#include "rathan_debug.h"





void print_packet_header(struct sk_buff *skb) {
    
    struct ieee80211_hdr *hdr;
    u16 frame_control;
    u8 protocol_version, type, subtype;
    bool to_ds, from_ds, more_frag, retry, power_mgmt, more_data, protected_frame, order;
    __le16 seq_ctrl, fc; // Sequence control field
    u16 seq_num; // Sequence number

    if (skb == NULL) {
        printk(KERN_DEBUG "Rathan: Packet is NULL\n");
        return;
    }


    hdr = (struct ieee80211_hdr *)skb->data;
    fc = hdr->frame_control; // Frame control field
    frame_control = le16_to_cpu(hdr->frame_control); 
    seq_ctrl = hdr->seq_ctrl; // Sequence control field
    seq_num = le16_to_cpu(seq_ctrl) >> 4; // Sequence number

    
    
    

    // Parse frame control fields
    protocol_version = (frame_control & IEEE80211_FCTL_VERS) >> 0;
    type = (frame_control & IEEE80211_FCTL_FTYPE) >> 2;
    subtype = (frame_control & IEEE80211_FCTL_STYPE) >> 4;
    to_ds = !!(frame_control & IEEE80211_FCTL_TODS);
    from_ds = !!(frame_control & IEEE80211_FCTL_FROMDS);
    more_frag = !!(frame_control & IEEE80211_FCTL_MOREFRAGS);
    retry = !!(frame_control & IEEE80211_FCTL_RETRY);
    power_mgmt = !!(frame_control & IEEE80211_FCTL_PM);
    more_data = !!(frame_control & IEEE80211_FCTL_MOREDATA);
    protected_frame = !!(frame_control & IEEE80211_FCTL_PROTECTED);
    order = !!(frame_control & IEEE80211_FCTL_ORDER);




    // Print the complete header
    printk(KERN_DEBUG "IEEE 802.11 Frame Header:\n");
    printk(KERN_DEBUG "  Address 1: %pM\n", hdr->addr1);
    printk(KERN_DEBUG "  Address 2: %pM\n", hdr->addr2);
    printk(KERN_DEBUG "  Address 3: %pM\n", hdr->addr3);
    printk(KERN_DEBUG "  Address 4: %pM\n", hdr->addr4);
    //printk(KERN_DEBUG "  Seq ctrl: %d\n", seq_ctrl);
    printk(KERN_DEBUG "  Seq Num: %u\n", seq_num);
    //printk(KERN_DEBUG "  frame ctrl %d\n", fc);
    printk(KERN_DEBUG "  Frame Control: %u\n", frame_control);
    printk(KERN_DEBUG "  Frame control feilds:");
    /* printk(KERN_DEBUG "    Protocol Version: %d\n", protocol_version); */
    printk(KERN_DEBUG "    Type: %d\n", type);
    printk(KERN_DEBUG "    Subtype: %d\n", subtype);
    /* printk(KERN_DEBUG "    To DS: %d\n", to_ds);
    printk(KERN_DEBUG "    From DS: %d\n", from_ds);
    printk(KERN_DEBUG "    More Fragments: %d\n", more_frag);
    printk(KERN_DEBUG "    Retry: %d\n", retry);
    printk(KERN_DEBUG "    Power Management: %d\n", power_mgmt);
    printk(KERN_DEBUG "    More Data: %d\n", more_data);
    printk(KERN_DEBUG "    Protected Frame: %d\n", protected_frame);
    printk(KERN_DEBUG "    Order: %d\n", order); */
    printk(KERN_DEBUG "  Duration ID: %d\n", le16_to_cpu(hdr->duration_id));
    //printk(KERN_DEBUG "  Sequence Control: %d\n", le16_to_cpu(hdr->seq_ctrl));

    // Check if QoS Control is present
    if (ieee80211_is_data_qos(frame_control)) {
        struct ieee80211_qos_hdr *qos_hdr = (struct ieee80211_qos_hdr *)skb->data;
        printk(KERN_INFO "  QoS Control: %d\n", le16_to_cpu(qos_hdr->qos_ctrl));
    }

    // Check if HT Control is present
    if (ieee80211_has_order(frame_control)) {
        u32 *ht_control = (u32 *)(skb->data + sizeof(struct ieee80211_hdr));
        printk(KERN_INFO "  HT Control: %d\n", le32_to_cpu(*ht_control));
    }


    

}




enum rathan_instance_type which_instance(struct ieee80211_local *local) {
    
    struct ieee80211_hw *hw;
	struct wiphy *wiphy;
	struct wireless_dev *wdev;
    const char *ifname;
	enum nl80211_iftype iftype;
    bool sta = false, ap = false;

    if (local == NULL) {
        printk(KERN_DEBUG "cannot find instance local is null");
        return rathan_INSTANCE_UNKNOWN;
    }

    hw = &local->hw;
    wiphy = hw->wiphy;

    list_for_each_entry(wdev, &wiphy->wdev_list, list) {
        struct net_device *netdev = wdev->netdev;
        if (!netdev) continue;

        ifname = netdev->name;
        
        // Determine the interface type
        iftype = wdev->iftype;

        // Print or log the interface name and type
        //printk(KERN_DEBUG "Function is running on interface: %s\n", ifname);

        // Check and log the type of the interface
        switch (iftype) {
            case NL80211_IFTYPE_STATION:
                //printk(KERN_DEBUG "Instance : STATION\n");
				sta = true;
                break;
            case NL80211_IFTYPE_AP:
                //printk(KERN_DEBUG "Instance : AP\n");
				ap = true;
                break;
            // Add other cases if needed
            default:
                printk(KERN_DEBUG "Instance : OTHER\n");
                break;
        }
    }

    if (ap && sta) {
        printk(KERN_DEBUG "Both AP and STA interfaces are present\n");
        // Prioritize AP if both are present
        return rathan_INSTANCE_UNKNOWN;
    } else if (ap) {
        return rathan_INSTANCE_AP;
    } else if (sta) {
        return rathan_INSTANCE_STA;
    } else {
        return rathan_INSTANCE_UNKNOWN;
    }

}


//local to sta_info

/* protected by RCU */
/* struct sta_info *local_to_sta_info(struct ieee80211_local *local,
			      const u8 *addr)
{
	
	struct ieee80211_sta *pubsta;
    struct rhlist_head *tmp;
    struct sta_info *sta;

    rcu_read_lock();
    for_each_sta_info(local, addr, sta, tmp) {
        printk(KERN_DEBUG "Found sta_info for %pM\n", sta->sta.addr);
        if (ether_addr_equal(sta->sta.addr, addr)) {
            rcu_read_unlock();
            return sta;
        }
    }
    rcu_read_unlock();

    return NULL;  // Not found
} */


//this is overall function to get sta_info for both sta and AP (Ap doesnot have sta_info)
/* struct sta_info *local_to_sta_info(struct ieee80211_local *local)
{
    struct sta_info *sta;
    //const u8 *dest_mac_addr;
    
    
    list_for_each_entry_rcu(sta, &local->sta_list, list) {
        //dest_mac_addr = sta->sta.addr;
       //printk(KERN_DEBUG "Found sta_info for %pM\n", dest_mac_addr);
       //printk(KERN_DEBUG "started station %lld ", sta->start_time_period);
       if (sta){
            return sta;
       }
       
       printk(KERN_DEBUG "APA station %lld ", sta->sdata->start_time_period);
    }

    return NULL; //not fund sta
} */

/* void testing (struct ieee80211_local *local){
    struct sta_info *sta;
    //const u8 *dest_mac_addr;
    
    printk(KERN_DEBUG "APA hello ");
    list_for_each_entry_rcu(sta, &local->sta_list, list) {
        //dest_mac_addr = sta->sta.addr;
       //printk(KERN_DEBUG "Found sta_info for %pM\n", dest_mac_addr);
       //printk(KERN_DEBUG "started station %lld ", sta->start_time_period);
       
       printk(KERN_DEBUG "APA station %lld ", sta->start_time_period);
    }
    printk(KERN_DEBUG "APA hello done ");

} */





struct ieee80211_sub_if_data *get_ap_sdata(struct ieee80211_local *local) {
    struct ieee80211_sub_if_data *sdata;

    rcu_read_lock();
    list_for_each_entry_rcu(sdata, &local->interfaces, list) {
        if (sdata->vif.type == NL80211_IFTYPE_AP) {
            rcu_read_unlock();
            return sdata;
        }
    }
    rcu_read_unlock();
    return NULL; // AP interface not found
}

struct sta_info *local_to_sta_info(struct ieee80211_local *local) {
    struct sta_info *sta;
    
    list_for_each_entry_rcu(sta, &local->sta_list, list) {
       
       //printk(KERN_DEBUG "APA station %lld ", sta->start_time_period);
       return sta;
    }
    return NULL; //not found sta
}

/* void get_ap_sdata(struct ieee80211_local *local) {
    struct ieee80211_sub_if_data *sdata;

    rcu_read_lock();
    list_for_each_entry_rcu(sdata, &local->interfaces, list) {
        if (sdata->vif.type == NL80211_IFTYPE_AP) {
            rcu_read_unlock();
            //return sdata;
            printk(KERN_DEBUG "APA station %lld ", sdata->start_time_period);
        }
    }
    rcu_read_unlock();
    printk(KERN_DEBUG "APA hello done ");
    //return NULL; // AP interface not found
} */






//this is for checking if the station is authorized

bool is_sta_authorized(struct ieee80211_sub_if_data *sdata, const u8 *addr) {
    struct sta_info *sta;
    bool authorized = false;

    rcu_read_lock();
    sta = sta_info_get(sdata, addr);
    if (sta) {
        if (sta->sta_state == IEEE80211_STA_AUTHORIZED) {
            authorized = true;
        }
    }
    rcu_read_unlock();

    return authorized;
}

//useage of is_sta_authorized
/* if (!is_sta_authorized(sdata, dest_mac_addr)) {
        				printk(KERN_ERR "STA is not authorized: %pM\n", dest_mac_addr);
    				}  */



//here below is work with ccmp parameter i.e, PN/Nounce

void print_pn(u8 pn[6]) {
    printk(KERN_DEBUG "Current PN: %02x %02x %02x %02x %02x %02x\n",
           pn[0], pn[1], pn[2], pn[3], pn[4], pn[5]);
}

void print_current_pn(struct ieee80211_key *key) {
    int i;

    
    // Check if the key is a CCMP key and has rx_pn
    if (!key || !key->u.ccmp.rx_pn) {
        printk(KERN_DEBUG "Key is not a CCMP key or does not have rx_pn.\n");
        return;
    }

    rcu_read_lock();
    // Print the PN for each TID and the management frames
    for (i = 0; i < IEEE80211_NUM_TIDS + 1; i++) {
        printk(KERN_DEBUG "TID %d PN: ", i);
        print_pn(key->u.ccmp.rx_pn[i]);
    }
    rcu_read_unlock();
}

/* void print_sta_pn(struct ieee80211_local *local) {
    struct sta_info *sta;
    struct ieee80211_key *key;

    rcu_read_lock();

    list_for_each_entry_rcu(sta, &local->sta_list, list) {
        printk(KERN_DEBUG "Printing PNs for STA %pM\n", sta->sta.addr);

        list_for_each_entry_rcu(key, &sta->ptk_list, list) {
            print_current_pn(key);
        }
    }

    rcu_read_unlock();
} */


