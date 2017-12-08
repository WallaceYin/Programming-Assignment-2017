#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};
#define KEYDOWN_MASK 0x8000
size_t events_read(void *buf, size_t len) {
  int key_data = _read_key();
  if (key_data == _KEY_NONE)
  {
    key_data = _uptime();
    sprintf(buf, "t %d\n", key_data);
  }
  else
  {
    if (((uint32_t)key_data & KEYDOWN_MASK) > 0)
    {
      key_data = key_data & ~KEYDOWN_MASK;
      sprintf(buf, "ku %s\n", keyname[key_data]);
    }
    else
    {
      sprintf(buf, "kd %s\n", keyname[key_data]);
    }
  }
  if (strlen(buf) > len)
    return len;
  else return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	memcpy(buf, dispinfo+offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
    offset >>=2;
    len>>=2;
    int width = _screen.width;
    int x, y;
    x = offset % width;
    y = offset / width;
    int len1 = 0, len2 = 0;
    if (len > width * 2 - x)
    {
      len2 = (len + width - x) % width;
      int len3 = y + (len - width + x)/width;
      _draw_rect(buf+len-len2, 0, len3, len2, 1);
    }
    if (len > width)
    {
      _draw_rect(buf+width-x, 0, y+1, width,(len-width+x)/width);
    }
    len1 = (len -len2)%width;
    _draw_rect(buf, x, y, len1, 1);
}

void init_device() {
  _ioe_init();
	sprintf(dispinfo,"WIDTH : %d\nHEIGHT: %d\n\0", _screen.width, _screen.height);
  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
