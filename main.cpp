#include <hidboot.h>
#include <SPI.h>
#include <SD.h>

// Отправляет коды клавиш на ПК.
class KeyboardOut
{
  private:
    KeyReport _keyReport; //  8 байт для кодировки.
    
    void send_report();
    
  public:
    size_t press(uint8_t mod, uint8_t key);
    size_t release(uint8_t mod, uint8_t key);
};

KeyboardOut keyboard_out;

// Отправляет код нажатия на компьютер.
size_t KeyboardOut::press(uint8_t mod, uint8_t key) {
  uint8_t i;
  
  _keyReport.modifiers |= mod;
  
  if (_keyReport.keys[0] != key && _keyReport.keys[1] != key && 
    _keyReport.keys[2] != key && _keyReport.keys[3] != key &&
    _keyReport.keys[4] != key && _keyReport.keys[5] != key) {

    for (i=0; i<6; i++) {
      if (_keyReport.keys[i] == 0x00) {
        _keyReport.keys[i] = key;
        break;
      }
    }
    if (i == 6) {
      return 0;
    }	
  }
  send_report();
  return 1;
}

// Отправляет код отжатия на компьютер.
size_t KeyboardOut::release(uint8_t mod, uint8_t key) {
  uint8_t i;
  
  _keyReport.modifiers &= mod;
  
  for (i=0; i<6; i++) {
    if (0 != key && _keyReport.keys[i] == key) {
      _keyReport.keys[i] = 0x00;
    }
  }
  send_report();
  return 1;
}

void KeyboardOut::send_report()
{
  HID_SendReport(2, &_keyReport, sizeof(KeyReport));	// Отправляем код клавиши на ПК.
}

// Принимает коды клавиш с USB Host Shield.
class KeyboardIn : public KeyboardReportParser
{
  protected:
    void OnKeyDown  (uint8_t mod, uint8_t key);
    void OnKeyUp  (uint8_t mod, uint8_t key);
};

KeyboardIn keyboard_in;

// Принимает код нажатия с USB Host Shield.
void KeyboardIn::OnKeyDown(uint8_t mod, uint8_t key)
{
  keyboard_out.press(mod, key);
  
  uint8_t c = OemToAscii(mod, key);    // символ с клавиатуры
  PrintHex<uint8_t>(key, 0x80);            // его шестнадцатеричное представление
}

// Принимает код отжатия с USB Host Shield.
void KeyboardIn::OnKeyUp(uint8_t mod, uint8_t key)
{
  keyboard_out.release(mod, key);
  
  uint8_t c = OemToAscii(mod, key);
  PrintHex<uint8_t>(key, 0x80);
}

USB     UsbHost;

HIDBoot<HID_PROTOCOL_KEYBOARD>    HidKeyboard(&UsbHost);

void setup()
{
  UsbHost.Init();
  
  delay( 200 );

  HidKeyboard.SetReportParser(0, (HIDReportParser*)&keyboard_in);
}

void loop()
{
  UsbHost.Task();
}
