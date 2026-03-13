import socket
import time
import random

HOST = '127.0.0.1'
PORT = 5005
FULL_SENTENCE = "$GPGGA,172814.0,3723.465877,N,12202.269578,W,1,10,0.8,54.0,M,-25.0,M,,*5B\r\n"

print(f"Starting Continuous Fragmentation Test on {HOST}:{PORT}...")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()

    while True:
        print("Waiting for ROS node connection...")
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            try:
                while True:
                    # Randomly decide how many chunks to split the sentence into (2 to 5)
                    num_chunks = random.randint(2, 5)

                    # Generate random split indices
                    indices = sorted(random.sample(range(1, len(FULL_SENTENCE)), num_chunks - 1))
                    indices = [0] + indices + [len(FULL_SENTENCE)]

                    print(f"--- Starting New Sentence Cycle ({num_chunks} chunks) ---")

                    for i in range(num_chunks):
                        chunk = FULL_SENTENCE[indices[i]:indices[i+1]]
                        print(f"  Sending: {repr(chunk)}") # repr shows the \r\n explicitly
                        conn.sendall(chunk.encode())

                        # Random delay between 0.05s and 0.2s to simulate network jitter
                        time.sleep(random.uniform(0.05, 0.2))

                    print("--- Sentence Cycle Complete ---\n")
                    time.sleep(0.5) # Short pause before the next full sentence starts

            except (ConnectionResetError, BrokenPipeError):
                print("Node disconnected. Waiting for new connection...")
