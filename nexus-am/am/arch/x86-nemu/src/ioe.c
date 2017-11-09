#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  unsigned long rtc_time = inl(RTC_PORT) - boot_time;
  return rtc_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int i, j;
  for (i = 0; i < h; i++)
    for (j = 0; j < w; j++)
      fb[(i + y) * _screen.width + j + x] = pixels[i * w + j];
}

void _draw_sync() {
}

int _read_key() {
  unsigned int key_tri;
  key_tri =inb(0x64);
  if (key_tri)
  {
    unsigned int DATA_PORT;
    DATA_PORT = inl(0x60);
    return DATA_PORT;
  }
  else
    return _KEY_NONE;
}
