import socket
import time
import random

# Configuration - Match your config.yaml
HOST = '127.0.0.1'
PORT = 5005

def calculate_checksum(payload):
    """
    Calculates the NMEA XOR checksum.
    The checksum is the 8-bit XOR of all characters in the sentence
    between '$' and '*' (not including those delimiters).
    """
    checksum = 0
    for char in payload:
        checksum ^= ord(char)
    # Return as a 2-character uppercase hex string
    return f"{checksum:02X}"

def generate_nmea_pair():
    """
    Generates a GGA and GST pair with matching timestamps
    and valid dynamic checksums.
    """
    # Current UTC time: HHMMSS.00
    t = time.strftime("%H%M%S.00", time.gmtime())

    # 1. Build GGA (Position)
    # Format: $GPGGA,time,lat,N,lon,W,fix,sats,hdop,alt,M,geoid,M,age,id*checksum
    gga_data = f"GPGGA,{t},3723.465877,N,12202.269578,W,4,10,0.8,54.0,M,-25.0,M,,"
    gga = f"${gga_data}*{calculate_checksum(gga_data)}\r\n"

    # 2. Build GST (Covariance)
    # Format: $GPGST,time,rms,smjr,smnr,orient,lat_std,lon_std,alt_std*checksum
    gst_data = f"GPGST,{t},0.05,0.02,0.02,0.0,0.015,0.015,0.03"
    gst = f"${gst_data}*{calculate_checksum(gst_data)}\r\n"

    return gga + gst

def main():
    print(f"--- Starting NMEA Fragmentation Test Server ---")
    print(f"Listening on {HOST}:{PORT}...")

    # Set up TCP Server
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen()

        while True:
            print("\n[STATUS] Waiting for Hemisphere Driver Node to connect...")
            conn, addr = s.accept()
            with conn:
                print(f"[CONNECTED] Node address: {addr}")
                try:
                    while True:
                        # 1. Create fresh valid data
                        full_payload = generate_nmea_pair()

                        # 2. Randomly decide how many chunks to split the 2 sentences into (3 to 7)
                        num_chunks = random.randint(3, 7)

                        # 3. Generate random split indices
                        indices = sorted(random.sample(range(1, len(full_payload)), num_chunks - 1))
                        indices = [0] + indices + [len(full_payload)]

                        print(f"Sending epoch at {full_payload[7:16]} in {num_chunks} fragments:")

                        for i in range(num_chunks):
                            chunk = full_payload[indices[i]:indices[i+1]]
                            print(f"  -> Chunk {i+1}: {repr(chunk)}")
                            conn.sendall(chunk.encode())

                            # Random delay to simulate network jitter / serial buffering
                            time.sleep(random.uniform(0.01, 0.05))

                        print("[OK] Epoch Sent.\n")

                        # GPS Update Rate (1Hz)
                        time.sleep(1.0)

                except (ConnectionResetError, BrokenPipeError):
                    print("[DISCONNECTED] Node closed the connection.")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n[EXIT] Server stopped by user.")
