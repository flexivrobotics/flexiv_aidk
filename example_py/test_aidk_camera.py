"""Test script for aidk get images.

Author: Flexiv
"""

import argparse
import logging
import struct
import sys

import cv2
import numpy as np

logging.basicConfig(level=logging.INFO)

# Import Flexiv AIDK Python library
sys.path.insert(0, "../lib_py/")
import flexivaidk as aidk


def main(
    ip="127.0.0.1",
    camera_id="CAMID_0",
    total_num=30,
    detect_timeout=10,
):
    client = aidk.AIDKClient(ip, detect_timeout)

    # get camera intrinsics
    state, intrinsic = client.get_camera_intrinsics(camera_id)
    logging.info(
        "width: {}, height: {}, ppx: {}, ppy: {}, fx: {}, fy: {}, depth scale: {}, serial number: {}, coeffs: {}".format(
            intrinsic.width,
            intrinsic.height,
            intrinsic.ppx,
            intrinsic.ppy,
            intrinsic.fx,
            intrinsic.fy,
            intrinsic.depth_scale,
            intrinsic.serial_number,
            intrinsic.coeffs,
        )
    )

    # get images
    for i in range(total_num):
        state, frame = client.get_images(camera_id)
        width = frame.width
        height = frame.height

        # process color image
        color = frame.color
        channel = int(len(color) / width / height)
        image_shape = [height, width, channel]
        rgb = np.reshape(color, image_shape)
        cv2.imwrite("color_{}.png".format(i), rgb)

        # process depth image
        depth_list = frame.depth
        element_size = int(len(depth_list) / width / height)
        depth_int8_bytes = bytes(depth_list)
        if element_size == 2:
            dtype = np.uint16
            depth_int16_list = struct.unpack(
                "h" * (len(depth_list) // 2), depth_int8_bytes
            )
            depth_array = np.array(depth_int16_list).astype(dtype)
            depth = depth_array.reshape(height, width).astype(dtype)
        elif element_size == 4:
            dtype = np.float
            depth_float_list = struct.unpack(
                "f" * (len(depth_list) // 4), depth_int8_bytes
            )
            depth_array = np.array(depth_float_list).astype(dtype)
            depth = depth_array.reshape(height, width).astype(dtype)
        elif element_size == 8:
            dtype = np.double
            depth_double_list = struct.unpack(
                "d" * (len(depth_list) // 8), depth_int8_bytes
            )
            depth_array = np.array(depth_double_list).astype(dtype=dtype)
            depth = depth_array.reshape(height, width).astype(dtype)
        else:
            raise TypeError(
                "The element type of depth image is incorrect from rpc server."
            )
        cv2.imwrite("depth_{}.png".format(i), depth.astype(dtype))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test Noema")
    parser.add_argument("--ip", help="Ip in a.b.c.d.", type=str, default="localhost")
    parser.add_argument("--camera-id", help="Camera ID", type=str, default="CAMID_0")
    parser.add_argument("--num", help="Number of get image times.", type=int, default=3)
    parser.add_argument(
        "--detect-timeout",
        help="Timeout of detect command request.",
        type=float,
        default=10,
    )

    args = parser.parse_args()

    logging.info(str(args))

    main(
        args.ip,
        camera_id=args.camera_id,
        total_num=args.num,
        detect_timeout=args.detect_timeout,
    )
