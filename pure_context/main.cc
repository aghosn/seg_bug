#include <iostream>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>
#include <atomic>
#include <unistd.h>


typedef void(*fncast)(); 
typedef unsigned char _BYTE;

#define STACKSIZE 0x4000
#define SIG_PREEMPT SIGUSR1

struct Behaviour 
{
  _BYTE* signal;          // Stack for the signal handler.
  _BYTE* behaviour;       // Stack for the behaviour.
  ucontext_t sched_ctxt; // Scheduler context.
  ucontext_t behav_ctxt; // Behaviour context.
  bool done;
};

static thread_local Behaviour state = {0};

std::atomic<bool> ready;
std::atomic<size_t> counter;
bool done = false;

/// Not doing much
void routine(void)
{
  std::cout << "Transitionned into the routine" << REG_RIP << std::endl;
  ready = true;
  while(counter == 0)
  {} 
  std::cout << "After the while loop" << std::endl;
}

/// Signal handler
static void signal_handler(int sig, siginfo_t* info, void* _context)
{
  
  ucontext_t* context = (ucontext_t*) _context;
  swapcontext(&state.behav_ctxt, &state.sched_ctxt);
}

void wrapper(fncast fn)
{
  state.done = false;
  fn();
  state.done = true;
  setcontext(&state.sched_ctxt);
}

void* scheduler(void* _unused)
{
  /// Allocate the stacks.
  state.behaviour = (_BYTE*) mmap(NULL, STACKSIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0); 
  state.signal = (_BYTE*) mmap(NULL, STACKSIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);

  if (state.behaviour == MAP_FAILED || state.signal == MAP_FAILED)
    abort();

  /// Set the signal handler stack. 
  stack_t ss;
  ss.ss_sp = state.signal;
  ss.ss_size = STACKSIZE;
  ss.ss_flags = 0;
  if (sigaltstack(&ss, NULL) == -1)
    abort();
 
  /// Make sure the counter is 0 before scheduling the behaviour.
  counter = 0;
  getcontext(&state.behav_ctxt);
  state.behav_ctxt.uc_stack.ss_sp = state.behaviour;
  state.behav_ctxt.uc_stack.ss_size = STACKSIZE;
  state.behav_ctxt.uc_stack.ss_flags = 0;

  makecontext(&state.behav_ctxt,(void (*)()) wrapper, 1, routine);

  while(!state.done) {
    /// Switch to the behaviour.
    swapcontext(&state.sched_ctxt, &state.behav_ctxt);
    counter++;
  }
  
  std::cout << "About to finish" << std::endl; 
  return NULL;
}

int main(void)
{

  /// Register signal handler.
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
  sa.sa_sigaction = signal_handler;
  sigaction(SIG_PREEMPT, &sa, NULL);

  ready = false;

  pthread_t thread;
  int ret = pthread_create(&thread, NULL, scheduler, NULL);
  if (ret != 0)
    abort();

  while(!ready)
    sleep(1);

  pthread_kill(thread, SIG_PREEMPT);
  pthread_join(thread, NULL);
  std::cout << "Done" << std::endl;
  return 0;
}
