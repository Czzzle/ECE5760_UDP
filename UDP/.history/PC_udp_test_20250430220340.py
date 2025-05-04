import socket

pico_ip = "192.168.x.x"  # 改成你PicoW的IP
port = 1234

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"Hello from PC!", (pico_ip, port))