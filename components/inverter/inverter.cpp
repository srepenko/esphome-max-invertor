#include "inverter.h"
#include "esphome/core/log.h"
#include <string>
using namespace std;

static const char *TAG = "inverter";

namespace esphome {
namespace inverter {

void Inverter::setup() {}

int Inverter::readline(int readch, char *buffer, int len)
  {
    static int pos = 0;
    int rpos;

    if (readch > 0) {
      switch (readch) {
        case '\n': // Ignore new-lines
          break;
        case '\r': // Return on CR
          rpos = pos;
          pos = 0;  // Reset position index ready for next time
          return rpos;
        default:
          if (pos < len-1) {
            buffer[pos++] = readch;
            buffer[pos] = 0;
          }
      }
    }
    // No end of line has been found, so return -1.
    return -1;
  }

void Inverter::loop() {
     const int max_line_length = 80;
     static char buffer[max_line_length];
     while (available()) {
          if(readline(read(), buffer, max_line_length) > 0) {
               ESP_LOGD(TAG, buffer);
          }
     }
//     uint8_t byte;
//     while (this->available()) {
//          ESP_LOGD(TAG, "available");
//          byte = this->read();
//          if (this->device_protocol_id_) {
//               this->device_protocol_id_->publish_state(byte);
//          }
//          this->write_byte(byte);
//     }
}

void Inverter::update() {
     if (this->device_protocol_id_) {
          this->device_protocol_id_->publish_state(30);
     }
     //this->write_str("QPIGS\r");
     this->write_str("QP");
}

void Inverter::dump_config() {
     ESP_LOGCONFIG(TAG, "Inverter component");
}

void Inverter::add_polling_command_(const char *command, ENUMPollingCommand polling_command) {
     for (auto &used_polling_command : this->used_polling_commands_) {
          if (used_polling_command.length == strlen(command)) {
               uint8_t len = strlen(command);
               if (memcmp(used_polling_command.command, command, len) == 0) {
                    return;
               }
          }
          if (used_polling_command.length == 0) {
               size_t length = strlen(command) + 1;
               const char *beg = command;
               const char *end = command + length;
               used_polling_command.command = new uint8_t[length];  // NOLINT(cppcoreguidelines-owning-memory)
               size_t i = 0;
               for (; beg != end; ++beg, ++i) {
                    used_polling_command.command[i] = (uint8_t)(*beg);
               }
               used_polling_command.errors = 0;
               used_polling_command.identifier = polling_command;
               used_polling_command.length = length - 1;
               return;
          }
     }
}

uint8_t Inverter::check_incoming_length_(uint8_t length) {
  if (this->read_pos_ - 3 == length) {
    return 1;
  }
  return 0;
}

uint8_t Inverter::check_incoming_crc_() {
  uint16_t crc16;
  crc16 = cal_crc_half_(read_buffer_, read_pos_ - 3);
  ESP_LOGD(TAG, "checking crc on incoming message");
  if (((uint8_t)((crc16) >> 8)) == read_buffer_[read_pos_ - 3] &&
      ((uint8_t)((crc16) &0xff)) == read_buffer_[read_pos_ - 2]) {
    ESP_LOGD(TAG, "CRC OK");
    read_buffer_[read_pos_ - 1] = 0;
    read_buffer_[read_pos_ - 2] = 0;
    read_buffer_[read_pos_ - 3] = 0;
    return 1;
  }
  ESP_LOGD(TAG, "CRC NOK expected: %X %X but got: %X %X", ((uint8_t)((crc16) >> 8)), ((uint8_t)((crc16) &0xff)),
           read_buffer_[read_pos_ - 3], read_buffer_[read_pos_ - 2]);
  return 0;
}

uint16_t Inverter::cal_crc_half_(uint8_t *msg, uint8_t len) {
     uint16_t crc;

     uint8_t da;
     uint8_t *ptr;
     uint8_t b_crc_hign;
     uint8_t b_crc_low;

     uint16_t crc_ta[16] = {0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
                         0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef};

     ptr = msg;
     crc = 0;

     while (len-- != 0) {
          da = ((uint8_t)(crc >> 8)) >> 4;
          crc <<= 4;
          crc ^= crc_ta[da ^ (*ptr >> 4)];
          da = ((uint8_t)(crc >> 8)) >> 4;
          crc <<= 4;
          crc ^= crc_ta[da ^ (*ptr & 0x0f)];
          ptr++;
     }

     b_crc_low = crc;
     b_crc_hign = (uint8_t)(crc >> 8);

     if (b_crc_low == 0x28 || b_crc_low == 0x0d || b_crc_low == 0x0a)
          b_crc_low++;
     if (b_crc_hign == 0x28 || b_crc_hign == 0x0d || b_crc_hign == 0x0a)
          b_crc_hign++;

     crc = ((uint16_t) b_crc_hign) << 8;
     crc += b_crc_low;
     return (crc);
}

}
}