import socket

pico_ip = "172.20.10.2"  # 改成你PicoW的IP
port = 1234

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"Hello from PC!", (pico_ip, port))