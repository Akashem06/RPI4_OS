/*******************************************************************************************************************************
 * @file   sync_test.c
 *
 * @brief  Kernel-space example, exercises the wait-channel core + semaphore/mutex/notify/pipe
 *
 * @details Runs a self-checking suite for the utils/ synchronization primitives. QEMU raspi4b
 *          cannot deliver the timer tick (secure PPI, unreachable at NS-EL2), so scheduling is
 *          cooperative here: an orchestrator thread drives each test and yields with schedule(),
 *          while init_task's idle loop keeps _schedule running so timed blocks still fire.
 *
 * @date   2026-08-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "irq.h"
#include "log.h"
#include "scheduler.h"
#include "timer.h"

/* Intra-component Headers */
#include "kernel.h"
#include "mutex.h"
#include "notify.h"
#include "pipe.h"
#include "semaphore.h"

static int passes = 0;
static int fails = 0;

static void check(bool cond, const char *what) {
  if (cond) {
    passes++;
    log("  [PASS] %s\n\r", what);
  } else {
    fails++;
    log("  [FAIL] %s\n\r", what);
  }
}

/* ------------------------------------------------------------------ wait-channel core ------ */
static int wchan_chan;
static volatile int wchan_woke;

static void wchan_waiter(u64 arg) {
  (void)arg;
  scheduler_block_on(&wchan_chan);
  wchan_woke = 1;
  scheduler_exit_task();
}

static void test_wchan(void) {
  log("[wchan] block_on / wake_chan\n\r");
  wchan_woke = 0;
  scheduler_create_task(PF_KTHREAD, (u64)&wchan_waiter, 0, 8);
  schedule(); /* let the waiter run and block */
  check(wchan_woke == 0, "waiter is still blocked before wake");
  scheduler_wake_chan(&wchan_chan);
  schedule(); /* let the waiter resume and exit */
  check(wchan_woke == 1, "waiter woke after wake_chan");
}

/* -------------------------------------------------------------------------- semaphore ------ */
static Semaphore sem;
static volatile int sem_consumed;

static void sem_consumer(u64 count) {
  for (u64 i = 0; i < count; i++) {
    semaphore_wait(&sem);
    sem_consumed++;
  }
  scheduler_exit_task();
}

static void test_semaphore(void) {
  log("[semaphore] post/wait, try_wait, timeout\n\r");
  semaphore_init(&sem, 0);
  sem_consumed = 0;

  check(semaphore_try_wait(&sem) == false, "try_wait on empty returns false");

  const int N = 5;
  scheduler_create_task(PF_KTHREAD, (u64)&sem_consumer, N, 8);
  for (int i = 0; i < N; i++) {
    semaphore_post(&sem);
    schedule(); /* hand off so the consumer takes each permit */
  }
  schedule();
  check(sem_consumed == N, "consumer took exactly N permits");

  u64 t0 = timer_get_ticks();
  ErrorCode e = semaphore_wait_timeout(&sem, 200);
  u64 dt_ms = (timer_get_ticks() - t0) / (CLOCK_HZ / 1000U);
  check(e == ERR_GEN_TIMEOUT, "wait_timeout returns ERR_GEN_TIMEOUT when unposted");
  check(dt_ms >= 150 && dt_ms <= 400, "wait_timeout waited roughly the deadline");
}

/* ----------------------------------------------------------------------------- notify ------ */
static Notif note;
static volatile int note_woke;

static void note_waiter(u64 arg) {
  (void)arg;
  notif_wait(&note);
  note_woke = 1;
  scheduler_exit_task();
}

static void test_notify(void) {
  log("[notify] signal/wait, sticky pre-signal, timeout\n\r");
  notif_init(&note);
  note_woke = 0;

  scheduler_create_task(PF_KTHREAD, (u64)&note_waiter, 0, 8);
  schedule(); /* waiter blocks on the notification */
  check(note_woke == 0, "waiter blocked before signal");
  notif_signal(&note);
  schedule();
  check(note_woke == 1, "waiter woke after signal");

  /* Sticky: signalling first means a later wait returns immediately */
  notif_init(&note);
  notif_signal(&note);
  notif_wait(&note);
  check(true, "pre-signalled wait returned immediately");

  notif_init(&note);
  ErrorCode e = notif_wait_timeout(&note, 200);
  check(e == ERR_GEN_TIMEOUT, "wait_timeout returns ERR_GEN_TIMEOUT when unsignalled");
}

/* ------------------------------------------------------------------------------ mutex ------ */
static Mutex mtx;
static volatile int mtx_trylock_saw_held;
static volatile int mtx_helper_ran;

static void mtx_contender(u64 arg) {
  (void)arg;
  /* The orchestrator owns the mutex right now, so trylock must fail */
  mtx_trylock_saw_held = (mutex_trylock(&mtx) == false);
  mutex_lock(&mtx); /* now block until the orchestrator releases */
  mtx_helper_ran = 1;
  mutex_unlock(&mtx);
  scheduler_exit_task();
}

static void test_mutex(void) {
  log("[mutex] lock/trylock/recursive/unlock\n\r");
  mutex_init(&mtx);
  mtx_trylock_saw_held = 0;
  mtx_helper_ran = 0;

  check(mutex_trylock(&mtx) == true, "trylock on free mutex succeeds");
  check(mutex_trylock(&mtx) == true, "recursive trylock by owner succeeds");
  mutex_unlock(&mtx); /* drop the recursive level, still held once */

  scheduler_create_task(PF_KTHREAD, (u64)&mtx_contender, 0, 8);
  schedule(); /* contender runs trylock (fails) then blocks on lock */
  check(mtx_trylock_saw_held == 1, "contender's trylock saw the mutex held");
  check(mtx_helper_ran == 0, "contender still blocked while we hold the mutex");
  mutex_unlock(&mtx); /* release the outer level, wakes the contender */
  schedule();
  check(mtx_helper_ran == 1, "contender acquired the mutex after release");
}

/* ------------------------------------------------------------------------------- pipe ------ */
static Pipe pp;
#define PIPE_TEST_BYTES 700
static volatile int pipe_writer_done;

static void pipe_writer(u64 arg) {
  (void)arg;
  u8 chunk[64];
  u16 sent = 0;
  while (sent < PIPE_TEST_BYTES) {
    u16 n = 0;
    while (n < sizeof(chunk) && sent + n < PIPE_TEST_BYTES) {
      chunk[n] = (u8)((sent + n) & 0xFF);
      n++;
    }
    pipe_write(&pp, chunk, n); /* blocks while the pipe is full */
    sent += n;
  }
  pipe_writer_done = 1;
  scheduler_exit_task();
}

static void test_pipe(void) {
  log("[pipe] blocking write > PIPE_SIZE, ordered read, close EOF\n\r");
  pipe(&pp);
  pipe_writer_done = 0;

  scheduler_create_task(PF_KTHREAD, (u64)&pipe_writer, 0, 8);

  u8 buf[64];
  int got = 0;
  bool ordered = true;
  while (got < PIPE_TEST_BYTES) {
    ErrorCode r = pipe_read(&pp, buf, sizeof(buf)); /* blocks while empty */
    if (r <= 0) break;
    for (int i = 0; i < (int)r; i++) {
      if (buf[i] != (u8)((got + i) & 0xFF)) ordered = false;
    }
    got += (int)r;
  }
  check(got == PIPE_TEST_BYTES, "read every byte written across the full boundary");
  check(ordered, "bytes arrived in order");
  check(pipe_writer_done == 1, "writer completed after the reader drained");

  /* Close-while-blocked: reader parks on the empty pipe, close unblocks it to EOF */
  pipe(&pp);
  pipe_close(&pp);
  ErrorCode eof = pipe_read(&pp, buf, sizeof(buf));
  check(eof == 0, "read on a closed, empty pipe returns EOF");
}

/* ------------------------------------------------------------------------------- suite ----- */
static void suite_thread(u64 arg) {
  (void)arg;
  log("\n\r===== KERNEL SYNC PRIMITIVES TEST =====\n\r");

  test_wchan();
  test_semaphore();
  test_notify();
  test_mutex();
  test_pipe();

  log("\n\r===== SYNC TEST DONE: %d passed, %d failed =====\n\r", passes, fails);

  /* Become an idle thread so the scheduler always has something runnable */
  while (1) {
    schedule();
  }
}

void kernel_main() {
  kernel_boot();

  log("\n\r===== STARTING SCHEDULER =====\n\r");
  scheduler_init();
  scheduler_start_tick(SCHED_TICK_HZ);

  scheduler_create_task(PF_KTHREAD, (u64)&suite_thread, 0, 9);

  irq_enable();

  /* init_task idle loop, also drives _schedule so timed blocks fire under cooperative sim */
  while (1) {
    schedule();
  }
}
