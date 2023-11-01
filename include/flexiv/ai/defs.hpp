/**
 * @file defs.hpp
 * @author Junfeng Ding (junfeng.ding@flexiv.com)
 * @brief declaration of common constants and function
 *
 * @copyright Copyright (C) 2023 Flexiv Ltd. All Rights Reserved.
 */

#pragma once
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace flexiv {
namespace ai {

// define available keys
extern std::vector<std::string> PROTO_DATA;
extern std::vector<std::string> SINGLE_DATA;
extern std::unordered_map<std::string, std::string> MAPPING;
extern std::vector<std::string> SUPPORTED_KEYS;

// define the AI command
extern std::unordered_map<std::string, int> AI_CMD;

extern std::unordered_map<std::string, std::string> ALIAS;

// define the AI state enum
enum AIState
{
    UNKNOWN = 0,
    ERROR,
    IDLE,
    POS3D,
    POSE6D,
    GRASP_POSE,
    KEYPOINT,
    BBOX,
    MULTIVIEW,
    CLASSIFY,
    SCENE,
    GET_INT_VALUE,
    GET_DOUBLE_VALUE,
    KEYPOINT3D,
    CUSTOM,

    FIRST = UNKNOWN,
    LAST = CUSTOM,
    NUM = LAST - FIRST + 1
};

// Data structure for object meta information
struct ObjMetaData
{
    ObjMetaData()
    {
        coordinate_id = 0;
        is_valid = true;
        double_value = 0.0;
        int_value = 0;
    }

    // left-top corner of the bbox [xmin, ymin] in the image coordinate [pixel]
    std::vector<int> bbox_min;

    // right-bottom corner of the bbox [xmax, ymax] in the image coordinate
    // [pixel]
    std::vector<int> bbox_max;

    // image feature (key point) position [u;v] in image coordinate [pixel]
    std::vector<std::vector<double>> img_pts;

    // image feature (key point) 3D position [x;y;z] in camera coordinate [m]
    std::vector<std::vector<double>> img_pts_pos;

    // coordinate_id defines which camera/global coordinate to use, object pose
    // in camera/global coordinate [m]
    std::vector<double> obj_pose;

    // coordinate_id defines which camera/global coordinate to use, grasp pose
    // in camera/global coordinate [m]
    std::vector<std::vector<double>> grasp_pose;

    // uncertainty of the object pose  6-dim values, in range [0,1], [x; y; z;
    // theta_x; theta_y; theta_z]
    std::vector<double> uncertainty;

    // coordinate system. 0 for world (global) coordinate. 1 for camera (local)
    // coordinate
    int coordinate_id;

    // flag: if meta data is valid, robot only process this instance when
    // is_valid is true
    bool is_valid;

    // custom data - DOUBLE
    double double_value;

    // custom data - INT
    double int_value;

    // object type distinguish by string name
    std::string name;
};

// Data structure for object information
struct ObjState
{
    ObjState()
    {
        obj_name = "";
        ai_index = 0;
    }

    // object name
    std::string obj_name;

    // time index in AI module
    uint32_t ai_index;

    // synchronized timestamp
    double synced_timestamp;

    // object meta data
    std::vector<ObjMetaData> obj_meta_data;
};

// transfer meta data to vector
std::vector<std::vector<double>> meta_to_vector(const ObjMetaData &obj,
                                                const std::string &key);

// calculate md5 of string
std::string calculate_string_md5(const std::string &input);

// Data structure for result store
struct Result
{
    bool valid;

    int int_value;

    double double_value;

    std::string name;

    std::vector<std::vector<double>> vect;
};

// Data structure for ai status
struct AIStatus
{
    int status_code = -1;

    std::string status_name;

    std::string status_message;
};

// Data structure for return response
struct Response
{
    int error_code = 1;

    std::string error_msg;
};

// Custom variant inplementation for different types
template <size_t arg1, size_t... others>
struct static_max;

template <size_t arg>
struct static_max<arg>
{
    static const size_t value = arg;
};

template <size_t arg1, size_t arg2, size_t... others>
struct static_max<arg1, arg2, others...>
{
    static const size_t value = arg1 >= arg2
                                    ? static_max<arg1, others...>::value
                                    : static_max<arg2, others...>::value;
};

template <typename... Ts>
struct variant_helper;

template <typename F, typename... Ts>
struct variant_helper<F, Ts...>
{
    inline static void destroy(size_t id, void *data)
    {
        if (id == typeid(F).hash_code())
            reinterpret_cast<F *>(data)->~F();
        else
            variant_helper<Ts...>::destroy(id, data);
    }

    inline static void move(size_t old_t, void *old_v, void *new_v)
    {
        if (old_t == typeid(F).hash_code())
            new (new_v) F(std::move(*reinterpret_cast<F *>(old_v)));
        else
            variant_helper<Ts...>::move(old_t, old_v, new_v);
    }

    inline static void copy(size_t old_t, const void *old_v, void *new_v)
    {
        if (old_t == typeid(F).hash_code())
            new (new_v) F(*reinterpret_cast<const F *>(old_v));
        else
            variant_helper<Ts...>::copy(old_t, old_v, new_v);
    }
};

template <>
struct variant_helper<>
{
    inline static void destroy(size_t id, void *data)
    {
        (void)id;
        (void)data;
    }

    inline static void move(size_t old_t, void *old_v, void *new_v)
    {
        (void)old_t;
        (void)old_v;
        (void)new_v;
    }

    inline static void copy(size_t old_t, const void *old_v, void *new_v)
    {
        (void)old_t;
        (void)old_v;
        (void)new_v;
    }
};

template <typename... Ts>
struct variant
{
private:
    static const size_t data_size = static_max<sizeof(Ts)...>::value;
    static const size_t data_align = static_max<alignof(Ts)...>::value;

    using data_t = typename std::aligned_storage<data_size, data_align>::type;

    using helper_t = variant_helper<Ts...>;

    static inline size_t invalid_type() { return typeid(void).hash_code(); }

    size_t type_id;
    data_t data;

public:
    variant()
    : type_id(invalid_type())
    {}

    variant(const variant<Ts...> &old)
    : type_id(old.type_id)
    {
        helper_t::copy(old.type_id, &old.data, &data);
    }

    variant(variant<Ts...> &&old)
    : type_id(old.type_id)
    {
        helper_t::move(old.type_id, &old.data, &data);
    }

    // Serves as both the move and the copy asignment operator.
    variant<Ts...> &operator=(variant<Ts...> old)
    {
        std::swap(type_id, old.type_id);
        std::swap(data, old.data);

        return *this;
    }

    template <typename T>
    bool is()
    {
        return (type_id == typeid(T).hash_code());
    }

    bool valid() { return (type_id != invalid_type()); }

    template <typename T, typename... Args>
    void set(Args &&... args)
    {
        // First we destroy the current contents
        helper_t::destroy(type_id, &data);
        new (&data) T(std::forward<Args>(args)...);
        type_id = typeid(T).hash_code();
    }

    template <typename T>
    T &get()
    {
        // It is a dynamic_cast-like behaviour
        if (type_id == typeid(T).hash_code())
            return *reinterpret_cast<T *>(&data);
        else
            throw std::bad_cast();
    }

    ~variant() { helper_t::destroy(type_id, &data); }
};

using value_variant = variant<int, double, float, bool, std::string>;

} /* namespace ai */
} /* namespace flexiv */
