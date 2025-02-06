from scapy.all import send, IP, ICMP
send(IP(src="172.16.0.2",dst="172.16.0.2")/ICMP()/"Hello World")
