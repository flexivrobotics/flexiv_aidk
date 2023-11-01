/**
 * @file aidk.hpp
 * @brief declaration of AIDKClient member functions

 * @copyright Copyright (C) 2023 Flexiv Ltd. All Rights Reserved.
 */

#pragma once

#include <memory>

#include "flexiv/ai/defs.hpp"

namespace flexiv {
namespace ai {

class AIDKImpl;

class AIDKClient
{
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
     * @param obj_name object name.
     * @param camera_id camera id to use.
     * @param coordinate_id result coordinate space id, 0 for world space, 1 for
     * camera space.
     * @param camera_pose camera pose, can be list/ndarray, 7D or 4x4.
     * @param tcp_pose optionally used. Robot tcp pose [x, y, z, qw, qx, qy, qz]
     * @param tcp_force optionally used. Robot tcp force & wrench. [x, y, z, wx,
     * wy, wx]
     * @param command command name, should always be "CUSTOM".
     * @param custom custom command.
     * @return success or not of detection request.
     */
    bool detect(const std::string obj_name, const std::string camera_id,
                const int coordinate_id = 1,
                const std::vector<double> &camera_pose = {0.0, 0.0, 0.0, 1.0,
                                                          0.0, 0.0, 0.0},
                const std::vector<double> &tcp_pose = {0.0, 0.0, 0.0, 1.0, 0.0,
                                                       0.0, 0.0},
                const std::vector<double> &tcp_force = {0.0, 0.0, 0.0, 0.0, 0.0,
                                                        0.0},
                const std::string command = "CUSTOM",
                const std::string custom = "");

    /**
     * @brief Detect request with image input.
     *
     * @param obj_name object name.
     * @param camera_id camera id to use.
     * @param coordinate_id result coordinate space id, 0 for world space, 1 for
     * camera space.
     * @param camera_pose camera pose, can be list/ndarray, 7D or 4x4.
     * @param camera_intrinsic camera intrinsic, vector of width, height, ppx,
     * ppy, fx, fy.
     * @param tcp_pose optionally used. Robot tcp pose [x, y, z, qw, qx, qy, qz]
     * @param tcp_force optionally used. Robot tcp force & wrench. [x, y, z, wx,
     * wy, wx]
     * @param rgb_input vector of uchar of encoded rgb image.
     * @param depth_input vector of uchar of encoded depth image.
     * @param custom custom command.
     * @return success or not of detection request.
     */
    bool detect_with_image(
        const std::string obj_name, const std::string camera_id,
        const int coordinate_id = 1,
        const std::vector<double> &camera_pose = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                                                  0.0},
        const std::vector<double> &camera_intrinsic = {0.0, 0.0, 0.0, 0.0, 0.0,
                                                       0.0},
        const std::vector<double> &tcp_pose = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                                               0.0},
        const std::vector<double> &tcp_force = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        const std::vector<u_char> &rgb_input = std::vector<u_char>(),
        const std::vector<u_char> &depth_input = std::vector<u_char>(),
        const std::string custom = "");

    /**
     * @brief Detect request V1x.
     *
     * @param command direct command name.
     * @param camera_pose camera pose, can be list/ndarray, 7D or 4x4.
     * @param tcp_pose optionally used. Robot tcp pose [x, y, z, qw, qx, qy, qz]
     * @param tcp_force optionally used. Robot tcp force & wrench. [x, y, z, wx,
     * wy, wx]
     * @return success or not of detection request.
     */
    bool detect_v1x(const std::string command,
                    const std::vector<double> &camera_pose = {0.0, 0.0, 0.0,
                                                              1.0, 0.0, 0.0,
                                                              0.0},
                    const std::vector<double> &tcp_pose = {0.0, 0.0, 0.0, 1.0,
                                                           0.0, 0.0, 0.0},
                    const std::vector<double> &tcp_force = {0.0, 0.0, 0.0, 0.0,
                                                            0.0, 0.0});

    /**
     * @brief Get timestamp of detect request.
     *
     * @return timestamp in seconds.
     */
    uint64_t get_detected_time() const noexcept;

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
     * @brief Function to get all current detect object names.
     *
     * @return vector of object names.
     */
    std::vector<std::string> get_detected_obj_names() const noexcept;

    /**
     * @brief Function to get all current detect object nums.
     *
     * @return vector of object nums.
     */
    std::vector<int> get_detected_obj_nums() const noexcept;

    /**
     * @brief Function to get detected object number based of object name.
     *
     * @param obj_name string of object name.
     * @return num.
     */
    int get_detected_obj_num(const std::string &obj_name) const noexcept;

    /**
     * @brief Function to parse detection result.
     *
     * @param obj_name string of object name.
     * @param key string of result key, one of SUPPORTED_KEYS.
     * @param index indexed data or all data, -1 means get all data.
     * @param result vector of struct Result
     * @return true/false.
     */
    bool parse_result(const std::string &obj_name, const std::string &key,
                      int index, std::vector<Result> &result) noexcept;

    /**
     * @brief Function to get camera intrinsic.
     *
     * @return vector of fx,fy,cx,cy,depth_scale.
     */
    std::vector<float> get_camera_intrinsic() const noexcept;

    /**
     * @brief Function to get runtime info.
     *
     * @return string of working directory, string of binary path, vector of
     * string of supported paths..
     */
    std::tuple<std::string, std::string, std::vector<std::string>>
    get_runtime_info() const noexcept;

    /**
     * @brief List file and dir list under remote directory.
     *
     * @param remote_dir path string of remote dir.
     * @return tuple of string list of file names and string list of folder
     * names.
     */
    std::tuple<std::vector<std::string>, std::vector<std::string>>
    list_remote_files(const std::string remote_dir) const noexcept;

    /**
     * @brief Function to get remote file info.
     *
     * @param remote_file_path string of remote file path.
     * @return tuple of file modification time and file size.
     */
    std::tuple<uint64_t, uint32_t>
    get_file_info(const std::string remote_file_path) const;

    /**
     * @brief Function to send local file to remote file path.
     *
     * @param local_file_path string of local file path.
     * @param remote_file_path string, must be absolute/relative path rather
     * than single file.
     * @return bool.
     */
    bool send_file(const std::string local_file_path,
                   const std::string remote_file_path) const;

    /**
     * @brief Receive remote file to local file path.
     *
     * @param remote_file_path file path string of remote file path.
     * @param local_file_path string of local file path.
     * @return bool.
     */
    bool receive_file(const std::string remote_file_path,
                      const std::string local_file_path) const noexcept;

    /**
     * @brief Copy local folder to remote folder.
     *
     * @param local_dir string of local directory.
     * @param remote_dir string of remote directory.
     */
    void send_folder(const std::string local_dir,
                     const std::string remote_dir) const;

    /**
     * @brief Copy remote folder to local folder.
     *
     * @param local_dir string of local directory
     * @param remote_dir string of remote directory
     */
    void receive_folder(const std::string remote_dir,
                        const std::string local_dir) const noexcept;

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

    /**
     * @brief Get settable variables.
     *
     * @return map of variable name and value.
     */
    std::unordered_map<std::string, value_variant>
    get_direct_setting_variables();

    /**
     * @brief Set variable.
     *
     * @param vars map of variable name and value
     * @return struct of error code and message.
     */
    Response set_direct_setting_variables(
        std::unordered_map<std::string, value_variant> &vars);

private:
    std::unique_ptr<AIDKImpl> pimpl;
};

} /* namespace ai */
} /* namespace flexiv */
