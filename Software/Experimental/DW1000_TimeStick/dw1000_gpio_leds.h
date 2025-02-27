
#ifndef _DW1000_GPIO_LEDS_H_
#define _DW1000_GPIO_LEDS_H_




#define GPIO_DEBUG 0

#define DECA_SETUP_BLINK_INTERVAL 500
#define DECA_LEDMODE_BLINK 0
#define DECA_LEDMODE_COUNTUP 1
#define DECA_LEDMODE_COUNTDOWN 2


uint8_t deca_led_mode = DECA_LEDMODE_COUNTUP; 
// the loop function runs over and over again forever
uint32_t deca_led_counter = 0;
uint8_t deca_led_shift_counter = 0;


uint64_t dw1000_read_reg(uint16_t reg, uint16_t subaddress, uint8_t nbytes) {

  byte buf[8];
  uint64_t to_return = 0;
#if GPIO_DEBUG
  Serial.print("DW1000 read reg 0x"); Serial.print(reg, HEX);
  Serial.print(" Subaddr 0x"); Serial.print(subaddress, HEX);
#endif
  if ( nbytes > 8 ) return 0;
  DW1000.readBytes(reg, subaddress, buf, nbytes);
  memcpy(&to_return, buf, nbytes);
#if GPIO_DEBUG
  Serial.print(" = 0x"); Serial.println(to_return, HEX);
#endif
  return to_return;
}

void dw1000_write_reg(uint16_t reg, uint16_t subaddress, uint64_t data, uint8_t nbytes) {  
  byte buf[8];

#if GPIO_DEBUG
  Serial.print("DW1000 write reg 0x"); Serial.print(reg, HEX);
  Serial.print(" Subaddr 0x"); Serial.print(subaddress, HEX);
#endif
  if ( nbytes > 8 ) return;
  memcpy(buf, &data, nbytes);
#if GPIO_DEBUG
  Serial.print(" 0x"); Serial.println(data, HEX);
#endif
  DW1000.writeBytes(reg, subaddress, buf, nbytes);
}

void dw1000_gpio_set_mode(uint8_t gpioNum, uint8_t mode) {
    uint32_t reg;
#if GPIO_DEBUG
    Serial.print("dw1000_gpio_set_mode gpio "); Serial.print(gpioNum);
    Serial.print(" mode 0x"); Serial.println(mode,HEX);
#endif
    reg = (uint32_t) dw1000_read_reg(GPIO_CTRL_ID, GPIO_MODE_OFFSET , sizeof(uint32_t));
    reg &= ~(0x3UL << (6+gpioNum*2));
#if GPIO_DEBUG
    Serial.print("First reg &= 0x"); Serial.println(reg, HEX);
#endif
    reg |= (mode << (6+gpioNum*2));
#if GPIO_DEBUG
    Serial.print("Second reg |= 0x"); Serial.println(reg, HEX);
#endif
    dw1000_write_reg(GPIO_CTRL_ID, GPIO_MODE_OFFSET , reg, sizeof(uint32_t));
}
uint8_t dw1000_gpio_get_mode(uint8_t gpioNum)
{
    uint32_t reg;
    reg = (uint32_t) dw1000_read_reg(GPIO_CTRL_ID, GPIO_MODE_OFFSET, sizeof(uint32_t));
    reg &= (0x3UL << (6+gpioNum*2));
    reg >>= (6+gpioNum*2);
    return (uint8_t)(reg&0x3);
}

void dw1000_gpio_set_direction(uint8_t gpioNum, uint8_t dir)
{
    uint32_t reg;
    uint8_t buf[GPIO_DIR_LEN];
    uint32_t command;
#if GPIO_DEBUG
    Serial.print("dw1000_gpio_set_direction gpio "); Serial.print(gpioNum);
    Serial.print(" dir "); Serial.println(dir);
#endif
    if (!(gpioNum < 9)) return;
    /* Activate GPIO Clock if not already active */
    reg = dw1000_read_reg(PMSC , PMSC_CTRL0_SUB , sizeof(uint32_t));
    if ((reg&PMSC_CTRL0_GPCE) == 0 || (reg&PMSC_CTRL0_GPRN) == 0) {
        dw1000_write_reg(PMSC, PMSC_CTRL0_SUB,
                         reg|PMSC_CTRL0_GPCE|PMSC_CTRL0_GPRN,
                         sizeof(uint32_t));
    }
    /* See GxM1-8 and GxP1-8 in dw1000_regs.h. Mask | Value */
    if (gpioNum < 4) { // gpio 0-3
        command = (1 << (gpioNum+4)) | (dir << gpioNum);
    } else if (gpioNum<8) { // gpio 4-7
        command = (1 << (gpioNum+8)) | (dir << (gpioNum+8-4));
    } else { // gpio 8
        command = (1 << (20)) | (dir << (16));
    }
    buf[0] = command & 0xff;
    buf[1] = (command >> 8) & 0xff;
    buf[2] = (command >> 16) & 0xff;
#if GPIO_DEBUG
    Serial.print("writeBytes reg 0x"); Serial.print(GPIO_CTRL_ID, HEX);
    Serial.print(" subaddress 0x"); Serial.print(GPIO_DIR_OFFSET, HEX);
    Serial.print(" len "); Serial.print(GPIO_DIR_LEN);
    Serial.print(" command 0x"); Serial.print(command, HEX);
    for ( int i = 0; i < GPIO_DIR_LEN; i++ ) {
      Serial.print(" 0x"); Serial.print(buf[i], HEX);
    }
    Serial.println("");
#endif
    DW1000.writeBytes(GPIO_CTRL_ID, GPIO_DIR_OFFSET, buf, GPIO_DIR_LEN);
}

uint8_t dw1000_gpio_get_direction(uint8_t gpioNum)
{
    uint8_t res;
    uint32_t reg;
    if (!(gpioNum < 9)) return 0;
    reg = dw1000_read_reg(GPIO_CTRL_ID, GPIO_DIR_OFFSET, sizeof(uint32_t));
    if (gpioNum < 4) {
        res = (uint8_t)(0x1 & (reg >> gpioNum));
    } else if (gpioNum<8) {
        res = (uint8_t)(0x1 & (reg >> (gpioNum+8)));
    } else {
        res = (uint8_t)(0x1 & (reg >> (gpioNum+12)));
    }
    return res;
}
void dw1000_gpio_set_value(uint8_t gpioNum, uint8_t value)
{
    uint8_t buf[GPIO_DOUT_LEN];
    uint32_t command;

    if (!(gpioNum < 9)) return;

    /* See GxM1-8 and GxP1-8 in dw1000_regs.h. Mask | Value */
    if (gpioNum < 4) { //gpio 0-3
        command = (1 << (gpioNum+4)) | (value << gpioNum);
    } else if (gpioNum<8) { //gpio 4-7
        command = (1 << (gpioNum+8)) | (value << (gpioNum+8-4));
    } else { // gpio 8
        command = (1 << (20)) | (value << (16));
    }
    buf[0] = command & 0xff;
    buf[1] = (command >> 8) & 0xff;
    buf[2] = (command >> 16) & 0xff;

#if GPIO_DEBUG
    Serial.print("dw1000_gpio_set_value reg 0x"); Serial.print(GPIO_CTRL_ID, HEX);
    Serial.print(" subaddress 0x"); Serial.print(GPIO_DOUT_OFFSET, HEX);
    Serial.print(" len "); Serial.print(GPIO_DOUT_LEN);
    Serial.print(" command 0x"); Serial.print(command, HEX);
    for ( int i = 0; i < GPIO_DOUT_LEN; i++ ) {
      Serial.print(" 0x"); Serial.print(buf[i], HEX);
    }
    Serial.println("");
#endif


    DW1000.writeBytes(GPIO_CTRL_ID, GPIO_DOUT_OFFSET, buf, GPIO_DOUT_LEN);
}
uint32_t dw1000_gpio_get_values()
{
    uint32_t reg;
    reg = (uint32_t) dw1000_read_reg(GPIO_CTRL_ID, GPIO_RAW_OFFSET,
                                     sizeof(uint32_t));
    return (reg&0xFFF);
}
void dw1000_gpio_init_out(int gpioNum, int val)
{
#if GPIO_DEBUG
    Serial.print("dw1000_gpio_init_out gpio "); Serial.print(gpioNum); 
    Serial.print(" val "); Serial.println(val);
#endif
    dw1000_gpio_set_direction(gpioNum, 0);
    dw1000_gpio_set_value(gpioNum, val);
}
void dw1000_gpio_init_in(int gpioNum)
{
#if GPIO_DEBUG
    Serial.print("dw1000_gpio_init_in gpio "); Serial.println(gpioNum); 
#endif
    dw1000_gpio_set_direction(gpioNum, 1);
}
int dw1000_gpio_read(uint8_t gpioNum)
{
  uint32_t reg = dw1000_gpio_get_values();
  if (gpioNum == 7 ) {
    // seems like a chip bug????? Reading raw register, sync input (gpio7) is bit 11
    return (reg&(1<<11)) ? 1:0;
  }
  return (reg&(1<<gpioNum)) ? 1:0;
}
void dw1000_gpio_write(int gpioNum, int val)
{
#if GPIO_DEBUG
    Serial.print("dw1000_gpio_write "); Serial.print(gpioNum);
    Serial.print(" = "); Serial.println(val);
#endif
    dw1000_gpio_set_value(gpioNum, val);
}

void dw1000_phy_external_sync(uint8_t delay, bool enable){

    uint16_t reg = dw1000_read_reg(EXT_SYNC_ID, EC_CTRL_OFFSET, sizeof(uint16_t));
    if (enable) {
        reg &= ~EC_CTRL_WAIT_MASK; //clear timer value, clear OSTRM
        reg |= EC_CTRL_OSTRM;      //External timebase reset mode enable
        reg |= ((((uint16_t) delay) & 0xff) << 3); //set new timer value

    }else {
        reg &= ~(EC_CTRL_WAIT_MASK | EC_CTRL_OSTRM); //clear timer value, clear OSTRM
    }
    dw1000_write_reg(EXT_SYNC_ID, EC_CTRL_OFFSET, reg, sizeof(uint16_t));
}

void debug_toggle_sync_as_output() {
  
  /* Debug toggle the sync pin 
  dw1000_gpio_init_out(7,0);
  dw1000_gpio_set_mode(7, 0x1);
  dw1000_gpio_init_out(7,0);
  while ( 1 ) {
    if ( (millis() - led_counter) >= 1000 ) {
      led_counter = millis();
      Serial.println("Toggle sync output");
      dw1000_gpio_write(7, blink ? 1 : 0 );
      blink = !blink;
    }
  }
  */
}


void deca_setup_gpio() {
  bool blink = true;
  // GPIO7-8 , Sync / IRQ, are 0x0 for Sync/IRQ (default) and 0x1 for GPIO mode
    

  
  /* setup decawave LEDs as gpios for fun */
  // GPIO0-6 are gpio by default, 0x0 sets as GPIO
  dw1000_gpio_set_mode(0, 0x0);
  dw1000_gpio_set_mode(1, 0x0);
  dw1000_gpio_set_mode(2, 0x0);
  dw1000_gpio_set_mode(3, 0x0);
  dw1000_gpio_init_out(0, 0); 
  dw1000_gpio_init_out(1, 0); 
  dw1000_gpio_init_out(2, 0); 
  dw1000_gpio_init_out(3, 0); 


  /* Wait for sync pin to get exercised */
  Serial.println("Waiting for decawave sync pin to toggle");
  dw1000_gpio_init_in(7); 
  dw1000_gpio_set_mode(7, 0x1);
  dw1000_gpio_init_in(7); 
  int init_val = 0;
  
  init_val = dw1000_gpio_read(7);
  while ( init_val == dw1000_gpio_read(7) ) {
    delay(250);
    if ( (millis() - deca_led_counter) >= DECA_SETUP_BLINK_INTERVAL ) {
      deca_led_counter = millis();
      dw1000_gpio_write(0, blink ? 1 : 0 );
      dw1000_gpio_write(1, blink ? 1 : 0 );
      dw1000_gpio_write(2, blink ? 1 : 0 );
      dw1000_gpio_write(3, blink ? 1 : 0 );
      blink = !blink;
    }
  }
  Serial.println("Saw sync pin toggle!");

  Serial.println("Setting sync pin to external reset mode (OSTR)");
  dw1000_gpio_set_mode(7, 0x0); // set it to mode zero, sync mode 
  dw1000_phy_external_sync(33, true); // 33 recommended by user guide
  // wait is 8 bits, and modulo 4 should give 1 
}

void decawave_led_setmode(uint8_t dw_led_mode) {
  deca_led_mode = dw_led_mode;
}

void decawave_led_loop() {
  if ( (millis() - deca_led_counter) >= DECA_SETUP_BLINK_INTERVAL ) {
    deca_led_counter = millis();
    if ( deca_led_mode == DECA_LEDMODE_COUNTUP ) {        
      dw1000_gpio_write(0, deca_led_shift_counter & 0x1 ? 1 : 0 );
      dw1000_gpio_write(1, deca_led_shift_counter & 0x2 ? 1 : 0 );
      dw1000_gpio_write(2, deca_led_shift_counter & 0x4 ? 1 : 0 );
      dw1000_gpio_write(3, deca_led_shift_counter & 0x8 ? 1 : 0 );
      deca_led_shift_counter++;
    } else if ( deca_led_mode == DECA_LEDMODE_COUNTDOWN ) {
      dw1000_gpio_write(0, deca_led_shift_counter & 0x1 ? 1 : 0 );
      dw1000_gpio_write(1, deca_led_shift_counter & 0x2 ? 1 : 0 );
      dw1000_gpio_write(2, deca_led_shift_counter & 0x4 ? 1 : 0 );
      dw1000_gpio_write(3, deca_led_shift_counter & 0x8 ? 1 : 0 );
      deca_led_shift_counter--;
    } else {
      dw1000_gpio_write(0, deca_led_shift_counter & 0x1 ? 1 : 0 );
      dw1000_gpio_write(1, deca_led_shift_counter & 0x1 ? 1 : 0 );
      dw1000_gpio_write(2, deca_led_shift_counter & 0x1 ? 1 : 0 );
      dw1000_gpio_write(3, deca_led_shift_counter & 0x1 ? 1 : 0 );
      deca_led_shift_counter++;
    }    
  }
}











#endif

  
