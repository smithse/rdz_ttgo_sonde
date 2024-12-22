#!/usr/bin/env python3
import os
import socket
import http.server
import socketserver
import tempfile
import shutil
import subprocess
import argparse

# Parse command-line arguments: port number (default is 8000)
parser = argparse.ArgumentParser(description="Run a local web server.")
parser.add_argument("--port", type=int, default=8000, help="Port number to run the server on (default: 8000)")
args = parser.parse_args()

PORT = args.port

# Note: run this script in the top directory of the project!
MYPATH = os.getcwd()

# update image and file system localtions, relative to top directory
UPDIMG = os.path.join(MYPATH, ".pio/build/ttgo-lora32/firmware.bin")
FSDATA = os.path.join(MYPATH, "RX_FSK/data")

# Create a temporary unique directory for WEBROOT
WEBROOT = tempfile.mkdtemp()

print("TTGO rdzSonde development update server")
print("Make sure to compile firmware before running this server\n")

# Run the command line script
print("Preparing file system update using", FSDATA)
makefsupdate_script = os.path.join(MYPATH, "scripts/makefsupdate.py")
output_file = os.path.join(WEBROOT, "update.fs.bin")
with open(output_file, "w") as output:
    subprocess.run(["python", makefsupdate_script, FSDATA], stdout=output, check=True)

# Copy UPDIMG to WEBROOT with a new name
print("Preparing firmware update using", UPDIMG)
shutil.copy(UPDIMG, os.path.join(WEBROOT, "update.ino.bin"))

# Find the local IP address
def get_local_ip():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        try:
            s.connect(("8.8.8.8", 80))
            return s.getsockname()[0]
        except Exception:
            return "127.0.0.1"

local_ip = get_local_ip()

# Print IP address and port
print(f"Serving on http://{local_ip}:{PORT}")

# Custom request handler to serve .bin files with the content type  "application/octet-stream"
class CustomHandler(http.server.SimpleHTTPRequestHandler):
    def guess_type(self, path):
        base, ext = os.path.splitext(path)
        print("base: ",base,"ext: ",ext)
        if ext == ".bin":
            return "application/octet-stream"
        return super().guess_type(path)

# Run a local web server serving files from WEBROOT
os.chdir(WEBROOT)
Handler = CustomHandler

class CustomTCPServer(socketserver.TCPServer):
    allow_reuse_address = True

    def server_bind(self):
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        super().server_bind()

with CustomTCPServer(("", PORT), Handler) as httpd:
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down server...")
        httpd.server_close()

# Clean up WEBROOT directory on exit
shutil.rmtree(WEBROOT)



