#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/select/select.h"
//#include "esphome/components/pipsolar/select/pipsolar_select.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/time.h"


namespace esphome {
namespace inverter {
enum ENUMPollingCommand {
  POLLING_QPIRI = 0,
  POLLING_QPIGS = 1,
  POLLING_QMOD = 2,
  POLLING_QFLAG = 3,
  POLLING_QPIWS = 4,
  POLLING_QT = 5,
  POLLING_QMN = 6,
  POLLING_QBATCD = 7,
  POLLING_QPI = 8,
};
struct PollingCommand {
  uint8_t *command;
  uint16_t interval = 0;
  uint16_t last_run = 0;
  uint8_t length = 0;
  uint8_t errors;
  ENUMPollingCommand identifier;
};

struct Polling_Command {
  uint8_t *command;
  uint16_t interval;
};


#define INVERTER_ENTITY_(type, name, polling_command) \
    protected: \
        type *name##_{}; /* NOLINT */ \
\
    public: \
        void set_##name(type *name) { /* NOLINT */ \
            this->name##_ = name; \
            this->add_polling_command_(#polling_command, POLLING_##polling_command); \
        }

#define INVERTER_VALUED_ENTITY_(type, name, polling_command, value_type) \
    protected: \
        value_type value_##name##_; \
        INVERTER_ENTITY_(type, name, polling_command)

#define INVERTER_SENSOR(name, polling_command, value_type) \
    INVERTER_VALUED_ENTITY_(sensor::Sensor, name, polling_command, value_type)

class Inverter : public uart::UARTDevice, public PollingComponent {
    // QPI values
    INVERTER_SENSOR(device_protocol_id, QPI, int)
    void setup() override;
    void loop() override;
    void update() override;
    void dump_config() override;

    protected:
        static const size_t READ_BUFFER_LENGTH = 200;  // maximum supported answer length
        static const size_t COMMAND_QUEUE_LENGTH = 10;
        static const size_t COMMAND_TIMEOUT = 500;
        void add_polling_command_(const char *command, ENUMPollingCommand polling_command);
        void empty_uart_buffer_();
        uint8_t check_incoming_crc_();
        uint8_t check_incoming_length_(uint8_t length);
        uint16_t cal_crc_half_(uint8_t *msg, uint8_t len);
        uint8_t send_next_command_();
        void queue_command_(const char *command, uint8_t length);
        std::string command_queue_[COMMAND_QUEUE_LENGTH];
        uint8_t command_queue_position_ = 0;
        uint8_t read_buffer_[READ_BUFFER_LENGTH];
        size_t read_pos_{0};  
        uint32_t command_start_millis_ = 0;
        uint8_t state_;
        enum State {
            STATE_IDLE = 0,
            STATE_POLL = 1,
            STATE_COMMAND = 2,
            STATE_POLL_COMPLETE = 3,
            STATE_COMMAND_COMPLETE = 4,
            STATE_POLL_CHECKED = 5,
            STATE_POLL_DECODED = 6,
        };

        PollingCommand used_polling_commands_[15];
};
}
}