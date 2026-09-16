#include <poker/poker.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
  const poker_card royal[5] = {32,36,40,44,48};
  poker_rank result = 0;
  if (poker_eval_high(royal,5,&result) != POKER_OK || result != 1) return 1;
  const poker_backend_info* info = poker_backend();
  void* state = malloc(info->state_bytes + info->state_alignment);
  if (!state) return 2;
  uintptr_t raw = (uintptr_t)state;
  void* aligned = (void*)((raw + info->state_alignment - 1) / info->state_alignment * info->state_alignment);
  int failed = poker_prepare_high(aligned,info->state_bytes,royal,3,5,1024*1024) != POKER_OK;
  if (!failed) failed = poker_complete(aligned,royal+3,NULL,&result) != POKER_OK || result != 1;
  free(state);
  return failed;
}
