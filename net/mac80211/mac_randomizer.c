// mac_randomizer.c

//written by Rathan Appana

#include <linux/random.h>
#include <linux/etherdevice.h>
#include "mac_randomizer.h"


//bool RND_MAC = false;

bool RND_MAC = true;    // Enable random MAC address generation logic 



bool debug = true;

//this function checks time period and generates random mac address for the station and update the table reset the sequence number 

void handle_random_mac(struct ieee80211_tx_data *tx) {

    //struct sk_buff *skb;
    //struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
    struct sta_info *sta;
    struct ieee80211_local *local;
    struct ieee80211_sub_if_data *sdata;
    struct mac_translation_entry *entry;
	struct mac_pair *s_entry;

    bool gen_flag, trigger_flag;
	const u8 *interface_mac_addr;
	const u8 *dest_mac_addr;
    long long int current_tp;

    //introducing some time delay for the stations only


    trigger_flag = true;
    //skb = tx->skb;
    //sta = tx->sta;
    //local = tx->local;
    //sdata = tx->sdata;



   

    if (tx->sta != NULL) {
        sta = tx->sta;
        /* if (sta->sdata->vif.type){
            printk(KERN_DEBUG "sdata is not null");
        }else{
            printk(KERN_DEBUG "sdata is null");
        } */
        switch (sta->sta_state) {
            case IEEE80211_STA_AUTHORIZED:
                //printk(KERN_DEBUG "sta state is authorized");
                break;
            case IEEE80211_STA_ASSOC:
                //printk(KERN_DEBUG "sta state is associated");
                break;
            case IEEE80211_STA_AUTH:
                //printk(KERN_DEBUG "sta state is auth");
                break;
            case IEEE80211_STA_NONE:
                //printk(KERN_DEBUG "sta state is none");
                break;
            default:
                //printk(KERN_DEBUG "sta state is unknown");
                break;
        }
    } else {
        //printk(KERN_DEBUG "tx: sta is NULL\n");
        trigger_flag = false;
    }
    if (tx->local != NULL) {
        local = tx->local;
    } else {
        //printk(KERN_DEBUG "Tx: local is NULL\n");
        trigger_flag = false;
    }
    /* if (tx->skb != NULL) {
        skb = tx->skb;
    } else {
        printk(KERN_DEBUG "Tx: skb is NULL\n");
       
       
        trigger_flag = false;
    } */
    if (tx->sdata != NULL) {
        sdata = tx->sdata;
        //printk(KERN_DEBUG "sdata is not null");
    } else {
        //printk(KERN_DEBUG "Tx: sdata is NULL\n");
        trigger_flag = false;
    }

    if(trigger_flag) {
        //printk(KERN_DEBUG "trigger flag is True\n");
    }

    //printk(KERN_DEBUG "debug 2");


    if (trigger_flag && (sta->sta_state == IEEE80211_STA_AUTHORIZED)) {
        // Generate random MAC address
        // Update table
        // Reset sequence number and CCMP parameter
        // we have to generate the mac address based on the instance
        
        if (sta != NULL) {
            //if (!ieee80211_is_mgmt(fc)) {
                
                
                dest_mac_addr = sta->sta.addr;
                
                //bool same_interval = (current_tp == interval_tp);

                switch (sta->sdata->vif.type) {
                    case NL80211_IFTYPE_AP:
                        //printk(KERN_DEBUG "Tx: sta type AP\n");
                        entry = search_by_base_mac(dest_mac_addr);
                        //printk(KERN_DEBUG "dest mac %pM\n", dest_mac_addr);
                        current_tp = (ktime_get_real_seconds()/RND_TP);

                        if (entry != NULL) {
                            //printk(KERN_DEBUG "Ap case1 tx.c");
                            gen_flag = true;
                        } else {
                            //printk(KERN_DEBUG "Ap case2 tx.c");
                            insert_entry(dest_mac_addr, dest_mac_addr);
                            gen_flag = false;
                            sta->sdata->start_time_period = current_tp;
                        }
                        /* if (sdata->sequence_number) {
                            printk(KERN_DEBUG "Ap TX: %u ",sdata->sequence_number);
                        } */

                        //&& (is_sta_authorized(sdata, dest_mac_addr))
                        if (((current_tp) != (sta->sdata->start_time_period)) && gen_flag) {
                            //printk(KERN_DEBUG "Ap case3 tx.c");
                            //it generates random mac for all stations and update them in the table directly
                            generate_mac_add_ap_all(local, current_tp);
                            //printk(KERN_DEBUG "Rathan: before curr %lld inter %lld", current_tp, sta->sdata->start_time_period);
                            sta->sdata->start_time_period = current_tp;
                            
                        }

                        //printk(KERN_DEBUG "Rathan: curr %lld inter %lld", current_tp, sta->sdata->start_time_period);
                        //print_mac_translation_table();
                        
                        break;

                    case NL80211_IFTYPE_STATION:
                        //printk(KERN_DEBUG "Tx: sta type STATION\n");
                        //printk(KERN_DEBUG "Rathan: dest mac %pM\n", dest_mac_addr);
                        
                        //intoduce manual time delay for the stations
                        current_tp = (ktime_get_real_seconds()/RND_TP);
                        //print the sta state also below by sta->sta_state
                        //sta_state11 = sta->sta_state;
                        interface_mac_addr = sta->sdata->vif.addr;
                        
                        //mac pair structure is completely changed, it is similar to mac translation table
                        //we have to change it 
                        s_entry = s_search_by_base_mac(interface_mac_addr);
                        //printk(KERN_DEBUG "interface mac %pM\n", interface_mac_addr);
                        
                        if (s_entry != NULL) {
                            //printk(KERN_DEBUG "STATION case1 tx.c");
                            gen_flag = true;
                        } else {
                            //printk(KERN_DEBUG "STATION case2 tx.c");
                            s_insert_entry(interface_mac_addr, interface_mac_addr);
                            
                            gen_flag = false;
                            sta->start_time_period = current_tp;
                        }
                        
                        //sta->sta_state == IEEE80211_STA_AUTHORIZED
                        if ((current_tp != (sta->start_time_period)) && gen_flag) {
                            //printk(KERN_DEBUG "Station case3 tx.c");
                            generate_mac_add_sta(sta, current_tp);
                            //generate_mac_add_sta(local, current_tp);
                            //printk(KERN_DEBUG "Rathan: curr %lld inter %lld", current_tp, sta->start_time_period);
                            sta->start_time_period = current_tp;
                            
                            
                        }

                            
                        //print_mac_translation_table();
                        //print_mac_pair_table();
                        
                        break;

                    default:
                        printk(KERN_DEBUG "Rathan: interface unknown \n");
                        break;
                }
            //}
        } else {
            //printk(KERN_DEBUG "Rathan: sta is NULL\n");
        }
    } 
}



//change the base mac address of the header in the packet with randomized mac address


void mac_addr_change_hdr_tx (struct sk_buff_head *skbs, struct ieee80211_vif *vif){

    
	struct ieee80211_hdr *hdr;
	struct sk_buff *skb_rnd;
	struct mac_translation_entry *entry;
	struct mac_pair *s_entry;


    skb_rnd = skb_peek(skbs);
	hdr = (struct ieee80211_hdr *) skb_rnd->data;

    if(RND_MAC){
			switch (vif->type) {
				case NL80211_IFTYPE_STATION:
					//printk(KERN_INFO "The instance is a STA (Station)\n");

					s_entry = s_search_by_base_mac(hdr->addr1);
					if (s_entry) {
						memcpy(hdr->addr1, s_entry->s_random_mac, ETH_ALEN);
						//printk(KERN_DEBUG "STA RX: addr1 changed back");
					}

					s_entry = s_search_by_base_mac(hdr->addr2);
					if (s_entry) {
						memcpy(hdr->addr2, s_entry->s_random_mac, ETH_ALEN);
						//printk(KERN_DEBUG "STA RX: addr2 changed back");
					}

					s_entry = s_search_by_base_mac(hdr->addr3);
					if (s_entry) {
						memcpy(hdr->addr3, s_entry->s_random_mac, ETH_ALEN);
						//printk(KERN_DEBUG "STA RX: addr3 changed back");
					}

					
					s_entry = s_search_by_base_mac(hdr->addr4);
					if (s_entry) {
						memcpy(hdr->addr4, s_entry->s_random_mac, ETH_ALEN);
						//printk(KERN_DEBUG "STA RX: addr4 changed back");
					}
					break;
				case NL80211_IFTYPE_AP:
					//printk(KERN_INFO "The instance is an AP (Access Point)\n");
					
					entry = search_by_base_mac(hdr->addr1);
					if (entry) {
						memcpy(hdr->addr1, entry->random_mac, ETH_ALEN);
						//printk(KERN_DEBUG "AP RX: addr1 changed back");
					}

					entry = search_by_base_mac(hdr->addr2);
					if (entry) {
						memcpy(hdr->addr2, entry->random_mac, ETH_ALEN);
						//printk(KERN_DEBUG "AP RX: addr2 changed back");
					}

					entry = search_by_base_mac(hdr->addr3);
					if (entry) {
						memcpy(hdr->addr3, entry->random_mac, ETH_ALEN);
						//printk(KERN_DEBUG "AP RX: addr3 changed back");
					}

					
					entry = search_by_base_mac(hdr->addr4);
					if (entry) {
						memcpy(hdr->addr4, entry->random_mac, ETH_ALEN);
						//printk(KERN_DEBUG "AP RX: addr4 changed back");
					}
					break;
				default:
					printk(KERN_INFO "The instance type is UNKNOWN\n");
					break;
		}

	}

}



void mac_addr_change_hdr_rx (struct ieee80211_local *local, struct ieee80211_hdr *hdr){

    enum rathan_instance_type instance_type;
    struct mac_pair *s_entry;
	struct mac_translation_entry *entry;
    struct ieee80211_sub_if_data *sdata_instance;
    long long int current_tp;

    
    


    if (local){
		instance_type = which_instance(local);
	}

    if(RND_MAC){
		switch (instance_type) {
			case rathan_INSTANCE_STA:
				//printk(KERN_INFO "The instance is a STA (Station)\n");
				//printk(KERN_DEBUG "Instance mac: %pM\n", instance_mac);
				
				
				//printk(KERN_DEBUG "sta RX skb: seq number %u\n", le16_to_cpu(hdr->seq_ctrl) >> 4);

				s_entry = s_search_by_random_mac(hdr->addr1);
				if (s_entry) {
					memcpy(hdr->addr1, s_entry->s_base_mac, ETH_ALEN);
					//printk(KERN_DEBUG "STA RX: addr1 changed back");
				}

				s_entry = s_search_by_random_mac(hdr->addr2);
				if (s_entry) {
					memcpy(hdr->addr2, s_entry->s_base_mac, ETH_ALEN);
					//printk(KERN_DEBUG "STA RX: addr2 changed back");
				}

				s_entry = s_search_by_random_mac(hdr->addr3);
				if (s_entry) {
					memcpy(hdr->addr3, s_entry->s_base_mac, ETH_ALEN);
					//printk(KERN_DEBUG "STA RX: addr3 changed back");
				}

				
				s_entry = s_search_by_random_mac(hdr->addr4);
				if (s_entry) {
					memcpy(hdr->addr4, s_entry->s_base_mac, ETH_ALEN);
					//printk(KERN_DEBUG "STA RX: addr4 changed back");
				}
				//print_mac_pair_table();

				break;
			case rathan_INSTANCE_AP:
				//printk(KERN_INFO "The instance is an AP (Access Point)\n");
				//printk(KERN_DEBUG "Interface: %pM\n", instance_mac);
				sdata_instance = get_ap_sdata(local);
                current_tp = (ktime_get_real_seconds()/RND_TP);
				if (sdata_instance) {
					//printk(KERN_DEBUG "AP RX current %lld instance: %lld\n", current_tp, sdata_instance->start_time_period);
					if (current_tp != sdata_instance->start_time_period ) {
						//printk(KERN_DEBUG " AP RX: New Time Period\n");
						generate_mac_add_ap_all(local, current_tp);
						sdata_instance->start_time_period = current_tp;
						
					}
				}
				entry = search_by_random_mac(hdr->addr1);
				if (entry) {
					memcpy(hdr->addr1, entry->base_mac, ETH_ALEN);
					//printk(KERN_DEBUG "AP RX: addr1 changed back");
				}

				entry = search_by_random_mac(hdr->addr2);
				if (entry) {
					memcpy(hdr->addr2, entry->base_mac, ETH_ALEN);
					//printk(KERN_DEBUG "AP RX: addr2 changed back");
				}

				entry = search_by_random_mac(hdr->addr3);
				if (entry) {
					memcpy(hdr->addr3, entry->base_mac, ETH_ALEN);
					//printk(KERN_DEBUG "AP RX: addr3 changed back");
				}

				
				entry = search_by_random_mac(hdr->addr4);
				if (entry) {
					memcpy(hdr->addr4, entry->base_mac, ETH_ALEN);
					//printk(KERN_DEBUG "AP RX: addr4 changed back");
				}
				//print_mac_translation_table();
				break;
			case rathan_INSTANCE_UNKNOWN:
			default:
				printk(KERN_INFO "The instance type is UNKNOWN\n");
				break;
		}

	}

}


//custom packet building function

struct sk_buff *construct_custom_packet(struct ieee80211_vif *vif){

    struct sk_buff *custom_skb;
    struct ieee80211_hdr *hdr;
    int hdr_len = sizeof(struct ieee80211_hdr);
    int payload_len = 100;
    int total_len = hdr_len + payload_len;
    u8 *payload;

    //Allocate a new skb for the custom packet 
    custom_skb = dev_alloc_skb(total_len);
    if (!custom_skb) {
        printk(KERN_DEBUG "Failed to allocate a new skb\n");
        return NULL;
    }

    //Reserve space for the header
    skb_reserve(custom_skb, hdr_len);

    //Add the payload
    payload = skb_put(custom_skb, payload_len);
    memset(payload, 0xab, payload_len);

    //Fill in the header
    hdr = (struct ieee80211_hdr *)skb_push(custom_skb, hdr_len);

    //set the frame control field of the header and subtype 
    hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | 0x00F0); // Invalid subtype for Management frame

    // Assuming hdr and vif are already defined and initialized
	memset(hdr->addr1, 0xFF, ETH_ALEN);  // Destination address (broadcast)
    memcpy(hdr->addr2, vif->addr, ETH_ALEN);           // Source address (AP address)
    memcpy(hdr->addr3, vif->addr, ETH_ALEN);           // BSSID (AP address)

    return custom_skb;

}