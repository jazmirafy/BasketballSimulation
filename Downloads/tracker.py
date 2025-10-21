import socket
import json
import sys
import pyrealsense2 as rs
import numpy as np

# ------------------- CAMERA SETUP -------------------
pipe = rs.pipeline()
cfg = rs.config()

# Enable streams supported by D455f
cfg.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
cfg.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)

# ----------------------------------------------------
try:
    IPs = sys.argv[1:]
except IndexError:
    IPs = ["127.0.0.1"]

print("RealSense D455f sending to", IPs)
TCP_PORT = 54321
pos_scale = 100.0

# Send initial metadata message to receivers
for IP in IPs:
    MESSAGE = '{"MHTrack":[{"Type":"CameraSubject"},{"FieldOfView":"true","AspectRatio":"true","FocalLength":"true","ProjectionMode":"false"}]}'
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect((IP, TCP_PORT))
    s.send(MESSAGE.encode('utf-8'))
    s.close()

# Start streaming
pipe.start(cfg)
align_to = rs.stream.color
align = rs.align(align_to)

try:
    while True:
        frames = pipe.wait_for_frames()
        aligned = align.process(frames)
        depth_frame = aligned.get_depth_frame()
        color_frame = aligned.get_color_frame()
        if not depth_frame or not color_frame:
            continue

        # Compute mean depth in meters (avoid 0 = invalid)
        depth_image = np.asanyarray(depth_frame.get_data())
        valid = depth_image[depth_image > 0]
        mean_depth = float(np.mean(valid)) if valid.size else 0.0

        # Package “fake pose” using mean depth
        z_pos = mean_depth  # meters
        MESSAGE = '{{"MHTrack":[{{"Type":"CameraAnimation"}},{{"Location":[0,0,{:.3f}],"Rotation":[0,0,0,1],"Scale":[1,1,1]}}]}}'.format(
            pos_scale * z_pos
        )

        for IP in IPs:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect((IP, TCP_PORT))
            s.send(MESSAGE.encode("utf-8"))
            s.close()

except KeyboardInterrupt:
    print("Stopping stream...")
finally:
    pipe.stop()
