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

logging.basicConfig(level=logging.INFO)

# Import Flexiv AIDK Python library
sys.path.insert(0, "../lib_py/")
import flexivaidk as aidk


def main(
    project,
    config,
    keys,
    ip="127.0.0.1",
    total_num=30,
    enable_v1x=False,
    detect_timeout=10,
):
    client = aidk.AIDKClient(ip, detect_timeout)

    logging.info("all supported keys: %s", aidk.SUPPORTED_KEYS)

    # AI state check
    while not client.is_ready():
        time.sleep(0.5)
    ai_status = client.get_current_state()
    logging.info("Current state code: {}".format(ai_status.status_code))
    logging.info("Current state name: {}".format(ai_status.status_name))
    logging.info("Current state message: {}".format(ai_status.status_message))

    # detect
    for idx in range(total_num):
        time.sleep(1)
        tic = time.time()

        # ai >= v2.11.1
        if enable_v1x:
            state = client.detect_v1x(
                command=config["custom"],
                instruction_id=config["instruction_id"],
            )

        # ai >= v2.10.0
        else:
            state = client.detect(
                **config,
            )
        infer_time = time.time() - tic
        logging.info(
            "detect %d: %.1f ms, %.1f Hz, instruction %d",
            idx,
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

        for key in keys:
            logging.info("key: {}".format(key))
            parse_state, result_list = client.parse_result(config["obj_name"], key, -1)
            if not parse_state:
                logging.error("Parse result error!!!")
                continue
            else:
                if key in ["bbox", "keypoints", "positions", "obj_pose"]:
                    for result in result_list:
                        for vec in result.vect:
                            logging.info(vec)
                elif key in ["valid", "double_value", "int_value", "name"]:
                    for result in result_list:
                        logging.info(getattr(result, key))

        config["instruction_id"] += 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test Noema")
    parser.add_argument("--ip", help="Ip in a.b.c.d.", type=str, default="localhost")
    parser.add_argument(
        "--config",
        help="Config file path",
        default="config/TUTORIAL.json",
        type=str,
    )
    parser.add_argument("--num", help="Number of detect times.", type=int, default=3)
    parser.add_argument(
        "--detect-timeout",
        help="Timeout of detect command request.",
        type=float,
        default=10,
    )
    parser.add_argument(
        "--enable-v1x",
        "-v1x",
        dest="enable_v1x",
        help="Enable detect v1x.",
        action="store_true",
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
        args.ip,
        total_num=args.num,
        enable_v1x=args.enable_v1x,
        detect_timeout=args.detect_timeout,
    )
