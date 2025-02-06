from scapy.all import *
import socket
import json

class BankProtocol(Packet):
    name = "BankProtocol"
    fields_desc = [
        StrField("message_type", ""),
        StrField("username", ""),
        StrField("password", ""),
        StrField("transaction_details", "")
    ]

bind_layers(TCP, BankProtocol)

def send_request(packet):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(packet, ('127.0.0.1', 1234))
    response, _ = sock.recvfrom(1024)
    return json.loads(response.decode())

def main():
    print("Welcome to ATM System")
    
    username = input("Enter username: ")
    password = input("Enter password: ")
    
    # Authentication request
    auth_packet = json.dumps({
        "packet_type": "auth",
        "username": username,
        "password": password
    }).encode()
    
    response = send_request(auth_packet)
    if response["status"] == "failed":
        print("Authentication failed")
        return

    while True:
        print("\n1. Check Balance")
        print("2. Withdraw")
        print("3. Deposit")
        print("4. Exit")
        
        choice = input("\nEnter choice (1-4): ")
        
        if choice == "1":
            packet = json.dumps({
                "packet_type": "balance",
                "username": username
            }).encode()
            response = send_request(packet)
            print(f"Current balance: ${response['balance']}")
            
        elif choice == "2":
            amount = int(input("Enter amount to withdraw: $"))
            packet = json.dumps({
                "packet_type": "withdraw",
                "username": username,
                "transaction_details": str(amount)
            }).encode()
            response = send_request(packet)
            if response["status"] == "success":
                print(f"Withdrawal successful. New balance: ${response['balance']}")
            else:
                print(response["message"])
            
        elif choice == "3":
            amount = int(input("Enter amount to deposit: $"))
            packet = json.dumps({
                "packet_type": "deposit",
                "username": username,
                "transaction_details": str(amount)
            }).encode()
            response = send_request(packet)
            print(f"Deposit successful. New balance: ${response['balance']}")
            
        elif choice == "4":
            break

if __name__ == "__main__":
    main()