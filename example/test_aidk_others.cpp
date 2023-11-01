/**
 * @example test_aidk.cpp
 * @brief test of AIDK client
 *
 * @copyright Copyright (C) 2023 Flexiv Ltd. All Rights Reserved.
 */

#include "flexiv/ai/aidk.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <typeinfo>
using json = nlohmann::json;

bool load_config(json &js, std::string file_path,
                 std::vector<double> &camera_pose,
                 std::vector<double> &tcp_pose, std::vector<double> &tcp_force)
{
    std::cout << "Config File: " << file_path << std::endl;
    std::ifstream file(file_path);
    file >> js;
    camera_pose = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0};
    tcp_pose = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0};
    tcp_force = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    if (js["command"]["camera_pose"].type() != nlohmann::json::value_t::null) {
        camera_pose = js["command"]["camera_pose"].get<std::vector<double>>();
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
    std::vector<double> tcp_pose;
    std::vector<double> tcp_force;
    load_config(js, argv[2], camera_pose, tcp_pose, tcp_force);

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

    std::string version = argv[3];

    // set variable
    if (version >= "v3.1.0") {
        std::unordered_map<std::string, flexiv::ai::value_variant> vars =
            client.get_direct_setting_variables();
        std::cout << "Setable Variables: " << std::endl;
        for (auto &pair : vars) {
            std::cout << pair.first << ": ";
            if (pair.second.is<int>()) {
                std::cout << pair.second.get<int>() << std::endl;
            } else if (pair.second.is<double>()) {
                std::cout << pair.second.get<double>() << std::endl;
            } else if (pair.second.is<float>()) {
                std::cout << pair.second.get<float>() << std::endl;
            } else if (pair.second.is<bool>()) {
                std::cout << pair.second.get<bool>() << std::endl;
            } else if (pair.second.is<std::string>()) {
                std::cout << pair.second.get<std::string>() << std::endl;
            }
        }
        std::cout << std::endl;

        std::unordered_map<std::string, flexiv::ai::value_variant> set_vars;
        set_vars["extern_scalar"].set<float>(3.1415926f);
        set_vars["extern_text"].set<std::string>("world");
        set_vars["bool_check"].set<bool>(true);
        set_vars["extern_int"].set<int>(2);
        client.set_direct_setting_variables(set_vars);

        set_vars["extern_scalar"].set<double>(3.141592653589793);
        set_vars["extern_text"].set<std::string>("world");
        set_vars["bool_check"].set<bool>(true);
        set_vars["extern_int"].set<int>(2);
        client.set_direct_setting_variables(set_vars);

        vars = client.get_direct_setting_variables();
        std::cout << "Setable Variables: " << std::endl;
        for (auto &pair : vars) {
            std::cout << pair.first << ": ";
            if (pair.second.is<int>()) {
                std::cout << pair.second.get<int>() << std::endl;
            } else if (pair.second.is<double>()) {
                std::cout << pair.second.get<double>() << std::endl;
            } else if (pair.second.is<float>()) {
                std::cout << pair.second.get<float>() << std::endl;
            } else if (pair.second.is<bool>()) {
                std::cout << pair.second.get<bool>() << std::endl;
            } else if (pair.second.is<std::string>()) {
                std::cout << pair.second.get<std::string>() << std::endl;
            }
        }
        std::cout << std::endl;

        // bad case tests
        flexiv::ai::Response res;
        set_vars.clear();
        set_vars["extern_scalar"].set<std::string>("abc");
        res = client.set_direct_setting_variables(set_vars);
        std::cout << "error code: " << res.error_code << std::endl;
        std::cout << "error message: " << res.error_msg << std::endl;

        set_vars.clear();
        set_vars["extern_invalid"].set<float>(3.1415926);
        res = client.set_direct_setting_variables(set_vars);
        std::cout << "error code: " << res.error_code << std::endl;
        std::cout << "error message: " << res.error_msg << std::endl;

        set_vars.clear();
        set_vars["extern_scalar"].set<bool>(true);
        res = client.set_direct_setting_variables(set_vars);
        std::cout << "error code: " << res.error_code << std::endl;
        std::cout << "error message: " << res.error_msg << std::endl;

        set_vars.clear();
        set_vars["extern_shape"].set<int>(1);
        res = client.set_direct_setting_variables(set_vars);
        std::cout << "error code: " << res.error_code << std::endl;
        std::cout << "error message: " << res.error_msg << std::endl;
    }

    // reload and warmup
    if (version >= "v2.11.1") {
        bool status = client.save_configs();
        if (!status)
            std::cerr << "save config failed!" << std::endl;

        status = client.reload_configs(js["project"]);
        if (!status)
            std::cerr << "reload config failed for project: " << js["project"]
                      << std::endl;
        std::cout << "reload project: " << js["project"] << " state: " << status
                  << std::endl;

        status = client.warmup();
        if (!status)
            std::cerr << "warmup failed!" << std::endl;
    }

    // File transfer related
    if (version >= "v3.1.0") {
        // get AI run time info
        auto [working_dir, program_path, params] = client.get_runtime_info();
        std::cout << "working dir: " << working_dir << std::endl;
        std::cout << "program path: " << program_path << std::endl;
        std::cout << "suppot dir: ";
        for (auto param : params) {
            std::cout << param << " ";
        }
        std::cout << std::endl;

        // get files and folders
        std::vector<std::string> files, folders;
        std::tie(files, folders) = client.list_remote_files("model/");
        std::cout << "files: ";
        for (auto file : files) {
            std::cout << file << " ";
        }
        std::cout << std::endl;
        std::cout << "folders: ";
        for (auto folder : folders) {
            std::cout << folder << " ";
        }
        std::cout << std::endl;

        // get file info
        auto [mtime, size] = client.get_file_info("model/upload.sh");
        std::cout << "mtime: " << mtime << " size: " << size << std::endl;

        // file transfer
        client.send_file("../include/flexiv/ai/aidk.hpp", "model/test.hpp");
        client.receive_file("model/test.hpp", "test.hpp");
        client.remove_path("model/test.hpp");

        client.send_folder("../config", "model/FILE_TRANSFER");
        client.receive_folder("model/FILE_TRANSFER", "./FILE_TRANSFER");
        client.remove_path("model/FILE_TRANSFER");

        std::tie(files, folders) = client.list_remote_files("model/");
        std::cout << "files: ";
        for (auto file : files) {
            std::cout << file << " ";
        }
        std::cout << std::endl;
        std::cout << "folders: ";
        for (auto folder : folders) {
            std::cout << folder << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
