#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <getopt.h>
#include "config.h"

long unsigned int YUYVtoJPEG(unsigned char* img, int width, int height, int jpegQuality, unsigned char* buf);
int init_socket(char *hostname, int portno);
void send_frame(unsigned char * frame, int len);

static int debug = 0;

static char *dev_name                   = "/dev/video0";
static int fd                           = -1;
static void *buffer_start               = NULL;
static size_t length                    = 0;
static unsigned char *jpegbuffer_start  = NULL;
static int portno                       = 8000;
static char *hostname                   = "127.0.0.1";
static struct v4l2_format fmt;

// necessary to silent clang warning
static void errno_exit(const char *s) __attribute__((noreturn));

static void errno_exit(const char *s) 
{
  fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
  exit(EXIT_FAILURE);
}

static void send_YUV()
{
	long unsigned int size;

	if (debug) {
		printf("send_YUV\n");
	}

  size = YUYVtoJPEG(buffer_start, fmt.fmt.pix.width, fmt.fmt.pix.height, ENCODE_QUALITY, jpegbuffer_start);
	if (debug) {
		printf("send_YUV: size %d\n", size);
	}

  send_frame(jpegbuffer_start, size);

}

static int jpegsize()
{
    int size, i;

    unsigned char * buf = buffer_start;

    for (i = length-1; i >= 0; i--) {
        if ((buf[i] << 8 | buf[i+1]) == 0xFFD9)
            break;
    }

    size = i+2;

    return size;
}

static void send_MJPEG()
{
    int size;

	if (debug) {
		printf("send_MJPEG\n");
	}

    size = jpegsize();

    send_frame(buffer_start, size);

}

static int read_frame()
{
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
		printf("ioctl dqbuf is wrong !!!\n");
		switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				errno_exit("VIDIOC_DQBUF");
		}
	}

	if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
    if(debug)
		  printf("V4L2_PIX_FMT_YUYV\n");
    send_YUV();
	} else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
    if(debug)
		  printf("V4L2_PIX_FMT_MJPEG\n");
    send_MJPEG();
  }

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
		errno_exit("VIDIOC_QBUF");

	return 0;
}

static void mainloop(void)
{
  for (;;) {
    fd_set fds;
    struct timeval tv;
    int r;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    tv.tv_sec = 20;
    tv.tv_usec = 0;

    r = select(fd + 1, &fds, NULL, NULL, &tv);

    if (-1 == r)
    {
      if (EINTR == errno)
        continue;

      errno_exit("select");
    }

    if (0 == r)
    {
      fprintf(stderr, "select timeout\n");
      exit(EXIT_FAILURE);
    }

    if (read_frame())
      break;
    // EAGAIN - continue select loop.
  }
}

static void stop_capturing(void)
{
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
    errno_exit("VIDIOC_STREAMOFF");
}

static void start_capturing(void)
{
  struct v4l2_buffer buf;
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  memset(&buf, 0, sizeof(buf));
  buf.type = type;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;

  if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
    errno_exit("VIDIOC_QBUF ... !!!");

  if (ioctl(fd, VIDIOC_STREAMON, &type) == -1)
    errno_exit("VIDIOC_STREAMON");
}

static void uninit_device(void)
{
  if (munmap(buffer_start, length) == -1)
    errno_exit("munmap");
}

static void init_mmap(void)
{
  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  req.count  = 1;
  req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    errno_exit("VIDIOC_REQBUFS");

  struct v4l2_buffer buf;
  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;

  if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
    errno_exit("VIDIOC_QUERYBUF");

  length = buf.length;
  buffer_start = mmap(NULL /* start anywhere */,
                      length,
                      PROT_READ | PROT_WRITE /* required */,
                      MAP_SHARED /* recommended */,
                      fd, buf.m.offset);

  if (buffer_start == MAP_FAILED)
    errno_exit("mmap");
}

static void get_pixelformat()
{
  struct v4l2_fmtdesc desc;
  memset(&desc, 0, sizeof(desc));
  desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  // iterate over all formats, and prefer MJPEG when available
  while (ioctl(fd, VIDIOC_ENUM_FMT, &desc) == 0) {
    desc.index++;

    if (desc.pixelformat == V4L2_PIX_FMT_MJPEG) {
      fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
      printf("Using: MJPEG\n");
      return;
    }
  }

  printf("Using: YUYV\n");
}

static void init_device(void)
{
  struct v4l2_capability cap;

  memset(&cap, 0, sizeof(cap));
  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    errno_exit("VIDIOC_QUERYCAP");

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf(stderr, "%s is no video capture device\n", dev_name);
    exit(EXIT_FAILURE);
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
    exit(EXIT_FAILURE);
  }

  // Default to YUYV
  memset(&fmt, 0, sizeof(fmt));
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

  get_pixelformat();

  // it'll adjust to the bigger screen available in the driver
  fmt.fmt.pix.width  = FRAME_WIDTH;
  fmt.fmt.pix.height = FRAME_HEIGHT;

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
  	errno_exit("VIDIOC_S_FMT");

  init_mmap ();
}

static void close_device(void)
{
  if (close (fd) == -1)
    errno_exit("close");
}

static void open_device(void)
{
  if ((fd = open(dev_name, O_RDWR)) == -1) {
    fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno,
            strerror (errno));
    exit(EXIT_FAILURE);
  }
}


/*
  print usage information
*/
static void usage(FILE* fp, int argc, char** argv)
{
  fprintf (fp,
    "Usage: %s [options]\n\n"
    "Options:\n"
    "-d | --device name       Video device name [/dev/video0]\n"
    "-h | --help              Print this message\n"
    "-s | --server address    Server [ip address/hostname]\n"
    "-p | --port port number  Port [8888]\n"
   "",
    argv[0]);
}

static const char short_options [] = "d:h:s:p:";

static const struct option
long_options [] = {
        { "device",     required_argument,      NULL,           'd' },
        { "help",       no_argument,            NULL,           'h' },
        { "server",     required_argument,      NULL,           's' },
        { "port",       required_argument,      NULL,           'p' },
       { 0, 0, 0, 0 }
};



int main(int argc, char **argv)
{

  for (;;) {
    int index, c = 0;
                
    c = getopt_long(argc, argv, short_options, long_options, &index);

    if (-1 == c)
      break;

    switch (c) {
      case 0: /* getopt_long() flag */
        break;

      case 'd':
        dev_name = optarg;
        break;

      case 'h':
        // print help
        usage (stdout, argc, argv);
        exit(EXIT_SUCCESS);

      case 's':
        // server hostname or ip address
        hostname = optarg;
        break;

      case 'p':
        // port number
        portno = atoi(optarg);
        break;


      default:
        usage(stderr, argc, argv);
        exit(EXIT_FAILURE);
    }
  }

  printf("video: %s\n", dev_name);
  printf("serverip: %s\n", hostname);
  printf("port: %d\n", portno);

  init_socket(hostname, portno);


  open_device();
  init_device();

  jpegbuffer_start = (unsigned char *)malloc(fmt.fmt.pix.width*fmt.fmt.pix.height*3);	

  start_capturing();
  mainloop();
  stop_capturing();
  uninit_device();
  close_device();

  free(jpegbuffer_start);
  return 0;
}
