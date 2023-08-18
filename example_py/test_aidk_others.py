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
    ip="127.0.0.1",
    detect_timeout=10,
    version="v3.1.0",
):
    client = aidk.AIDKClient(ip, detect_timeout)

    # AI state check
    while not client.is_ready():
        time.sleep(0.5)
    ai_status = client.get_current_state()
    logging.info("Current state code: {}".format(ai_status.status_code))
    logging.info("Current state name: {}".format(ai_status.status_name))
    logging.info("Current state message: {}".format(ai_status.status_message))

    if version >= "v3.1.0":
        """test variable setting."""
        # set variable
        logging.info(
            "Setable Variables: {}".format(client.get_direct_setting_variables())
        )
        res = client.set_direct_setting_variables(
            {
                "extern_scalar": 3.1415926,
                "extern_text": "world",
                "bool_check": True,
                "extern_int": 2,
            }
        )
        logging.info(res.error_code)
        logging.info(res.error_msg)
        logging.info(
            "Setable Variables: {}".format(client.get_direct_setting_variables())
        )

        # bad case tests
        res = client.set_direct_setting_variables({"extern_scalar": "abc"})
        logging.info(res.error_code)
        logging.info(res.error_msg)

        res = client.set_direct_setting_variables({"extern_invalid": 3.1415926})
        logging.info(res.error_code)
        logging.info(res.error_msg)

        res = client.set_direct_setting_variables({"extern_scalar": True})
        logging.info(res.error_code)
        logging.info(res.error_msg)

        res = client.set_direct_setting_variables({"extern_shape": 1})
        logging.info(res.error_code)
        logging.info(res.error_msg)

    """ test config related """
    if version >= "v2.11.1":
        state_code = client.save_configs()
        if not state_code:
            logging.error("save config failed!")

        state_code = client.reload_configs(project)
        if not state_code:
            logging.error("reload config failed for project: %s", project)
        logging.info("reload project: %s, state: %s", project, state_code)

        state_code = client.warmup()
        if not state_code:
            logging.error("warmup failed!")

    """test file transfer."""
    if version >= "v3.1.0":
        # get run time info
        working_dir, program_path, params = client.get_runtime_info()
        logging.info("working dir: {}".format(working_dir))
        logging.info("program path: {}".format(program_path))
        logging.info("support directory: {}".format(params))

        # get files and folders
        files, folders = client.list_remote_files("model/")
        logging.info("files: {}".format(files))
        logging.info("folders: {}".format(folders))

        # get file info
        mtime, size = client.get_file_info("model/upload.sh")
        logging.info("modify time: {}, size: {}".format(mtime, size))

        # file transfer
        client.send_file("./test_grasping_with_rdk.py", "model/test.py")
        client.receive_file("model/test.py", "test.py")
        client.remove_path("model/test.py")

        client.send_folder("../config", "model/FILE_TRANSFER")
        client.receive_folder("model/FILE_TRANSFER", "./FILE_TRANSFER")
        client.remove_path("model/FILE_TRANSFER")

        files, folders = client.list_remote_files("model/")
        logging.info("files: {}".format(files))
        logging.info("folders: {}".format(folders))


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
    parser.add_argument("--ip", help="Ip in a.b.c.d.", type=str, default="localhost")
    parser.add_argument(
        "--version", help="NoemaApp version in vx.x.x", type=str, default="v2.11.0"
    )

    args = parser.parse_args()

    assert os.path.exists(args.config), "Test config file %s not exist!" % args.config

    with open(args.config, "r") as f:
        config = json.load(f)

    logging.info(str(args))
    logging.info("Test config:\n%s", config)

    main(
        config["project"],
        args.ip,
        args.detect_timeout,
        args.version,
    )
