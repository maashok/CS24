#include "get_ids.h"
#include <stdio.h>

/* Finds and prints the user ID and group ID from the kernel */
int main() {
  int uid, gid;
  /* Find the uid and gid */
  getID(&uid, &gid);

  printf("User ID is %d. Group ID is %d.\n", uid, gid);

  return 0;
}
