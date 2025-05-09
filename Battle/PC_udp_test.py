import socket

pico_ip = "172.20.10.2"  # 改成你PicoW的IP
port = 1234

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"Hello from PC", (pico_ip, port))
print("Send hello");
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", port))

print(f"UDP server listening on port {port}...")

while True:
    data, addr = sock.recvfrom(512)
    print(f"Received from {addr}: {data.decode()}")
    sock.sendto(b"Recieved", (pico_ip, port))
# Hello from PCECFCFHDEDADFECEEDIDCDGDBECDGDCAA
