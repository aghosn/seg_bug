#include <iostream>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>
#include <atomic>
#include <unistd.h>


typedef void(*fncast)(void); 
typedef unsigned char _BYTE;

#define STACKSIZE 0x4000
#define SIG_PREEMPT SIGUSR1

/// Assembly function to_behaviour to save current stack, load new one
/// and call a switcher routine.
/// Calling convention is rdi, rsi, rdx, rcx, r8, r9 
__asm__(
  "\t.type to_behaviour,@function\n"
  "to_behaviour:\n"
  "\tmov %rsp, (%rdi) #Save system stack.\n"
  "\tmov %rsi, %rsp  # Switch stack\n"
  "\tcall *%rdx # Call the __switcher; arguments are already in correct registers\n"
);


/// Assembly function to_system sets %eax to 0 to signify we were not preempted
/// and switches back to the system stack to perform a return.
__asm__(
  "\t.type to_system,@function\n"
  "to_system:\n"
  "\t mov $0x0, %eax # returned without preemption\n"
  "\tmov %rdi, %rsp # Stack pointer for the system, 1st argument\n"
  "\tret # Return on the system stack\n"
);

__asm__(
  "\t.type trampoline_preempt,@function\n"
  "trampoline_preempt:\n"
  "\tret # Just return on the system stack\n"
);

/// Switches to a behaviour stack. Arguments are:
/// fn, behaviour, _switcher, stacks.behaviour, &stacks.system
extern "C" bool to_behaviour(_BYTE**, _BYTE*, fncast);
extern "C" void to_system(_BYTE* system_stack);
extern "C" void trampoline_preempt(void);

struct Stacks
{
  _BYTE* system;
  _BYTE* behaviour;
  _BYTE* signal;
  ucontext_t context;
};

static Stacks stacks = {nullptr, nullptr};
std::atomic<bool> ready;
std::atomic<size_t> counter;
size_t preempted = 0;
size_t rip = REG_RIP;

/// Not doing much
void routine(void)
{
  std::cout << "Transitionned into the routine" << REG_RIP << std::endl;
  ready = true;
  while(counter == 0)
  {} 
  std::cout << "After the while loop" << std::endl;
  to_system(stacks.system);
}

/// Signal handler
static void signal_handler(int sig, siginfo_t* info, void* _context)
{
  
  ucontext_t* context = (ucontext_t*) _context;
  memcpy(&stacks.context, context, sizeof(ucontext_t));
  context->uc_mcontext.gregs[REG_RAX] = 0x1;
  context->uc_mcontext.gregs[REG_RSP] = (greg_t) stacks.system;
  context->uc_mcontext.gregs[REG_RIP] = (greg_t) trampoline_preempt;
  preempted++;
}

bool switcher()
{
  setcontext(&stacks.context); 
  return false;
}

void* scheduler(void* _unused)
{
  /// Allocate the stacks.
  stacks.behaviour = (_BYTE*) mmap(NULL, STACKSIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0); 
  stacks.signal = (_BYTE*) mmap(NULL, STACKSIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);

  if (stacks.behaviour == MAP_FAILED || stacks.signal == MAP_FAILED)
    abort();

  /// Set the signal handler stack. 
  stack_t ss;
  ss.ss_sp = stacks.signal;
  ss.ss_size = STACKSIZE;
  ss.ss_flags = 0;
  if (sigaltstack(&ss, NULL) == -1)
    abort();
  
  counter = 0;
  bool res = true;
  
  while(res) {
    /// Switch to the behaviour.
    if (counter == 0)
    {
      res = to_behaviour(&stacks.system, stacks.behaviour+STACKSIZE, routine);
      counter++;
    }
    else
      res = switcher();
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
