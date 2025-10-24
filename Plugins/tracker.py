import socket
import json
import sys
import pyrealsense2 as rs
import numpy as np

# --- setup ---
try:
    IPs = sys.argv[1:]
except IndexError:
    IPs = ["127.0.0.1"]
print("Realsense D455f sending to", IPs)

TCP_PORT = 54321
pos_scale = 100  # to scale to your target system units
rot_scale = 1
starting_height = 1.0

# --- configure D455f streams ---
pipe = rs.pipeline()
cfg = rs.config()
cfg.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
cfg.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)

# --- send initial handshake message ---
for IP in IPs:
    MESSAGE = '{"MHTrack": [{"Type": "CameraSubject"}, {"FieldOfView": "true","AspectRatio": "true","FocalLength": "true","ProjectionMode": "false"}]}'
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect((IP, TCP_PORT))
    s.send(MESSAGE.encode("utf-8"))
    s.close()

# --- start streaming ---
pipe.start(cfg)
print("Pipeline started. Streaming from D455f...")

try:
    while True:
        frames = pipe.wait_for_frames()
        depth_frame = frames.get_depth_frame()
        color_frame = frames.get_color_frame()

        if not depth_frame or not color_frame:
            continue

        # get intrinsics
        depth_intrin = depth_frame.profile.as_video_stream_profile().intrinsics
        width, height = depth_intrin.width, depth_intrin.height
        cx, cy = width // 2, height // 2

        # depth at image center
        depth = depth_frame.get_distance(cx, cy)

        # project the center pixel into 3D space
        X, Y, Z = rs.rs2_deproject_pixel_to_point(depth_intrin, [cx, cy], depth)

        # add starting height offset
        Y += starting_height

        # scale for external system
        X_scaled = X * pos_scale
        Y_scaled = Y * pos_scale
        Z_scaled = Z * pos_scale

        # fake a quaternion rotation (no real pose data on D455f)
        rot = [0.0, 0.0, 0.0, 1.0]

        MESSAGE = (
            '{{"MHTrack": [{{"Type":"CameraAnimation"}},{{"Location":[{:.3f},{:.3f},{:.3f}],'
            '"Rotation":[{:.4f},{:.4f},{:.4f},{:.4f}],"Scale":[1,1,1]}}]}}'.format(
                X_scaled, Y_scaled, Z_scaled,
                rot_scale * rot[0],
                rot_scale * rot[1],
                rot_scale * rot[2],
                rot_scale * rot[3]
            )
        )

        for IP in IPs:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect((IP, TCP_PORT))
            s.send(MESSAGE.encode("utf-8"))
            s.close()

finally:
    pipe.stop()
    print("Pipeline stopped.")
