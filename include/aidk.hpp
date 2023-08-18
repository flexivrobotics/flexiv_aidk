/**
* @file aidk.hpp
* @brief declaration of AIDKClient member functions
* @version 1.1

* @copyright Copyright (C) 2023 Flexiv Ltd. All Rights Reserved.
*/

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include <typeindex>
#include <unordered_map>
#include "variant.hpp"

namespace flexiv {
namespace aidk {

extern std::vector<std::string> SUPPORTED_KEYS;

// Data structure for result store
struct Result{
    bool valid;

    int int_value;

    double double_value;

    std::string name;

    std::vector<std::vector<double>> vect;
};

// Data structure for ai status
struct AIStatus
{
    int status_code=-1;

    std::string status_name;

    std::string status_message;
};

// Data structure for return response
struct Response
{
    int error_code=1;

    std::string error_msg;
};

// AIDKClient class
class AIDKClient {
    public:
        /**
        * @brief Constructor of client.
        *
        * @param ip string of AI Noema App ip.
        * @param request_timeout timeout of detect request(unit:second).
        */
        AIDKClient(const std::string ip, float request_timeout);

        /**
        * @brief Destructor of client.
        */
        ~AIDKClient();

        /**
        * @brief Check if AI edge is ready.
        *
        * @return true/false.
        */
        bool is_ready() const noexcept;

        /**
        * @brief Get current AI edge state code.
        *
        * @return AIStatus struct.
        */
        AIStatus get_current_state() const noexcept;

        /**
        * @brief Detect request.
        *
        * @param command command name.
        * @param obj_name object name.
        * @param camera_id camera id to use.
        * @param coordinate_id result coordinate id. 1 for world space.
        * @param camera_pose camera pose, can be list/ndarray, 7D or 4x4.
        * @param tcp_pose optionally used. Robot tcp pose [x, y, z, qw, qx, qy, qz]
        * @param tcp_force optionally used. Robot tcp force & wrench. [x, y, z, wx, wy, wx]
        * @param custom custom command.
        * @param instruction_id instruction id, each one should be unique and will be used for debug and data analysis.
        * @return detection result.
        */
        bool detect(
            const std::string command,
            const std::string obj_name,
            const std::string camera_id,
            const int coordinate_id = 1,
            const std::vector<double>& camera_pose={0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
            const std::vector<double>& tcp_pose={0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
            const std::vector<double>& tcp_force={0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            const std::string custom = "",
            int instruction_id = -1
        );

        /**
        * @brief Detect request V1x.
        *
        * @param command command name.
        * @param camera_pose camera pose, can be list/ndarray, 7D or 4x4.
        * @param tcp_pose optionally used. Robot tcp pose [x, y, z, qw, qx, qy, qz]
        * @param tcp_force optionally used. Robot tcp force & wrench. [x, y, z, wx, wy, wx]
        * @param instruction_id instruction id, each one should be unique and will be used for debug and data analysis.
        * @return detection result.
        */
        bool detect_v1x(
            const std::string command,
            const std::vector<double>& camera_pose={0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
            const std::vector<double>& tcp_pose={0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
            const std::vector<double>& tcp_force={0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            int instruction_id = -1
        );

        /**
        * @brief Reload project config.
        *
        * @param name project name.
        * @return status.
        */
        bool reload_configs(const std::string name) const noexcept;

        /**
        * @brief Save current project config.
        *
        * @return status.
        */
        bool save_configs() const noexcept;

        /**
        * @brief Warmup current project, if configured on NoemaEdge.
        *
        * @return status.
        */
        bool warmup() const noexcept;

        /**
        * @brief All direct commands.
        *
        * @return string vector of all direct commands.
        */
        std::vector<std::string> all_direct_command() const noexcept;

        /**
        * @brief Function to get all detected object names.
        * @return vector of object names.
        */
        std::vector<std::string> get_detected_obj_names() const noexcept;

        /**
        * @brief Function to get all detected object nums.
        * @return vector of object nums.
        */
        std::vector<int> get_detected_obj_nums() const noexcept;

        /**
        * @brief Function to get detected object number based of object name.
        * @param obj_name string of object name.
        * @return num.
        */
        int get_detected_obj_num(const std::string& obj_name) const noexcept;

        /**
        * @brief Function to parse detection result.
        * @param obj_name string of object name.
        * @param key string of result key, one of SUPPORTED_KEYS.
        * @param index  indexed data or all data. -1 means get all data.
        * @param result vector of struct Result
        * @return true/false.
        */
        bool parse_result(const std::string& obj_name, const std::string& key, int index, std::vector<Result>& result) const noexcept;

        /**
        * @brief Function to get camera intrinsic.
        * @return vector of fx,fy,cx,cy,depth_scale.
        */
        std::vector<float> get_camera_intrinsic() const noexcept;

        /**
        * @brief Function to get runtime info.
        *
        * @return tuple of string of working directory, string of binary path and string list of supported paths..
        */
        std::tuple<std::string, std::string, std::vector<std::string>> get_runtime_info() const noexcept;

        /**
        * @brief List file and dir list under remote directory.
        *
        * @param remote_dir path string of remote dir.
        * @return tuple of string list of file names and string list of folder names.
        */
        std::tuple<std::vector<std::string>, std::vector<std::string>> list_remote_files(const std::string remote_dir) const noexcept;

        /**
        * @brief Function to get remote file info.
        *
        * @param remote_file_path string of remote file path.
        * @return tuple of file modification time and file size.
        */
        std::tuple<uint64_t, uint32_t> get_file_info(const std::string remote_file_path) const noexcept;

        /**
        * @brief Function to send local file to remote file path.
        *
        * @param local_file_path string of local file path.
        * @param remote_file_path string, must be absolute/relative path rather than single file.
        * @return bool.
        */
        bool send_file(const std::string local_file_path, const std::string remote_file_path) const noexcept;

        /**
        * @brief Receive remote file to local file path.
        *
        * @param remote_file_path file path string of remote file path.
        * @param local_file_path string of local file path.
        * @return bool.
        */
        bool receive_file(const std::string remote_file_path, const std::string local_file_path) const noexcept;

        /**
        * @brief Copy local folder to remote folder.
        *
        * @param local_dir string of local directory.
        * @param remote_dir string of remote directory.
        * @return bool.
        */
        bool send_folder(const std::string local_dir, const std::string remote_dir) const noexcept;

        /**
        * @brief Copy remote folder to local folder.
        *
        * @param local_dir string of local directory
        * @param remote_dir string of remote directory
        * @return bool.
        */
        bool receive_folder(const std::string remote_dir, const std::string local_dir) const noexcept;

        /**
        * @brief Remove remote files or folders.
        *
        * @param remote_file_path path string of remote file or directory
        * @return bool.
        */
        bool remove_path(const std::string remote_file_path) const noexcept;

        /**
        * @brief Make directory in remote.
        *
        * @param remote_dir path string of remote dir
        * @return bool.
        */
        bool make_remote_directory(const std::string remote_dir) const noexcept;

        std::unordered_map<std::string, value_variant> get_direct_setting_variables();

        Response set_direct_setting_variables(std::unordered_map<std::string, value_variant>& vars);

    private:
        // ip address
        std::string m_ip;

        // instruction id
        int m_instruction_id = 0;

        // camera intrinsic parameter, fx, fy are focal length in horizontal x-axis and vertical y-axis in pixels
        // cx,cy are Optical center in horizontal x-axis and vertical y-axis in pixels
        float m_fx, m_fy, m_cx, m_cy, m_depth_scale;

        // camera id
        int m_cam_id;

        // timeout for detect request(s)
        float m_request_timeout;

        // receive ai status thread terminate flag
        bool m_terminate_flag;

        // AI status
        AIStatus m_ai_status;
};

} /* namespace aidk */
} /* namespace flexiv */
