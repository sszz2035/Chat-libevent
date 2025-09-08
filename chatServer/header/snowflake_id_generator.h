#ifndef SNOWFLAKE_ID_GENERATOR_H
#define SNOWFLAKE_ID_GENERATOR_H

#include <chrono>
#include <atomic>
#include <mutex>
#include <cstdint>
#include <stdexcept>

class SnowflakeIDGenerator {
public:
    SnowflakeIDGenerator(int64_t data_center_id, int64_t machine_id);
    
    uint64_t generate_uid();
    uint64_t generate_gid();
    
    void set_data_center_id(int64_t data_center_id);
    void set_machine_id(int64_t machine_id);
    
    int64_t get_data_center_id() const { return data_center_id_; }
    int64_t get_machine_id() const { return machine_id_; }

private:
    uint64_t generate_id();
    uint64_t get_current_timestamp();
    void wait_next_millis(uint64_t last_timestamp);

    static const int64_t EPOCH = 1609459200000L; // 2021-01-01 00:00:00
    static const int64_t SEQUENCE_BITS = 12L;
    static const int64_t MACHINE_ID_BITS = 5L;
    static const int64_t DATA_CENTER_ID_BITS = 5L;
    
    static const int64_t MAX_SEQUENCE = (1LL << SEQUENCE_BITS) - 1;
    static const int64_t MAX_MACHINE_ID = (1LL << MACHINE_ID_BITS) - 1;
    static const int64_t MAX_DATA_CENTER_ID = (1LL << DATA_CENTER_ID_BITS) - 1;
    
    static const int64_t MACHINE_ID_SHIFT = SEQUENCE_BITS;
    static const int64_t DATA_CENTER_ID_SHIFT = SEQUENCE_BITS + MACHINE_ID_BITS;
    static const int64_t TIMESTAMP_SHIFT = SEQUENCE_BITS + MACHINE_ID_BITS + DATA_CENTER_ID_BITS;

    int64_t data_center_id_;
    int64_t machine_id_;
    std::atomic<uint64_t> sequence_{0};
    std::atomic<uint64_t> last_timestamp_{0};
    std::mutex mutex_;
};

#endif