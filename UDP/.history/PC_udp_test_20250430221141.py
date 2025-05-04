import socket

pico_ip = "172.20.10.3"  # 改成你PicoW的IP
port = 1234

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"Hello from PC!", (pico_ip, port))

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", port))

print(f"✅ UDP server listening on port {port}...")

# === 无限循环接收数据 ===
while True:
    data, addr = sock.recvfrom(128)
    print(f"📩 Received from {addr}: {data.decode()}")