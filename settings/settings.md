# Settings

Having defined the [user interface](userinterface/userinterface.md), we want to store the settings after a power failure.

Looking closely at the [code](userinterface/userinterface.ino), the following variables need to be stored:
- color_index (byte)
- it_is (bit)
- auto_high (byte)
- auto_low (byte)
- current_brightness (byte)

Because the ROM only has a minimal amount of writes, we are only going to write the settings after a timeout.

Based on the technical specifications of the DS1307 clock module in the kit, these values can be written to addresses 0x08 to 0x0C. The code read and write the registers is stolen from the [DS3231](https://github.com/rodan/ds3231) library.

```
void store_settings() 
{
  Serial.println("store_settings");
  writeRegister8(0x08, (byte)color_index);
  writeRegister8(0x09, (byte)it_is);
  writeRegister8(0x0A, (byte)auto_high);
  writeRegister8(0x0B, (byte)auto_low);

  int bm = max(0, current_brightness);
  writeRegister8(0x0C, (byte)bm);
  
}

void restore_settings()
{
  Serial.println("restore_settings");
  color_index = min(nr_led_colors, readRegister8(0x08));
  it_is = readRegister8(0x09);
  auto_high = min(15, readRegister8(0x0A));
  auto_low = min(auto_high-1, readRegister8(0x0B));
  byte bmb = readRegister8(0x0C);
  if (bmb == 0) 
    current_brightness = automatic;
  else
    current_brightness = bmb;
}
```

Because the restore_settings behaves a little strange when reading uninitialized values, a [default settings](default_settings/default_settings.ino) program is provided to initialize the clock and the settings to appropriate default values.