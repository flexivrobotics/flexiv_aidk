/**
 * @example test_aidk_camera.cpp
 * @brief test of AIDK client to get images and camera intrinsics
 *
 * @copyright Copyright (C) 2023 Flexiv Ltd. All Rights Reserved.
 */

#include "flexiv/ai/aidk.hpp"

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

#include <ctime>
#include <fstream>
#include <typeinfo>

using json = nlohmann::json;

int main(int argc, char **argv)
{
    std::cout
        << "Example Usage: ./test_aidk_camera [address] [camera_id] [total_num]"
        << std::endl;

    // init AIDK
    flexiv::ai::AIDKClient client(argv[1], 10);
    std::string camera_id(argv[2]);

    // get camera intrinsics
    flexiv::ai::IntrinsicResult intrin;
    client.get_camera_intrinsics(camera_id, intrin);
    std::cout << "width: " << intrin.width << std::endl;
    std::cout << "height: " << intrin.height << std::endl;
    std::cout << "ppx: " << intrin.ppx << std::endl;
    std::cout << "ppy: " << intrin.ppy << std::endl;
    std::cout << "fx: " << intrin.fx << std::endl;
    std::cout << "fy: " << intrin.fy << std::endl;
    std::cout << "depth scale: " << intrin.depth_scale << std::endl;
    std::cout << "serial number: " << intrin.serial_number << std::endl;
    std::cout << "coeff: ";
    for (auto coeff = intrin.coeffs.begin(); coeff != intrin.coeffs.end();
         ++coeff) {
        std::cout << *coeff << " ";
    }
    std::cout << std::endl;

    // get images
    flexiv::ai::FrameResult r;
    bool state;
    auto total_num = std::stoi(argv[3]);
    for (auto idx = 0; idx < total_num; idx++) {
        state = client.get_images(camera_id, r);
        if (!state) {
            std::cerr << "Get image failed!" << std::endl;
            return 0;
        }
        int width = r.width;
        int height = r.height;

        // process rgb data
        cv::Mat color_img(height, width, CV_8UC3, r.color.data());

        // process depth data type
        int element_size = r.depth.size() / width / height;
        cv::Mat depth_img;
        if (element_size == 2) {
            depth_img = cv::Mat(height, width, CV_16UC1, r.depth.data());
        } else if (element_size == 4) {
            depth_img = cv::Mat(height, width, CV_32FC1, r.depth.data());
        } else if (element_size == 8) {
            depth_img = cv::Mat(height, width, CV_64FC1, r.depth.data());
        }

        cv::imwrite("color_" + std::to_string(idx) + ".png", color_img);
        cv::imwrite("depth_" + std::to_string(idx) + ".png", depth_img);
    }

    return 0;
}
