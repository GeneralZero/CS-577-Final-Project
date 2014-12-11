#!/usr/bin/env python

import sys, subprocess, re, pickle, os.path, datetime

# The previous line ensures that this script is run under the context
# of the Python interpreter. Next, import the Scapy functions:
from scapy.all import Dot11, Dot11Elt, Dot11Beacon, sniff


# Define the interface name that we will be sniffing from, you can
# change this if needed.
if len(sys.argv) > 1:
    monitor_interface = sys.argv[1]
else:
    monitor_interface = "wlan0-1"
maxAPs = 20

# Next, declare a Python list to keep track of client MAC addresses
# that we have already seen so we only print the address once per client.
FoundAPs = {}

currentSpoofedNetworks = {}

previouslySpoofed = {}

# The sniffmgmt() function is called each time Scapy receives a packet
# (we'll tell Scapy to use this function below with the sniff() function).
# The packet that was sniffed is passed as the function argument, "p".
def sniffmgmt(p):
    info = {}
    info["crypto"] = []

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
                if pkt.info and pkt.info in FoundAPs:
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

        if info != None and info["ssid"] != '':
            print info
            createAP(info["ssid"], info["crypto"])
            FoundAPs[info["ssid"]] = info

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
            if output.find("No such wireless device:") == -1 and dev != monitor_interface:
                info = re.findall('ESSID: "(.*)"|Encryption: (.*)', output)
                info = [y if y!= "" else z for y,z in info ]
                if dev not in currentSpoofedNetworks:
                    currentSpoofedNetworks[dev] = {}
                currentSpoofedNetworks[dev]["ssid"] = info[0]
                currentSpoofedNetworks[dev]["crypto"] = info[1]
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
            if output.find("No station connected") == -1 and dev != monitor_interface:
                #find all MAC addresses
                maclist = re.findall("(([0-9A-F]{2}[:]){5}[0-9A-F]{2})", output)
                maclist = [y for y,_ in maclist ]
                if dev not in currentSpoofedNetworks:
                    currentSpoofedNetworks[dev] = {}
                    currentSpoofedNetworks[dev]["start"] = datetime.datetime.now()
                currentSpoofedNetworks[dev]["clients"] = maclist

                if len(maclist) > 0:
                    #remove Other ssid than this one.
                    removeOthers(dev)

                    #add to list of completed aps
                    if currentSpoofedNetworks[dev]["ssid"] not in previouslySpoofed:
                        previouslySpoofed[currentSpoofedNetworks[dev]["ssid"]] = currentSpoofedNetworks[dev]
            else:
                return
        except subprocess.CalledProcessError, e:
            return

def removeAP(interface):
    if interface != monitor_interface:
        subprocess.call(["lua", "removeap.lua", interface])

def removeOldAP(count):
    time = datetime.max
    oldinf = ""

    for inf in currentSpoofedNetworks:
        if inf != monitor_interface and time > currentSpoofedNetworks[inf]["start"]:
            time = currentSpoofedNetworks[inf]["start"]
            oldinf = inf
    removeAP(oldinf)

def removeOthers(interface):
    ssid = currentSpoofedNetworks[interface]["ssid"]
    for int in currentSpoofedNetworks:
        if int != interface and currentSpoofedNetworks[inf]["ssid"] == ssid:
            removeAP(inf)
    subprocess.call(["lua", "saveAP.lua",])

def createAP(ssid, crypto):
    getAPinfo()

    #Create profile for all
    if crypto == "none":
        if ssid in previouslySpoofed:
            return createAP(ssid, previouslySpoofed[ssid]["crypto"])
        if len(currentSpoofedNetworks) + 6 > maxAPs:
            for x in range(len(currentSpoofedNetworks) + 6 - maxAPs):
                removeOldAP()
                
        subprocess.call(["lua", "apgenerator.lua", ssid, crypto])
        subprocess.call(["lua", "apgenerator.lua", ssid, "wep", "ABCDEF1234567890"])
        subprocess.call(["lua", "apgenerator.lua", ssid, "wpa", "ABCDEF1234567890"])
        subprocess.call(["lua", "apgenerator.lua", ssid, "wpa"])
        subprocess.call(["lua", "apgenerator.lua", ssid, "wpa2", "ABCDEF1234567890"])
        subprocess.call(["lua", "apgenerator.lua", ssid, "wpa2"])
        subprocess.call(["lua", "saveAP.lua"])


    #Create profile for WEP-open WEP-PSK
    elif crypto == "wep+mixed":
        if len(currentSpoofedNetworks) + 1 > maxAPs:
            removeOldAP()
        subprocess.call(["lua", "apgenerator.lua", ssid, crypto, "ABCDEF1234567890"])
        subprocess.call(["lua", "saveAP.lua"])

    #Create profile for WPA2-EAP and WPA2-PSK
    elif crypto == "wpa":
        if len(currentSpoofedNetworks) + 2 > maxAPs:
            for x in range(len(currentSpoofedNetworks) + 2 - maxAPs):
                removeOldAP()
        subprocess.call(["lua", "apgenerator.lua", ssid, crypto, "ABCDEF1234567890"])
        subprocess.call(["lua", "apgenerator.lua", ssid, crypto])
        subprocess.call(["lua", "saveAP.lua"])

    #Create profile for WPA2-EAP and WPA2-PSK
    elif crypto == "wpa2":
        if len(currentSpoofedNetworks) + 2 > maxAPs:
            for x in range(len(currentSpoofedNetworks) + 2 - maxAPs):
                removeOldAP()
        subprocess.call(["lua", "apgenerator.lua", ssid, crypto, "ABCDEF1234567890"])
        subprocess.call(["lua", "apgenerator.lua", ssid, info["crypto"]])
        subprocess.call(["lua", "saveAP.lua"])


def loadAPinfo():
    if os.path.isfile("AP.pickle"):
        previouslySpoofed = pickle.load(open("AP.pickle", "rb+"))

def saveAPinfo():
    pickle.dump(previouslySpoofed, open("AP.pickle", "wb+"))

# With the sniffmgmt() function complete, we can invoke the Scapy sniff()
# function, pointing to the monitor mode interface, and telling Scapy to call
# the sniffmgmt() function for each packet received. Easy!

loadAPinfo()
while 1:
    try:
        sniff(iface=monitor_interface, lfilter=lambda x: x.haslayer(Dot11) and x.type == 0, prn=sniffmgmt)
    except socket.error:
        sleep(10)

saveAPinfo()