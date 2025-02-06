from scapy.all import *
import socket
import json

# Database simulation
bank_database = {
    "user123": {
        "password": "pass123",
        "balance": 1000,
        "transactions": []
    }
}

def process_request(data):
    try:
        message = json.loads(data.decode())
        packet_type = message.get("packet_type")
        username = message.get("username")
        password = message.get("password")
        transaction_details = message.get("transaction_details")

        if packet_type == "auth":
            if username in bank_database and bank_database[username]["password"] == password:
                return {"status": "success", "message": "Authentication successful"}
            return {"status": "failed", "message": "Invalid credentials"}
        
        if packet_type == "balance":
            if username in bank_database:
                return {"status": "success", "balance": bank_database[username]["balance"]}
        
        if packet_type == "withdraw":
            if username in bank_database:
                amount = int(transaction_details)
                if bank_database[username]["balance"] >= amount:
                    bank_database[username]["balance"] -= amount
                    bank_database[username]["transactions"].append(f"Withdrawal: ${amount}")
                    return {"status": "success", "balance": bank_database[username]["balance"]}
                return {"status": "failed", "message": "Insufficient funds"}
        
        if packet_type == "deposit":
            if username in bank_database:
                amount = int(transaction_details)
                bank_database[username]["balance"] += amount
                bank_database[username]["transactions"].append(f"Deposit: ${amount}")
                return {"status": "success", "balance": bank_database[username]["balance"]}
            
    except json.JSONDecodeError:
        return {"status": "failed", "message": "Invalid data format"}
    except Exception as e:
        return {"status": "failed", "message": str(e)}

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('127.0.0.1', 1234))
    print("Bank server started on port 1234")
    
    while True:
        data, addr = sock.recvfrom(1024)
        response = process_request(data)
        sock.sendto(json.dumps(response).encode(), addr)

if __name__ == "__main__":
    main()