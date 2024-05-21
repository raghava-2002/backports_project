#include <linux/kernel.h>
#include <linux/slab.h>

#include "rathan_flags.h"

// Define the global flag
bool packet_sent_from_ap = false;
bool packet_sent_from_sta = false;

/* void send_packet_from_ap() {
    // Set the flag to true when sending a packet from AP
    packet_sent_from_ap = true;

    // Code to send the packet...
}

void send_packet_from_sta() {
    // Set the flag to false when sending a packet from STA
    packet_sent_from_ap = false;

    // Code to send the packet...
}
 */