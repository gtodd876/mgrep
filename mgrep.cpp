#include <stdio.h>
#include <unistd.h>

static char cwd_buffer[512];

int main() {

  if (getcwd(cwd_buffer, sizeof(cwd_buffer)) != NULL) {
    printf("%s\n", cwd_buffer);
  }

  return 0;
}