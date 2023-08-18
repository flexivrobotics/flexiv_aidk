"""
Test script for flexivaidk.so
Author: Flexiv
"""

import argparse
import json
import logging
import os
import sys
import time

# Import Flexiv AIDK Python library
sys.path.insert(0, "../lib_py/")
import flexivaidk as aidk

try:
    import flexivrdk
except Exception:
    print("please put flexivrdk.so in current directory")

logging.basicConfig(level=logging.INFO)


def main(
    project,
    config,
    keys,
    ai_ip="127.0.0.1",
    robot_ip="192.168.2.100",
    local_ip="192.168.2.101",
    detect_timeout=10,
):
    # use rdk to get camera pose
    robot = flexivrdk.Robot(robot_ip, local_ip)
    while not robot.isConnected():
        time.sleep(0.001)
    robot_states = flexivrdk.RobotStates()
    robot.getRobotStates(robot_states)
    camera_pose = robot_states.camPose
    logging.info("camera pose: {}".format(camera_pose))

    # use aidk to get grasp pose
    client = aidk.AIDKClient(ai_ip, detect_timeout)

    # AI state check
    while not client.is_ready():
        time.sleep(0.5)
    ai_status = client.get_current_state()
    logging.info("Current state code: {}".format(ai_status.status_code))
    logging.info("Current state name: {}".format(ai_status.status_name))
    logging.info("Current state message: {}".format(ai_status.status_message))

    tic = time.time()
    config["camera_pose"] = camera_pose
    state = client.detect(
        **config,
    )
    infer_time = time.time() - tic
    logging.info(
        "detect: %.1f ms, %.1f Hz, instruction %d",
        1000 * infer_time,
        1 / infer_time,
        config["instruction_id"],
    )
    logging.info("state: %s", state)
    logging.info(
        "current detected object names: %s, current detected object nums: %s",
        client.get_detected_obj_names(),
        client.get_detected_obj_nums(),
    )

    key = "obj_pose"
    parse_state, result_list = client.parse_result(config["obj_name"], key, -1)
    grasp_pose = result_list[0].vect[0]
    logging.info("grasp pose: {}".format(grasp_pose))
    key = "double_value"
    parse_state, result_list = client.parse_result(config["obj_name"], key, -1)
    width = result_list[0].double_value
    logging.info("grasp width: {}".format(width))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test Noema")
    parser.add_argument(
        "--config",
        help="config file path",
        default="config/TUTORIAL.json",
        type=str,
    )
    parser.add_argument(
        "--detect-timeout",
        help="Timeout of detect command request.",
        type=float,
        default=10,
    )
    parser.add_argument("--ai-ip", help="Ip in a.b.c.d.", type=str, default="localhost")
    parser.add_argument(
        "--robot-ip", help="Ip in a.b.c.d.", type=str, default="192.168.2.100"
    )
    parser.add_argument(
        "--local-ip", help="Ip in a.b.c.d.", type=str, default="192.168.2.101"
    )

    args = parser.parse_args()

    assert os.path.exists(args.config), "Test config file %s not exist!" % args.config

    with open(args.config, "r") as f:
        config = json.load(f)

    logging.info(str(args))
    logging.info("Test config:\n%s", config)

    main(
        config["project"],
        config["command"],
        config["keys"],
        args.ai_ip,
        args.robot_ip,
        args.local_ip,
    )
