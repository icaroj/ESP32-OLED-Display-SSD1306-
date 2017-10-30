// Libs

#include "ssd1306.h"
#include "font8x8_basic.h"
#include "glcdfont.h"

// Functions

static void i2c_master_init();



esp_err_t oled_ssd1306_init() {

        i2c_master_init();

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        i2c_master_write_byte(cmd, (SSD1306_OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);

        // Initialization (page 64)
        // The next bytes are commands
        i2c_master_write_byte(cmd, COMMAND_MODE, ACK_CHECK_EN);

        // Mux Ratio
        i2c_master_write_byte(cmd, MULTIPLEX_RATIO, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0x3F, ACK_CHECK_EN);

        // Set display offset
        i2c_master_write_byte(cmd, DISPLAY_OFFSET, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

        // Set display line start
        i2c_master_write_byte(cmd, DISPLAY_LINE_START, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

        // Set Segment re-map
        i2c_master_write_byte(cmd, SEGMENT_REMAP, ACK_CHECK_EN);

        // Set COM output scan dir
        i2c_master_write_byte(cmd, COM_OUTPUT_SCAN_DIR, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

        // Set COM pins hardware config
        i2c_master_write_byte(cmd, COM_PINS_HARDWARE_CONFIG, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0x12, ACK_CHECK_EN);

        // Set contrast Control
        i2c_master_write_byte(cmd, CONTRAST_CONTROL, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0x7F, ACK_CHECK_EN);

        // Disable entire display ON
        i2c_master_write_byte(cmd, DISABLE_ENTIRE_DISPLAY, ACK_CHECK_EN);

        // Set normal display
        i2c_master_write_byte(cmd, NORMAL_DISPLAY, ACK_CHECK_EN);

        // Set OSC frequency
        i2c_master_write_byte(cmd, DISPLAY_CLK_RATIO, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0x80, ACK_CHECK_EN);

        // Enable charge pump regulator
        i2c_master_write_byte(cmd, CHARGE_PUMP_SET, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, 0x14, ACK_CHECK_EN);

        // Display on
        i2c_master_write_byte(cmd, DISPLAY_ON, ACK_CHECK_EN);

        // Stop bit
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);

        return ret;
}



void oled_ssd1306_test(void *ignore) {
        i2c_cmd_handle_t cmd;
        static uint8_t page = 0;
        static uint8_t color = 0x00;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, (SSD1306_OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);

        i2c_master_write_byte(cmd, SINGLE_COMMAND_MODE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, (PAGE_START_ADDR | (page++)), ACK_CHECK_EN);

	i2c_master_write_byte(cmd, DATA_MODE, ACK_CHECK_EN);
        for(uint8_t col = 0; col < 128; col++) {
	       i2c_master_write_byte(cmd, color, ACK_CHECK_EN);
        }
        // Stop bit
	i2c_master_stop(cmd);

        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        if(page == 8) page = 0;

	vTaskDelete(NULL);
}



void oled_ssd1306_clear(void *ignore) {
        i2c_cmd_handle_t cmd;
        for(uint8_t page = 0; page < 8; page++) {
        	cmd = i2c_cmd_link_create();
        	i2c_master_start(cmd);

        	i2c_master_write_byte(cmd, (SSD1306_OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);

                i2c_master_write_byte(cmd, SINGLE_COMMAND_MODE, ACK_CHECK_EN);
        	i2c_master_write_byte(cmd, (PAGE_START_ADDR | page), ACK_CHECK_EN);

        	i2c_master_write_byte(cmd, DATA_MODE, ACK_CHECK_EN);
                for(uint8_t col = 0; col < 128; col++)
        	       i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);

                // Stop bit
        	i2c_master_stop(cmd);

                i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
                i2c_cmd_link_delete(cmd);
        }
	vTaskDelete(NULL);
}



void oled_ssd1306_print(void *args) {
        char *text = (char*)args;
        // char text[] = "! ! ! ! ! ! ";
        
        uint8_t len = strlen(text);
        uint8_t page = 0, i = 0;
        uint16_t char_index = 0;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        i2c_master_write_byte(cmd, (SSD1306_OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);

        i2c_master_write_byte(cmd, COMMAND_MODE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, (PAGE_START_ADDR | page++), ACK_CHECK_EN);
        i2c_master_write_byte(cmd, LOWER_COL_START_ADDR, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, HIGHER_COL_START_ADDR, ACK_CHECK_EN);
        i2c_master_stop(cmd);

        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        for(i = 0; i < len; i++) {
                // Calculate the pos of the char in font array
                char_index = (text[i] <= 0) ? 0 : font_width * text[i];

                cmd = i2c_cmd_link_create();
                i2c_master_start(cmd);

                i2c_master_write_byte(cmd, (SSD1306_OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);

                if(text[i] == '\n') { // new page, reset cols
                        i2c_master_write_byte(cmd, COMMAND_MODE, ACK_CHECK_EN);
                        i2c_master_write_byte(cmd, (PAGE_START_ADDR | page++), ACK_CHECK_EN);
                        i2c_master_write_byte(cmd, LOWER_COL_START_ADDR, ACK_CHECK_EN);
                        i2c_master_write_byte(cmd, HIGHER_COL_START_ADDR, ACK_CHECK_EN);
                }

        	i2c_master_write_byte(cmd, DATA_MODE, ACK_CHECK_EN);

                for(uint8_t col = 0; col < font_width; col++){
        	        i2c_master_write_byte(cmd, font[char_index + col], ACK_CHECK_EN);
                }

               // Stop bit
               i2c_master_stop(cmd);

               i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
               i2c_cmd_link_delete(cmd);
        }
	vTaskDelete(NULL);
}



void oled_ssd1306_screensaver(void *ignore) {
        uint8_t square = 0xFF;
        i2c_cmd_handle_t cmd;

        // chess pattern
        for(uint8_t page = 0; page < 8; page++) {
        	cmd = i2c_cmd_link_create();
        	i2c_master_start(cmd);

        	i2c_master_write_byte(cmd, (SSD1306_OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);

                i2c_master_write_byte(cmd, SINGLE_COMMAND_MODE, ACK_CHECK_EN);
        	i2c_master_write_byte(cmd, (PAGE_START_ADDR | page), ACK_CHECK_EN);

        	i2c_master_write_byte(cmd, DATA_MODE, ACK_CHECK_EN);
                for(uint8_t col = 0; col < 128; col++){
                        square = (col % 7 == 0) ? ~square : square;
        	       i2c_master_write_byte(cmd, square, ACK_CHECK_EN);
                }
                // Stop bit
        	i2c_master_stop(cmd);

                i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
                i2c_cmd_link_delete(cmd);
        }

        // hor and vertical scrolling using graphic acceleration commands
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (SSD1306_OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, COMMAND_MODE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, DEACTIVATE_SCROLL, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, VERTICAL_AND_RIGHT_HOR_SCROLL, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, DUMMY_BYTE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, PAGE_START_ADDR, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, SIX_FRAMES_PER_SEC, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, (PAGE_START_ADDR | 0x07), ACK_CHECK_EN);
        i2c_master_write_byte(cmd, VERTICAL_OFFSET_ONE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, ACTIVATE_SCROLL, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        vTaskDelete(NULL);
}






static void i2c_master_init() {
        int i2c_master_port = I2C_MASTER_NUM;
        i2c_config_t conf;
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = I2C_MASTER_SDA_IO;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_io_num = I2C_MASTER_SCL_IO;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
        i2c_param_config(i2c_master_port, &conf);
        i2c_driver_install(i2c_master_port,
                           conf.mode,
                           I2C_MASTER_RX_BUF_DISABLE,
                           I2C_MASTER_TX_BUF_DISABLE,
                           0);
}
