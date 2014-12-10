#!/usr/bin/env python

import subprocess, re, Queue, pickle, os.path

# The previous line ensures that this script is run under the context
# of the Python interpreter. Next, import the Scapy functions:
from scapy.all import Dot11, Dot11Elt, Dot11Beacon, sniff


# Define the interface name that we will be sniffing from, you can
# change this if needed.
interface = "wlan0-1"
maxAPs = 20

# Next, declare a Python list to keep track of client MAC addresses
# that we have already seen so we only print the address once per client.
observedclients = {}

queueAP = Queue.Queue()

wlans = {}

dumpData = {}

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
                    
            elif pkt.ID == 48:
                info["crypto"] = "wpa2"
            elif pkt.ID == 221 and pkt.info.startswith('\x00P\xf2\x01\x01\x00'):
                info["crypto"] = "wpa"
            elif 'privacy' in cap:
                info["crypto"] = "wep+mixed"
            else:
                if len(info["crypto"]) == 0:
                    info["crypto"] = "none"

            pkt = pkt.payload

        if info["ssid"] != '':
            print info
            observedclients[info["ssid"]] = info

def getAPinfo():
    """
    This function will scrape the info from the commandline.
    This function will also create a array
    """
    for x in range(maxAPs):
        try:
            if x == 0:
                dev = "wlan0"
            else:
                dev = "wlan0-" + str(x)
            output = subprocess.check_output(["iwinfo", dev, "info"])

            #Parse Output
            if output.find("No station connected") == -1:
                info = re.findall('ESSID: "(.*)"|Encryption: (.*)', output)
                info = [y if y!= "" else z for y,z in info ]
                if "wlan0-" + str(x) not in wlans:
                    wlans[dev] = {}
                wlans[dev]["ssid"] = info[0]
                wlans[dev]["crypto"] = info[1]
            else:
                return
        except subprocess.CalledProcessError, e:
            return

def getAPclients():
    """
    This Function scrapes association list from the commandline.
    This will also grab ssids with the same name and if someone has assocated with one of the APs it will remove the other ones.
    """
    for x in range(maxAPs):
        try:
            if x == 0:
                dev = "wlan0"
            else:
                dev = "wlan0-" + str(x)
            output = subprocess.check_output(["iwinfo", dev, "assoclist"])
            #Parse output
            if output.find("No such wireless device:") == -1:
                #find all MAC addresses
                maclist = re.findall("(([0-9A-F]{2}[:]){5}[0-9A-F]{2})", output)
                maclist = [y for y,_ in maclist ]
                if dev not in wlans:
                    wlans[dev] = {}
                wlans[dev]["clients"] = maclist

                if len(maclist) > 0:
                    #remove Other ssid than this one.

                    #add to list of completed aps
                    dumpData.append(wlans[dev])
            else:
                return
        except subprocess.CalledProcessError, e:
            return

def createAP(ssid, crypto):
    getAPinfo()

    #Create profile for all
    if crypto == "none":
        if len(wlans) + 7 <= maxAPs:
            subprocess.call(["lua", "apgenerator.lua", info["ssid"], info["crypto"]])
            subprocess.call(["lua", "apgenerator.lua", info["ssid"], "wep+mixed"])
            subprocess.call(["lua", "apgenerator.lua", info["ssid"], "wpa"])
            subprocess.call(["lua", "apgenerator.lua", info["ssid"], "wpa2"])
        else:
            queueAP.put({"ssid": ssid,"crypto": crypto})
            #put in Queue and wait for APs to be brought down.

    #Create profile for WEP-open WEP-PSK
    elif crypto == "wep+mixed":
        if len(wlans) + 2 <= maxAPs:
            subprocess.call(["lua", "apgenerator.lua", info["ssid"], info["crypto"]])
        else:
            #put in Queue and wait for APs to be brought down.
            queueAP.put({"ssid": ssid,"crypto": crypto})

    #Create profile for WPA2-EAP and WPA2-PSK
    elif crypto == "wpa":
        if len(wlans) + 2 <= maxAPs:
            subprocess.call(["lua", "apgenerator.lua", info["ssid"], info["crypto"]])
        else:
            #put in Queue and wait for APs to be brought down.
            queueAP.put({"ssid": ssid,"crypto": crypto})

    #Create profile for WPA2-EAP and WPA2-PSK
    elif crypto == "wpa2":
        if len(wlans) + 2 < maxAPs:
            subprocess.call(["lua", "apgenerator.lua", info["ssid"], info["crypto"]])
        else:
            #put in Queue and wait for APs to be brought down.
            queueAP.put({"ssid": ssid,"crypto": crypto})


def loadAPinfo():
    if os.path.isfile("AP.pickle"):
        dumpData = pickle.load("AP.pickle")

def saveAPinfo():
    pickle.dump(dumpData, "AP.pickle")

# With the sniffmgmt() function complete, we can invoke the Scapy sniff()
# function, pointing to the monitor mode interface, and telling Scapy to call
# the sniffmgmt() function for each packet received. Easy!

loadAPinfo()

sniff(iface=interface, lfilter=lambda x: x.haslayer(Dot11), prn=sniffmgmt)

saveAPinfo()