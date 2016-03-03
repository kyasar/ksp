#include <stdio.h>
#include <sys/time.h>
#include <event.h>

unsigned int i = 0;

void timer_handler(int fd, short event, void *arg)
{
  printf("%06d\n", i++);
}

int main(int argc, const char* argv[])
{
  struct event ev;
  struct timeval tv;

  tv.tv_sec = 0;
  tv.tv_usec = 1000;

    event_init();    
    event_set(&ev, -1, EV_PERSIST, timer_handler, NULL);
    event_add(&ev, &tv);
 //   event_loop(0);

  return 0;
}
