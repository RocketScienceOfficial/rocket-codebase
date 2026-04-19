import socket
import sys
from sim import datalink


class TCPSocket:
    def __init__(self, name, ip, port, is_server, blocking):
        self.name = name
        self.blocking = blocking
        self.current_buffer = bytearray()

        if is_server:
            self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_sock.bind(("", port))
            self.server_sock.listen(1)

            self._log(f"Listening on port {port}...")

            try:
                while True:
                    try:
                        self.server_sock.settimeout(1.0)
                        conn, addr = self.server_sock.accept()
                        self.sock = conn
                        self.server_sock.setblocking(self.blocking)
                        break
                    except socket.timeout:
                        continue
            except:
                sys.exit(1)

            self._log(f"Client connected: {addr[0]}:{addr[1]}")
        else:
            self._log(f"Connecting to {ip}:{port}...")

            try:
                while True:
                    try:
                        self.server_sock = None
                        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        self.sock.connect((ip, port))
                        break
                    except ConnectionRefusedError:
                        self._log(f"Connection refused, retrying...")
            except:
                sys.exit(1)

            self._log(f"Connected")

        self.sock.setblocking(self.blocking)

    def send_raw(self, data: bytes):
        try:
            self.sock.sendall(data)
        except (BlockingIOError, BrokenPipeError, ConnectionResetError, OSError):
            self.close()

    def send(self, msg: datalink.datalink_message):
        self.send_raw(datalink.datalink_serial.serialize(msg))

    def receive_raw(self, n) -> bytes:
        while len(self.current_buffer) < n:
            try:
                to_read = n - len(self.current_buffer)
                data = self.sock.recv(to_read)

                if not data:
                    self.close()
                    return None

                self.current_buffer.extend(data)

                if not self.blocking and len(self.current_buffer) < n:
                    return None
            except BlockingIOError:
                if not self.blocking:
                    return None
            except (ConnectionResetError, BrokenPipeError):
                self.close()
                return None

        result = self.current_buffer[:n]
        self.current_buffer = self.current_buffer[n:]

        return bytes(result)

    def receive(self) -> datalink.datalink_message | None:
        while True:
            try:
                data = self.sock.recv(2048)

                if not data:
                    self.close()
                    return None

                self.current_buffer.extend(data)

                for i in range(len(self.current_buffer)):
                    if self.current_buffer[i] == 0x00:
                        msg = datalink.datalink_serial.deserialize(self.current_buffer[:i+1])
                        self.current_buffer = self.current_buffer[i+1:]
                        return msg

                if not self.blocking:
                    return None
            except BlockingIOError:
                if not self.blocking:
                    return None
            except (ConnectionResetError, BrokenPipeError):
                self.close()
                return None

    def close(self):
        if self.sock:
            self.sock.close()
            self.sock = None

        if self.server_sock:
            self.server_sock.close()
            self.server_sock = None

        self._log("Closed")

    def _log(self, msg):
        print(f"[network]   ({self.name})  {msg}")
