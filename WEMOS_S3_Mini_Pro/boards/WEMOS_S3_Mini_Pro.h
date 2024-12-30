#define BOARD_HAS_RGB_LED
#define BOARD "WEMOS-S3-MINI-PRO"
#define LCD_WIDTH 128
#define LCD_HEIGHT 128
#define LCD_ROTATION 0
#define LCD_USES "GC9107"
#define pclk_hz 20000000
#define reset_gpio_num GFX_NOT_DEFINED 

// SPI
#define SPI_MOSI 38
#define SPI_MISO 39
#define SPI_CLK 40

// TFT
#define TFT_BL 33
#define TFT_DC 36
#define TFT_CS 35
#define TFT_RST 34

// I2C
#define I2C_SDA 12
#define I2C_SCL 11

// RGB_LED
#define RGB_POWER 7
#define RGB_DATA 8

// IR
#define PIN_IR 9

//  BUTTON
#define BUTTON0 0
#define BUTTON47 47
#define BUTTON48 48

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS ,SPI_CLK ,SPI_MOSI ,SPI_MISO );

// Arduino_GFX *gfx = new Arduino_GC9107(bus, TFT_RST, LCD_ROTATION /* rotation */, true /* IPS */);
// Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */);


