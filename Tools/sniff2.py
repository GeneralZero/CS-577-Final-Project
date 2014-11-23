#!/usr/bin/env python
# The previous line ensures that this script is run under the context
# of the Python interpreter. Next, import the Scapy functions:
from scapy.all import *

# Define the interface name that we will be sniffing from, you can
# change this if needed.
interface = "wlan0-1"

# Next, declare a Python list to keep track of client MAC addresses
# that we have already seen so we only print the address once per client.
observedclients = []

# The sniffmgmt() function is called each time Scapy receives a packet
# (we'll tell Scapy to use this function below with the sniff() function).
# The packet that was sniffed is passed as the function argument, "p".
def sniffmgmt(p):
    info = {}
    info["crypto"] = []

    # Define our tuple (an immutable list) of the 3 management frame
    # subtypes sent exclusively by clients. I got this list from Wireshark.
    stamgmtstypes = (4, 10)

    # Make sure the packet has the Scapy Dot11 layer present
    if p.haslayer(Dot11Elt):
        pkt = p[Dot11Elt]

        cap = pkt.sprintf("{Dot11Beacon:%Dot11Beacon.cap%}"
                      "{Dot11ProbeResp:%Dot11ProbeResp.cap%}").split('+')

        # Check to make sure this is a management frame (type=0) and that
        # the subtype is one of our management frame subtypes indicating a
        # a wireless client
        
        while isinstance(pkt, Dot11Elt):
            if pkt.ID == 0:
                if pkt.info and pkt.info in observedclients:
                    return
                elif pkt.info == '':
                    return
                else:
                    info["ssid"] = pkt.info
                    observedclients.append(pkt.info)

            elif pkt.ID == 48:
                info["crypto"] = ["WPA2"]
            elif pkt.ID == 221 and pkt.info.startswith('\x00P\xf2\x01\x01\x00'):
                info["crypto"] = ["WPA"]
            elif 'privacy' in cap:
                info["crypto"] = ["WEP"]
            else:
                if len(info["crypto"]) == 0:
                    info["crypto"] = ["OPEN"]

            pkt = pkt.payload
        print info
        
# With the sniffmgmt() function complete, we can invoke the Scapy sniff()
# function, pointing to the monitor mode interface, and telling Scapy to call
# the sniffmgmt() function for each packet received. Easy!
sniff(iface=interface, prn=sniffmgmt)
