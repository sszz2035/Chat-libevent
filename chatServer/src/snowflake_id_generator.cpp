#include "snowflake_id_generator.h"
#include <thread>
#include <iostream>

SnowflakeIDGenerator::SnowflakeIDGenerator(int64_t data_center_id, int64_t machine_id) 
    : data_center_id_(data_center_id), machine_id_(machine_id) {
    
    if (data_center_id_ < 0 || data_center_id_ > MAX_DATA_CENTER_ID) {
        throw std::invalid_argument("Data center ID out of range");
    }
    if (machine_id_ < 0 || machine_id_ > MAX_MACHINE_ID) {
        throw std::invalid_argument("Machine ID out of range");
    }
}

uint64_t SnowflakeIDGenerator::generate_uid() {
    return generate_id();
}

uint64_t SnowflakeIDGenerator::generate_gid() {
    return generate_id();
}

void SnowflakeIDGenerator::set_data_center_id(int64_t data_center_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (data_center_id < 0 || data_center_id > MAX_DATA_CENTER_ID) {
        throw std::invalid_argument("Data center ID out of range");
    }
    data_center_id_ = data_center_id;
}

void SnowflakeIDGenerator::set_machine_id(int64_t machine_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (machine_id < 0 || machine_id > MAX_MACHINE_ID) {
        throw std::invalid_argument("Machine ID out of range");
    }
    machine_id_ = machine_id;
}

uint64_t SnowflakeIDGenerator::generate_id() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t current_timestamp = get_current_timestamp();
    
    if (current_timestamp < last_timestamp_) {
        throw std::runtime_error("Clock moved backwards");
    }
    
    if (current_timestamp == last_timestamp_) {
        sequence_ = (sequence_ + 1) & MAX_SEQUENCE;
        if (sequence_ == 0) {
            wait_next_millis(last_timestamp_);
            current_timestamp = get_current_timestamp();
        }
    } else {
        sequence_ = 0;
    }
    
    last_timestamp_ = current_timestamp;
    
    return (((current_timestamp - EPOCH) << TIMESTAMP_SHIFT)
           | (data_center_id_ << DATA_CENTER_ID_SHIFT)
           | (machine_id_ << MACHINE_ID_SHIFT)
           | sequence_)%100000000;
}

uint64_t SnowflakeIDGenerator::get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void SnowflakeIDGenerator::wait_next_millis(uint64_t last_timestamp) {
    uint64_t timestamp = get_current_timestamp();
    while (timestamp <= last_timestamp) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        timestamp = get_current_timestamp();
    }
}