/**
 * @example test_aidk.cpp
 * @brief test of AIDK client
 *
 * @copyright Copyright (C) 2023 Flexiv Ltd. All Rights Reserved.
 */

#include "flexiv/ai/aidk.hpp"
#include <ctime>
#include <fstream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <typeinfo>
using json = nlohmann::json;

bool load_config(json &js, std::string file_path,
                 std::vector<double> &camera_pose,
                 std::vector<double> &camera_intrinsic,
                 std::vector<double> &tcp_pose, std::vector<double> &tcp_force)
{
    std::cout << "Config File: " << file_path << std::endl;
    std::ifstream file(file_path);
    file >> js;
    camera_pose = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0};
    camera_intrinsic = {0, 0, 0, 0, 0, 0};
    tcp_pose = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0};
    tcp_force = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    if (js["command"]["camera_pose"].type() != nlohmann::json::value_t::null) {
        camera_pose = js["command"]["camera_pose"].get<std::vector<double>>();
    }
    if (js["command"]["camera_intrinsic"].type() !=
        nlohmann::json::value_t::null) {
        camera_intrinsic =
            js["command"]["camera_intrinsic"].get<std::vector<double>>();
    }
    if (js["command"]["tcp_pose"].type() != nlohmann::json::value_t::null) {
        tcp_pose = js["command"]["tcp_pose"].get<std::vector<double>>();
    }
    if (js["command"]["tcp_force"].type() != nlohmann::json::value_t::null) {
        tcp_force = js["command"]["tcp_force"].get<std::vector<double>>();
    }
    return true;
}

int main(int argc, char **argv)
{
    // init AIDK
    flexiv::ai::AIDKClient client(argv[1], 10);

    // load config file
    json js;
    std::vector<double> camera_pose;
    std::vector<double> camera_intrinsic;
    std::vector<double> tcp_pose;
    std::vector<double> tcp_force;
    load_config(js, argv[2], camera_pose, camera_intrinsic, tcp_pose,
                tcp_force);

    // AI state check
    while (!client.is_ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    flexiv::ai::AIStatus ai_status = client.get_current_state();
    std::cout << "current state code: " << ai_status.status_code << std::endl;
    std::cout << "current state name: " << ai_status.status_name << std::endl;
    std::cout << "current state message: " << ai_status.status_message
              << std::endl;
    std::cout << std::endl;

    // detect
    std::string arg2_str(argv[3]);
    auto total_num = std::stoi(arg2_str);

    for (auto idx = 0; idx < total_num; idx++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto tic = std::chrono::system_clock::now();

        // ai >= v2.10.0
        bool state;

        std::string rgb_path = js["command"]["rgb_path"];
        cv::Mat rgb_mat = cv::imread(rgb_path);
        std::string rgb_encoder = rgb_path.substr(rgb_path.find_last_of('.'));
        std::vector<u_char> rgb_buf;
        rgb_buf.resize(50 * 1024 * 1024);
        cv::imencode(rgb_encoder, rgb_mat, rgb_buf);

        std::string depth_path = js["command"]["depth_path"];
        cv::Mat depth_mat = cv::imread(depth_path, cv::IMREAD_ANYDEPTH);
        std::string depth_encoder =
            depth_path.substr(depth_path.find_last_of('.'));
        std::vector<u_char> depth_buf;
        depth_buf.resize(50 * 1024 * 1024);
        cv::imencode(depth_encoder, depth_mat, depth_buf);

        // check rgb and depth have same size
        if (rgb_mat.rows != depth_mat.rows || rgb_mat.cols != depth_mat.cols) {
            throw std::runtime_error("Error: rgb and depth shape mismatch!");
        }

        state = client.detect_with_image(
            js["command"]["obj_name"], js["command"]["camera_id"],
            js["command"]["coordinate_id"], camera_pose, camera_intrinsic,
            tcp_pose, tcp_force, rgb_buf, depth_buf, js["command"]["custom"]);

        // print result
        auto toc = std::chrono::system_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic);
        double infer_time = duration.count();
        std::cout << "detect " << idx << ": " << infer_time << " ms, "
                  << 1000 / infer_time << " Hz, " << std::endl;

        std::cout << "state: " << state << std::endl;

        std::cout << "current detected object names: ";
        auto obj_names_size = client.get_detected_obj_names().size();
        std::vector<std::string> obj_names = client.get_detected_obj_names();
        for (size_t i = 0; i < obj_names_size; i++) {
            std::cout << obj_names[i] << " ";
        }
        std::cout << " ";

        std::cout << "current detected object nums: ";
        auto obj_nums_size = client.get_detected_obj_nums().size();
        std::vector<int> obj_nums = client.get_detected_obj_nums();
        for (size_t i = 0; i < obj_nums_size; i++) {
            std::cout << obj_nums[i] << " ";
        }
        std::cout << std::endl;

        for (size_t i = 0; i < js["keys"].size(); i++) {
            std::cout << "key: " << js["keys"][i] << std::endl;
            std::vector<flexiv::ai::Result> result;
            bool parse_state = client.parse_result(js["command"]["obj_name"],
                                                   js["keys"][i], -1, result);

            // parse detected timestamp
            time_t timestamp = client.get_detected_time();
            struct tm *timeinfo;
            char buffer[80];
            timeinfo = std::gmtime(&timestamp);
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
            std::cout << "detected time stamp: " << buffer << std::endl;

            if (!parse_state) {
                std::cout << "Parse result error!!!" << std::endl;
                continue;
            }
            auto result_size = result.size();
            if ((js["keys"][i] == "bbox") | (js["keys"][i] == "keypoints") |
                (js["keys"][i] == "positions") |
                (js["keys"][i] == "obj_pose")) {
                for (size_t num = 0; num < result_size; num++) {
                    for (size_t j = 0; j < result[num].vect.size(); j++) {
                        for (size_t k = 0; k < result[num].vect[j].size();
                             k++) {
                            std::cout << result[num].vect[j][k] << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            } else if (js["keys"][i] == "valid") {
                for (size_t num = 0; num < result_size; num++) {
                    std::cout << result[num].valid << std::endl;
                }
            } else if (js["keys"][i] == "double_value") {
                for (size_t num = 0; num < result_size; num++) {
                    std::cout << result[num].double_value << std::endl;
                }
            } else if (js["keys"][i] == "int_value") {
                for (size_t num = 0; num < result_size; num++) {
                    std::cout << result[num].int_value << std::endl;
                }
            } else if (js["keys"][i] == "name") {
                for (size_t num = 0; num < result_size; num++) {
                    std::cout << result[num].name << std::endl;
                }
            }
        }
    }

    return 0;
}
