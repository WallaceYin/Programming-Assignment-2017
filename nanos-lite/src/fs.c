#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
	off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
	int fd;
	fd = fs_open("/dev/fb", 0, 0);
	file_table[fd].size = _screen.width * _screen.height;
}

extern void ramdisk_read(void *, off_t, size_t);
extern void ramdisk_write(const void *, off_t, size_t);
extern void dispinfo_read(void *, off_t, size_t);
extern void fb_write(const void *, off_t, size_t);
extern size_t events_read(void *buf, size_t);

int fs_open(const char *pathname, int flags, int mode) {
  int ret = -1;
  int i;
  for (i = 0; i < NR_FILES; i++)
  {
    if (strcmp(file_table[i].name,pathname) == 0)
    {
      ret = i;
      break;
    }
  }
  if (ret == -1)
    panic("File not found");
  return ret;
}

ssize_t fs_read(int fd, const void *buf, size_t len) {
	switch (fd) {
		case (FD_STDIN):
		case (FD_STDOUT):
		case (FD_STDERR):
			return 0;
		case (FD_DISPINFO):
      len = (file_table[fd].open_offset + len > file_table[fd].size) ? file_table[fd].size - file_table[fd].open_offset: len;
			dispinfo_read((void*)buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		case (FD_EVENTS):
			return events_read((void*)buf, len);
			break;
		default:
      len = (file_table[fd].open_offset + len > file_table[fd].size) ? file_table[fd].size - file_table[fd].open_offset: len;
			ramdisk_read((void*)buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
	switch (fd) {
		case (FD_STDIN):
			return -1;
			break;
		case (FD_STDOUT):
		case (FD_STDERR):
      for (int i = 0; i < len; i++)
        _putc(((char *)buf)[i]);
			return len;
			break;
		case (FD_FB):
			fb_write((void*)buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		default:
			len = (file_table[fd].open_offset + len > file_table[fd].size) ? file_table[fd].size - file_table[fd].open_offset : len;
			ramdisk_write((void*)buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
	}
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
	//printf("in fs_lseek, fd = %d\n",fd);
  assert(fd != 1 && fd != 2);
  if (fd == FD_FB)
  {
    file_table[fd].open_offset = offset;
    return file_table[fd].open_offset;
  }
	switch (whence) {
		case SEEK_SET:
			file_table[fd].open_offset = offset;
      if (file_table[fd].open_offset > file_table[fd].size)
        file_table[fd].open_offset = file_table[fd].size;
			break;

		case SEEK_CUR:
			file_table[fd].open_offset += offset;
      if (file_table[fd].open_offset > file_table[fd].size)
        file_table[fd].open_offset = file_table[fd].size;
			break;

		case SEEK_END:
			file_table[fd].open_offset = file_table[fd].size + offset;
      if (file_table[fd].open_offset > file_table[fd].size)
        file_table[fd].open_offset = file_table[fd].size;
			break;
		default: panic("An error happened in fs_lseek");
	}
	//printf("in fs_lseek, fd = %d and size = %d",fd, file_table[fd].size);
	return file_table[fd].open_offset;
}

int fs_close(int fd) {
	return 0;
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}
