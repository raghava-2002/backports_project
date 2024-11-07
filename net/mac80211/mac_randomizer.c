// mac_randomizer.c

//written by Rathan Appana

#include <linux/random.h>
#include <linux/etherdevice.h>
#include "mac_randomizer.h"


bool RND_MAC = false;

//bool RND_MAC = true;    // Enable random MAC address generation logic 

//enable random mac address generation by kernal time interval  
bool RND_KERN = false;

//enable random mac address generation by AP intiated triggers
bool RND_AP = false;

int packet_count = 0; //packet count for the AP initiated trigger
int no_of_custom_packets = 3; //no of packets to be sent by the AP to trigger the random mac address generation

//long long int rcv_mac_seed =0; //to keep track of current seed for the station

bool debug = true;

//update the variable rnd_mac_validity_period for the pn generation used only by the AP , but it is updated by the station also and uses 
u8 rnd_mac_validity_period = RND_TP;
long long int gen_mac_seed = 0;

//################for the kernel time period based random mac address privacy analysis #####################
//enable the clock drift for the stations
//introduce some clock drift
bool KERN_DRIFT = false;

//since 11 stations are there we have to keep track of the time drift for each station
//after kernel time directly impacts all the stations to have same time drift so we introducindelay in the generation of mac address for the stations

//#############################################################################################################

// Define the MAC addresses for the specific stations
static unsigned char station0_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char station1_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00};
static unsigned char station2_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x02, 0x00};
static unsigned char station3_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x03, 0x00};
static unsigned char station4_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x04, 0x00};
static unsigned char station5_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x05, 0x00};
static unsigned char station6_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x06, 0x00};
static unsigned char station7_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x07, 0x00};
static unsigned char station8_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x08, 0x00};
static unsigned char station9_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x09, 0x00};
static unsigned char station10_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x0A, 0x00};


// Define delays for each station, in nanoseconds these are changes according to the normal distribution with mean 5ms and standard deviation 2ms and max 10ms

// [3.89438268 2.48618961 4.59870502]
/* static u64 drift_0 = 3894382;
static u64 drift_1 = 2486189;
static u64 drift_2 = 4598705;
static u64 drift_3 = 0;
static u64 drift_4 = 0;
static u64 drift_5 = 0;
static u64 drift_6 = 0;
static u64 drift_7 = 0;
static u64 drift_8 = 0;
static u64 drift_9 = 0;
static u64 drift_10 = 0; */


//                              [ 1111051  2420851  6267830  3536533 -3317616  -5152418  -506623  -7624538 -4046560 4617554 -1938435]
static s64 station_delays[] = {0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


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
    u64 current_tp_ns;
    s64 drift_ns;

    current_tp = (ktime_get_real_seconds()/RND_TP);


    trigger_flag = true;



   

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

                        //introducting some clock drift here for the station
                        if (KERN_DRIFT && interface_mac_addr) {   
                            current_tp_ns = ktime_get_real_ns();
                            drift_ns = 0;
                            // Apply different drift values based on station MAC address
                            if(memcmp(interface_mac_addr, station0_mac, ETH_ALEN) == 0) {
                                drift_ns = station_delays[0];
                            } else if (memcmp(interface_mac_addr, station1_mac, ETH_ALEN) == 0) {
                                drift_ns = station_delays[1]; 
                            } else if (memcmp(interface_mac_addr, station2_mac, ETH_ALEN) == 0) {
                                drift_ns = station_delays[2];  
                            } else if (memcmp(interface_mac_addr, station3_mac, ETH_ALEN) == 0){
                                drift_ns = station_delays[3]; 
                            }else if (memcmp(interface_mac_addr, station4_mac, ETH_ALEN) == 0){
                                drift_ns = station_delays[4];  
                            } else if (memcmp(interface_mac_addr, station5_mac, ETH_ALEN) == 0){
                                drift_ns = station_delays[5];
                            }else if (memcmp(interface_mac_addr, station6_mac, ETH_ALEN) == 0){
                                drift_ns = station_delays[6];
                            }else if (memcmp(interface_mac_addr, station7_mac, ETH_ALEN) == 0){
                                drift_ns = station_delays[7];
                            }else if (memcmp(interface_mac_addr, station8_mac, ETH_ALEN) == 0){
                                drift_ns = station_delays[8];
                            }else if (memcmp(interface_mac_addr, station9_mac, ETH_ALEN) == 0){
                                drift_ns = station_delays[9];
                            }else if (memcmp(interface_mac_addr, station10_mac, ETH_ALEN) == 0){
                                drift_ns = station_delays[10];
                            } else {
                                drift_ns = 0;  // No drift for other stations
                            }
                            current_tp_ns = (current_tp_ns + drift_ns);  // Adjust with drift

                            current_tp = ((current_tp_ns/1000000000)/RND_TP);
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
    struct sta_info *test_sta;
    //introducing some time delay for the stations only
    const u8 *interface_mac_addr;
    u64 current_tp_ns;
    s64 drift_ns;

    


    if (local){
		instance_type = which_instance(local);
	}

    if(RND_MAC){
		switch (instance_type) {
			case rathan_INSTANCE_STA:
				//printk(KERN_INFO "The instance is a STA (Station)\n");
				//printk(KERN_DEBUG "Instance mac: %pM\n", instance_mac);
				test_sta = local_to_sta_info(local);

                if (test_sta && RND_KERN) {
                    current_tp = (ktime_get_real_seconds()/RND_TP);

                    interface_mac_addr = test_sta->sdata->vif.addr;
                    //introducting some clock drift here for the station
                    if (KERN_DRIFT && interface_mac_addr) {   
                        current_tp_ns = ktime_get_real_ns();
                        drift_ns = 0;
                        // Apply different drift values based on station MAC address
                        if(memcmp(interface_mac_addr, station0_mac, ETH_ALEN) == 0) {
                            drift_ns = station_delays[0];
                        } else if (memcmp(interface_mac_addr, station1_mac, ETH_ALEN) == 0) {
                            drift_ns = station_delays[1]; 
                        } else if (memcmp(interface_mac_addr, station2_mac, ETH_ALEN) == 0) {
                            drift_ns = station_delays[2];  
                        } else if (memcmp(interface_mac_addr, station3_mac, ETH_ALEN) == 0){
                            drift_ns = station_delays[3]; 
                        }else if (memcmp(interface_mac_addr, station4_mac, ETH_ALEN) == 0){
                            drift_ns = station_delays[4];  
                        } else if (memcmp(interface_mac_addr, station5_mac, ETH_ALEN) == 0){
                            drift_ns = station_delays[5];
                        }else if (memcmp(interface_mac_addr, station6_mac, ETH_ALEN) == 0){
                            drift_ns = station_delays[6];
                        }else if (memcmp(interface_mac_addr, station7_mac, ETH_ALEN) == 0){
                            drift_ns = station_delays[7];
                        }else if (memcmp(interface_mac_addr, station8_mac, ETH_ALEN) == 0){
                            drift_ns = station_delays[8];
                        }else if (memcmp(interface_mac_addr, station9_mac, ETH_ALEN) == 0){
                            drift_ns = station_delays[9];
                        }else if (memcmp(interface_mac_addr, station10_mac, ETH_ALEN) == 0){
                            drift_ns = station_delays[10];
                        } else {
                            drift_ns = 0;  // No drift for other stations
                        }
                        current_tp_ns = (current_tp_ns + drift_ns);  // Adjust with drift
                        current_tp = ((current_tp_ns/1000000000)/RND_TP);
                    }

                    //printk(KERN_DEBUG "sta RX: extra load here");
                    if ((current_tp != (test_sta->start_time_period))) {
                        //printk(KERN_DEBUG "Station case3 tx.c");
                        generate_mac_add_sta(test_sta, current_tp);
                        //generate_mac_add_sta(local, current_tp);
                        //printk(KERN_DEBUG "Rathan: curr %lld inter %lld", current_tp, sta->start_time_period);
                        test_sta->start_time_period = current_tp;    
                        //printk(KERN_DEBUG "sta RX: extra load sucess here ");         
                    }
                }
				
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
                //below if only works with the mac randomization by kernel time interval
				if (sdata_instance && RND_KERN) {
					//printk(KERN_DEBUG "AP RX current %lld instance: %lld\n", current_tp, sdata_instance->start_time_period);
                    //printk(KERN_DEBUG "AP RX: %lld\n", sdata_instance->start_time_period);
					current_tp = (ktime_get_real_seconds()/RND_TP);
                    if (current_tp != sdata_instance->start_time_period ) {
						//printk(KERN_DEBUG " AP RX: New Time Period\n");
						generate_mac_add_ap_all(local, current_tp);
						sdata_instance->start_time_period = current_tp;
                        //printk(KERN_DEBUG "Fucking hell loop ");
						
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

struct sk_buff *construct_custom_packet(struct ieee80211_vif *vif, long long int mac_seed) {

    struct sk_buff *custom_skb;
    struct ieee80211_hdr *hdr;
    struct custom_packet_payload payload_data; // Custom packet payload data
    int hdr_len = sizeof(struct ieee80211_hdr);
    int payload_len = sizeof(payload_data);
    int total_len = hdr_len + payload_len;
    u8 *payload; //pointer to the payload
    struct ieee80211_tx_info *info;
    struct ieee80211_sub_if_data *sdata = NULL;
	enum nl80211_band band;
	struct ieee80211_chanctx_conf *chanctx_conf;
    struct ieee80211_tx_rate_control txrc;
    struct ieee80211_hw *hw;
    struct ieee80211_local *local;

    

    sdata = vif_to_sdata(vif);

    // Access the hw structure from sdata
    hw = &sdata->local->hw;
    local = hw_to_local(hw);
	chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);

    //Allocate a new skb for the custom packet 
    custom_skb = dev_alloc_skb(total_len);
    if (!custom_skb) {
        printk(KERN_DEBUG "Failed to allocate a new skb\n");
        return NULL;
    }

    // Prepare the payload data
    payload_data.mac_validity_period = RND_TP; //validity period for MAC address in seconds
    payload_data.mac_generation_seed = mac_seed; // Seed for MAC address generation
    strncpy(payload_data.message, "This is was a extra message to the station", sizeof(payload_data.message) - 1);
    payload_data.message[sizeof(payload_data.message) - 1] = '\0';

    //Reserve space for the header
    skb_reserve(custom_skb, hdr_len);

    //Add the payload
    payload = skb_put(custom_skb, payload_len);
    memcpy(payload, &payload_data, payload_len);



    //Fill in the header
    hdr = (struct ieee80211_hdr *)skb_push(custom_skb, hdr_len);

    //set the frame control field of the header and subtype 
    hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | 0x00F0); // Invalid subtype for Management frame
    hdr->frame_control |= cpu_to_le16(IEEE80211_FCTL_PROTECTED); // Protected frame

    // Assuming hdr and vif are already defined and initialized
	memset(hdr->addr1, 0xFF, ETH_ALEN);  // Destination address (broadcast)
    memcpy(hdr->addr2, vif->addr, ETH_ALEN);           // Source address (AP address)
    memcpy(hdr->addr3, vif->addr, ETH_ALEN);           // BSSID (AP address)

    // Set the frame control field (if needed)
    //hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_QOS_DATA); // QoS Data Frame

    band = chanctx_conf->def.chan->band;
    // Set flags in ieee80211_tx_info
    info = IEEE80211_SKB_CB(custom_skb);
    info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;
	info->flags |= IEEE80211_TX_CTL_NO_ACK;
    //info->flags |= IEEE80211_TX_CTL_CLEAR_PS_FILT;       
    //custom_skb->priority = 7;
    //info->hw_queue = IEEE80211_AC_VO;        // Voice queue 
	info->band = band;

    memset(&txrc, 0, sizeof(txrc));
	txrc.hw = hw;
	txrc.sband = local->hw.wiphy->bands[band];
	txrc.bss_conf = &sdata->vif.bss_conf;
	txrc.skb = custom_skb;
	txrc.reported_rate.idx = -1;
	txrc.rate_idx_mask = sdata->rc_rateidx_mask[band];
	txrc.bss = true;
	rate_control_get_rate(sdata, NULL, &txrc);

	info->control.vif = vif;

	info->flags |= IEEE80211_TX_CTL_CLEAR_PS_FILT |
			IEEE80211_TX_CTL_ASSIGN_SEQ |
			IEEE80211_TX_CTL_FIRST_FRAGMENT;
    //printk(KERN_DEBUG "Custom packet Flags are set\n");
    return custom_skb;

}