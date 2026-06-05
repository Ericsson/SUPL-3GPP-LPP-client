#!/usr/bin/env python3
"""Read one serial port, broadcast raw bytes to all connected TCP clients."""
import socket, threading, os, sys, termios, struct

device   = os.environ.get('GNSS_DEVICE', '/dev/ttyACM0')
baudrate = int(os.environ.get('GNSS_BAUDRATE', '115200'))
port     = int(os.environ.get('MUX_PORT', '3001'))

BAUD_MAP = {
    9600: termios.B9600, 19200: termios.B19200, 38400: termios.B38400,
    57600: termios.B57600, 115200: termios.B115200, 230400: termios.B230400,
}

clients = []
clients_lock = threading.Lock()

def open_serial(dev, baud):
    fd = os.open(dev, os.O_RDONLY | os.O_NOCTTY | os.O_NONBLOCK)
    os.set_blocking(fd, True)
    attrs = termios.tcgetattr(fd)
    attrs[4] = attrs[5] = BAUD_MAP[baud]
    attrs[0] = attrs[1] = attrs[3] = 0  # raw
    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    return fd

def accept_loop(srv):
    while True:
        conn, addr = srv.accept()
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        with clients_lock:
            clients.append(conn)
        print(f"client connected: {addr}", flush=True)

srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(('0.0.0.0', port))
srv.listen(8)
print(f"listening on :{port}, reading {device} @ {baudrate}", flush=True)

threading.Thread(target=accept_loop, args=(srv,), daemon=True).start()

fd = open_serial(device, baudrate)
while True:
    data = os.read(fd, 4096)
    if not data:
        continue
    dead = []
    with clients_lock:
        for c in clients:
            try:
                c.sendall(data)
            except Exception:
                dead.append(c)
        for c in dead:
            clients.remove(c)
            print("client disconnected", flush=True)
