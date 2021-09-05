/* Simple firmware for a ESP32 displaying a static image on an EPaper Screen.
 *
 * Write an image into a header file using a 3...2...1...0 format per pixel,
 * for 4 bits color (16 colors - well, greys.) MSB first.  At 80 MHz, screen
 * clears execute in 1.075 seconds and images are drawn in 1.531 seconds.
 */

#include <Arduino.h>
#include "epd_driver.h"
#include <WiFi.h>

#include <HTTPClient.h> // https://github.com/amcewen/HttpClient

// #include <AddrList.h>
#include <lwip/dns.h>

#include "config.h"
// #include "pic1.h"
// #include "pic2.h"
// #include "pic3.h"

uint8_t *framebuffer;

uint8_t done;

HTTPClient http;

const uint32_t SCREEN_WIDTH = 960;
const uint32_t SCREEN_HEIGHT = 540;
const uint32_t SCREEN_PIXELS = SCREEN_WIDTH * SCREEN_HEIGHT;


void setup()
{
    Serial.begin(115200);

    epd_init();

    framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    done = 0;

    // connectToNetwork();

    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());
    // WiFi.disconnect(true);
    // Serial.println(WiFi.localIP());

    connectToNetwork();

}

void connectToNetwork() {
    uint8_t loopcount = 0;
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    WiFi.enableIpV6();

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print("Establishing connection to WiFi.. ");
        Serial.println(loopcount);
        loopcount++;
        if (loopcount > 250) {
            loopcount = 0;
        }
    }

    Serial.println("Connected to network");

    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("IPv6 Address: ");
    Serial.println(WiFi.localIPv6());

}

void disconnectNetwork() {
    WiFi.disconnect(true);
}

void update(uint32_t delay_ms)
{
    epd_poweron();
    epd_clear();
    volatile uint32_t t1 = millis();
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
    volatile uint32_t t2 = millis();
    Serial.printf("EPD draw took %dms.\n", t2 - t1);
    epd_poweroff();
    delay(delay_ms);
}

void loop()
{
    // Rect_t area = {
    //     .x = 0,
    //     .y = 0,
    //     .width = pic1_width,
    //     .height =  pic1_height
    // };

    uint8_t pic_data[SCREEN_PIXELS] = {};


    Serial.println("Hello there, booting up!");

    Serial.print("Grabbing URL: ");
    Serial.println(IMAGE_URL);
    http.begin(IMAGE_URL);
    int httpCode = http.GET();
    if (httpCode > 0) { //Check for the returning code

        WiFiClient stream = http.getStream();
        Serial.println("Reading bytes");
        stream.readBytes(pic_data, SCREEN_PIXELS);
        Serial.println("Done!");

        // String payload = http.getString();
        // Serial.println(httpCode);
        // Serial.println(payload);

    }

    else {
        Serial.println("Error on HTTP request, restarting network");
        disconnectNetwork();
        connectToNetwork();
    }
    if (done == 0) {
        epd_poweron();
        epd_clear();
        // epd_draw_grayscale_image(area, (uint8_t *) pic1_data);
        epd_poweroff();

        done = 1;
    }
    Rect_t image = {
        .x = 0,
        .y = 0,
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT
    };
    Serial.println("Copying to framebuffer");
    epd_copy_to_framebuffer(image, (uint8_t *) pic_data, framebuffer);
    Serial.println("Updating screen...");
    update(REFRESH_DELAY);
    Serial.println("Done updating screen...");

    // update(3000);

    // Rect_t area2 = {
    //     .x = 0,
    //     .y = 0,
    //     .width = pic3_width,
    //     .height =  pic3_height
    // };
    // epd_copy_to_framebuffer(area2, (uint8_t *) pic3_data, framebuffer);

    // update(3000);
}
