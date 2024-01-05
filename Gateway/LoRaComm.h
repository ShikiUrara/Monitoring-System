#ifndef LORACOMM_H
#define LORACOMM_H

#include "Communication.h"
#include <SPI.h>
#include <LoRa.h>
#include <heatshrink_encoder.h>
#include <heatshrink_decoder.h>

class LoRaComm : public Communication {
private:
    int ss;
    int rst;
    int dio0;
    heatshrink_encoder hse;
    heatshrink_decoder hsd;

public:
    // Khởi tạo đối tượng, thiết lập chân kết nối
    LoRaComm(int ss, int rst, int dio0) : ss(ss), rst(rst), dio0(dio0) {
        // Sử dụng các thông số chân kết nối
        LoRa.setPins(ss, rst, dio0);
    }

    void begin(long frequency) override {
        // Khởi tạo LoRa với tần số cung cấp
        if (!LoRa.begin(frequency)) {
            Serial.println("Starting LoRa failed!");
            while (1);
        }
    }

    void sendNode(String message) override {
        // Nén dữ liệu trước khi gửi
        uint8_t compressedData[1024]; // Đặt kích thước bộ đệm nén ở đây
        uint32_t compressedSize = sizeof(compressedData);
        compress(message.c_str(), message.length(), compressedData, compressedSize);

        // Gửi thông điệp qua LoRa
        LoRa.beginPacket();
        LoRa.write(compressedData, compressedSize);
        LoRa.endPacket();
    }

//    String receiveNode() override {
//        // Nhận thông điệp từ LoRa
//        String message = "";
//        int packetSize = LoRa.parsePacket();
//        if (packetSize) {
//            // Đọc dữ liệu từ LoRa
//            uint8_t receivedData[256]; // Đặt kích thước bộ đệm nhận ở đây
//            int bytesRead = LoRa.readBytes(receivedData, packetSize);
//
//            // Giải nén dữ liệu
//            uint8_t decompressedData[256]; // Đặt kích thước bộ đệm giải nén ở đây
//            uint32_t decompressedSize = sizeof(decompressedData);
//            decompress(receivedData, bytesRead, decompressedData, decompressedSize);
//
//            // Chuyển đổi dữ liệu giải nén thành String
//            for (int i = 0; i < decompressedSize; i++) {
//                message += (char)decompressedData[i];
//            }
//        }
//        return message;
//    }
  String receiveNode() override {
    // Nhận thông điệp từ LoRa
    String message = "";
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // Đọc dữ liệu từ LoRa
        uint8_t receivedData[1024]; // Đặt kích thước bộ đệm nhận ở đây
        int bytesRead = LoRa.readBytes(receivedData, packetSize);

        // In kích thước của dữ liệu chưa giải nén
        //Serial.print("Received Original Size: ");
        //Serial.println(bytesRead);

        // Giải nén dữ liệu
        uint8_t decompressedData[1024]; // Đặt kích thước bộ đệm giải nén ở đây
        uint32_t decompressedSize = sizeof(decompressedData);
        decompress(receivedData, bytesRead, decompressedData, decompressedSize);

        // In kích thước của dữ liệu sau khi giải nén
        //Serial.print("Decompressed Size: ");
        //Serial.println(decompressedSize);

        // Chuyển đổi dữ liệu giải nén thành String
        for (int i = 0; i < decompressedSize; i++) {
            message += (char)decompressedData[i];
        }
    }
    return message;
}



private:
    // Hàm nén dữ liệu
    void compress(const char *input, uint32_t input_size, uint8_t *output, uint32_t &output_size) {
        heatshrink_encoder_reset(&hse);
        size_t count = 0;
        uint32_t sunk = 0;
        uint32_t polled = 0;

        while (sunk < input_size) {
            heatshrink_encoder_sink(&hse, const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&input[sunk])), input_size - sunk, &count);
            sunk += count;
            if (sunk == input_size) {
                heatshrink_encoder_finish(&hse);
            }

            HSE_poll_res pres;
            do {
                pres = heatshrink_encoder_poll(&hse, &output[polled], output_size - polled, &count);
                polled += count;
            } while (pres == HSER_POLL_MORE);
        }

        // Cập nhật kích thước đầu ra
        output_size = polled;
    }

    // Hàm giải nén dữ liệu
    void decompress(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t &output_size) {
        heatshrink_decoder_reset(&hsd);
        size_t count = 0;
        uint32_t sunk = 0;
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
        }

        // Cập nhật kích thước đầu ra
        output_size = polled;
    }
};

#endif
