#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <net/mac80211.h>


#include "ieee80211_i.h"


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