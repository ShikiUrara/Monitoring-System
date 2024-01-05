// #ifndef LORACOMM_H
// #define LORACOMM_H

// #include "Communication.h"
// #include <SPI.h>
// #include <LoRa.h>
// #include <heatshrink_encoder.h>
// #include <heatshrink_decoder.h>

// class LoRaComm : public Communication {
// private:
//     int ss;
//     int rst;
//     int dio0;
//     heatshrink_encoder hse;
//     heatshrink_decoder hsd;

// public:
//     // Khởi tạo đối tượng, thiết lập chân kết nối
//     LoRaComm(int ss, int rst, int dio0) : ss(ss), rst(rst), dio0(dio0) {
//         // Sử dụng các thông số chân kết nối
//         LoRa.setPins(ss, rst, dio0);
//     }

//     void begin(long frequency) override {
//         // Khởi tạo LoRa với tần số cung cấp
//         if (!LoRa.begin(frequency)) {
//             Serial.println("Starting LoRa failed!");
//             while (1);
//         }
//     }

// void send(String message) override {
//     // In kích thước dữ liệu chưa nén
//     Serial.print("Uncompressed Size: ");
//     Serial.println(message.length());

//     // Nén dữ liệu trước khi gửi
//     uint8_t compressedData[1024]; // Đặt kích thước bộ đệm nén ở đây
//     uint32_t compressedSize = sizeof(compressedData);
//     compress(message.c_str(), message.length(), compressedData, compressedSize);

//     // In kích thước dữ liệu đã nén
//     Serial.print("Compressed Size: ");
//     Serial.println(compressedSize);

//     // Gửi thông điệp qua LoRa
//     LoRa.beginPacket();
//     LoRa.write(compressedData, compressedSize);
//     LoRa.endPacket();
// }


//     String receive() override {
//         // Nhận thông điệp từ LoRa
//         String message = "";
//         int packetSize = LoRa.parsePacket();
//         if (packetSize) {
//             // Đọc dữ liệu từ LoRa
//             uint8_t receivedData[256]; // Đặt kích thước bộ đệm nhận ở đây
//             int bytesRead = LoRa.readBytes(receivedData, packetSize);

//             // Giải nén dữ liệu
//             uint8_t decompressedData[256]; // Đặt kích thước bộ đệm giải nén ở đây
//             uint32_t decompressedSize = sizeof(decompressedData);
//             decompress(receivedData, bytesRead, decompressedData, decompressedSize);

//             // Chuyển đổi dữ liệu giải nén thành String
//             for (int i = 0; i < decompressedSize; i++) {
//                 message += (char)decompressedData[i];
//             }
//         }
//         return message;
//     }

// private:
//     // Hàm nén dữ liệu
//     void compress(const char *input, uint32_t input_size, uint8_t *output, uint32_t &output_size) {
//         heatshrink_encoder_reset(&hse);
//         size_t count = 0;
//         uint32_t sunk = 0;
//         uint32_t polled = 0;

//         while (sunk < input_size) {
//             heatshrink_encoder_sink(&hse, const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&input[sunk])), input_size - sunk, &count);
//             sunk += count;
//             if (sunk == input_size) {
//                 heatshrink_encoder_finish(&hse);
//             }

//             HSE_poll_res pres;
//             do {
//                 pres = heatshrink_encoder_poll(&hse, &output[polled], output_size - polled, &count);
//                 polled += count;
//             } while (pres == HSER_POLL_MORE);
//         }

//         // Cập nhật kích thước đầu ra
//         output_size = polled;
//     }

//     // Hàm giải nén dữ liệu
//     void decompress(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t &output_size) {
//         heatshrink_decoder_reset(&hsd);
//         size_t count = 0;
//         uint32_t sunk = 0;
//         uint32_t polled = 0;

//         while (sunk < input_size) {
//             heatshrink_decoder_sink(&hsd, &input[sunk], input_size - sunk, &count);
//             sunk += count;
//             if (sunk == input_size) {
//                 heatshrink_decoder_finish(&hsd);
//             }

//             HSD_poll_res pres;
//             do {
//                 pres = heatshrink_decoder_poll(&hsd, &output[polled], output_size - polled, &count);
//                 polled += count;
//             } while (pres == HSDR_POLL_MORE);
//         }

//         // Cập nhật kích thước đầu ra
//         output_size = polled;
//     }
// };

// #endif
#ifndef LORACOMM_H
#define LORACOMM_H

#include "Communication.h"
#include <SPI.h>
#include <LoRa.h>
#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"

#define BUFFER_SIZE 300 // Thay đổi giá trị này nếu cần


class LoRaComm : public Communication {
private:
    int ss;
    int rst;
    int dio0;

    heatshrink_encoder hse;
    heatshrink_decoder hsd;

public:
    LoRaComm(int ss, int rst, int dio0) : ss(ss), rst(rst), dio0(dio0) {
        LoRa.setPins(ss, rst, dio0);
    }

    void begin(long frequency) override {
        if (!LoRa.begin(frequency)) {
          Serial.println("Starting LoRa failed!");
          while (1);
        }
    }

    void send(String message) override {
    uint32_t orig_size = message.length();
    uint32_t comp_size = BUFFER_SIZE; // assuming BUFFER_SIZE is defined appropriately
    uint8_t comp_buffer[BUFFER_SIZE];

    // Tạo một bản sao của dữ liệu để tránh lỗi const_cast
    char* message_copy = new char[orig_size + 1];
    strcpy(message_copy, message.c_str());

    // Nén dữ liệu
    compress(reinterpret_cast<uint8_t*>(message_copy), orig_size, comp_buffer, comp_size);

    // Serial.print("Original Size: ");
    // Serial.println(orig_size);
    // Serial.print("Compressed Size: ");
    // Serial.println(comp_size);

    // Gửi thông điệp nén qua LoRa
    LoRa.beginPacket();
    LoRa.write(comp_buffer, comp_size);
    LoRa.endPacket();

    // Giải phóng bộ nhớ
    delete[] message_copy;
}
    String receive() override {
        String message = "";
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            uint32_t comp_size = packetSize;
            uint8_t comp_buffer[BUFFER_SIZE];
            uint32_t decomp_size = BUFFER_SIZE;
            uint8_t decomp_buffer[BUFFER_SIZE];

            // Read the compressed message
            LoRa.readBytes(comp_buffer, packetSize);

            // Decompress the message
            decompress(comp_buffer, comp_size, decomp_buffer, decomp_size);

            // Convert decompressed data to a String
            for (int i = 0; i < decomp_size; i++) {
                message += (char)decomp_buffer[i];
            }
        }
        return message;
    }

private:
    void compress(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t &output_size) {
        heatshrink_encoder_reset(&hse);

        size_t   count  = 0;
        uint32_t sunk   = 0;
        uint32_t polled = 0;

        while (sunk < input_size) {
            heatshrink_encoder_sink(&hse, &input[sunk], input_size - sunk, &count);
            sunk += count;

            if (sunk == input_size) {
                heatshrink_encoder_finish(&hse);
            }

            HSE_poll_res pres;
            do {
                pres = heatshrink_encoder_poll(&hse, &output[polled], output_size - polled, &count);
                polled += count;
            } while (pres == HSER_POLL_MORE);

            if (sunk == input_size) {
                heatshrink_encoder_finish(&hse);
            }
        }

        output_size = polled;
    }

    void decompress(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t &output_size) {
        heatshrink_decoder_reset(&hsd);

        size_t   count  = 0;
        uint32_t sunk   = 0;
        uint32_t polled = 0;

        while (sunk < input_size) {
            heatshrink_decoder_sink(&hsd, &input[sunk], input_size - sunk, &count);
            sunk += count;

            if (sunk == input_size) {
                heatshrink_decoder_finish(&hsd);
            }

            HSD_poll_res pres;
            do {
                pres = heatshrink_decoder_poll(&hsd, &output[polled], output_size - polled, &count);
                polled += count;
            } while (pres == HSDR_POLL_MORE);

            if (sunk == input_size) {
                heatshrink_decoder_finish(&hsd);
            }

            if (polled > output_size) {
                // Handle error if needed
            }
        }

        output_size = polled;
    }
};

#endif


