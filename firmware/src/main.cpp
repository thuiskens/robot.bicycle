#include "ch.h"
#include "chprintf.h"
#include "hal.h"
#include "shell.h"
#include "evtimer.h"

#include "ff.h"

#include "calibration.h"
#include "control_loop.h"
#include "system_commands.h"
#include "vector_table.h"
#include "motor_controller.h"

#if defined(BUILD_TEST)
#include "test.h"
#endif

/*===========================================================================*/
/* Card insertion monitor.                                                   */
/*===========================================================================*/

#define POLLING_INTERVAL 10
#define POLLING_DELAY 10

/**
 * @brief   Card monitor timer.
 */
static VirtualTimer tmr;

/**
 * @brief   Debounce counter.
 */
static unsigned cnt;

/**
 * @brief   Card event sources.
 */
static EventSource inserted_event, removed_event;

/**
 * @brief   Insertion monitor timer callback function.
 *
 * @param[in] p         pointer to the @p BaseBlockDevice object
 *
 * @notapi
 */
static void tmrfunc(void *p) {
  BaseBlockDevice *bbdp = static_cast<BaseBlockDevice *>(p);

  chSysLockFromIsr();
  if (cnt > 0) {
    if (blkIsInserted(bbdp)) {
      if (--cnt == 0) {
        chEvtBroadcastI(&inserted_event);
      }
    }
    else
      cnt = POLLING_INTERVAL;
  }
  else {
    if (!blkIsInserted(bbdp)) {
      cnt = POLLING_INTERVAL;
      chEvtBroadcastI(&removed_event);
    }
  }
  chVTSetI(&tmr, MS2ST(POLLING_DELAY), tmrfunc, bbdp);
  chSysUnlockFromIsr();
}

/**
 * @brief   Polling monitor start.
 *
 * @param[in] p         pointer to an object implementing @p BaseBlockDevice
 *
 * @notapi
 */
static void tmr_init(void *p)
{
  chEvtInit(&inserted_event);
  chEvtInit(&removed_event);
  chSysLock();
  cnt = POLLING_INTERVAL;
  chVTSetI(&tmr, MS2ST(POLLING_DELAY), tmrfunc, p);
  chSysUnlock();
}

/*===========================================================================*/
/* FatFs related.                                                            */
/*===========================================================================*/

/**
 * @brief FS object.
 */
static FATFS SDC_FS;

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state time name\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu %s\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time, tp->p_name);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static const ShellCommand commands[] = {
  {"collect", hardware::ControlLoop::shell_command}, // enable/disable data collection and control
  {"disable", SystemCommands::disable_controllers},
  {"reset", SystemCommands::reset},
  {"threads", cmd_threads},
  {"calibrate", calibration::fork_encoder_calibration},
  {"homefork", calibration::fork_encoder_home},
  // TODO: move all YawRateController static functions elsehwere
  {"e_thresh", hardware::ForkMotorController::set_estimation_threshold_shell},
  {"c_thresh", hardware::ForkMotorController::set_control_delay_shell},
  {"thresh", hardware::ForkMotorController::set_thresholds_shell},
  {"disturb", hardware::ForkMotorController::disturb_shell},
  {"speed_limit", hardware::RearMotorController::speed_limit_shell},
  {"torque", hardware::RearMotorController::torque_shell}, // use: torque = disable rear motor; torque [-|+][x...][.][y...] = command a rear motor torque [N*m]
  {"gains", hardware::RearMotorController::gains_shell}, // use: gains [-|+][x...][.][y...] = set rear motor controller gain K to a new value; gains [-|+][x...][.][y...] [-|+][x...][.][y...] = set K and Ti to new values
  {"l_thresh", hardware::ControlLoop::set_lean_threshold_shell},
  {"yaw_rate", hardware::set_reference_shell<hardware::fork>},
  {"speed", hardware::set_reference_shell<hardware::rear_wheel>},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD2,
  commands
};

/*===========================================================================*/
/* Main and generic code.                                                    */
/*===========================================================================*/

/*
 * MMC card insertion event.
 */
static void InsertHandler(__attribute__((unused)) eventid_t id)
{
  FRESULT err;

  /*
   * On insertion SDC initialization and FS mount.
   */
  if (sdcConnect(&SDCD1))
    return;

  err = f_mount(0, &SDC_FS);
  if (err != FR_OK) {
    sdcDisconnect(&SDCD1);
    return;
  }
  palClearPad(GPIOC, GPIOC_LED);
}

/*
 * MMC card removal event.
 */
static void RemoveHandler(__attribute__((unused)) eventid_t id)
{
  sdcDisconnect(&SDCD1);
  palSetPad(GPIOC, GPIOC_LED);
}

/*
 * Application entry point.
 */
int main()
{
#if defined(BUILD_TEST)
  if (!test_all())
    while(1){}
#endif

  VectorTable v;
  v.relocate();
  static const evhandler_t evhndl[] = { InsertHandler, RemoveHandler };
  Thread * shelltp = NULL;
  static struct EventListener el0, el1;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  chRegSetThreadName("main");

  shellInit();            // Shell manager initialization

  sdStart(&SD2, NULL);    // Activate serial driver 2, default configuration

  sdcStart(&SDCD1, NULL); // Activate SDC driver 1, default configuration
  tmr_init(&SDCD1);       // Activates the card insertion monitor.

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and listen for events.
   */
  chEvtRegister(&inserted_event, &el0, 0);
  chEvtRegister(&removed_event, &el1, 1);
  static WORKING_AREA(waShell, 2048);
  while (true) {
    if (!shelltp)
      shelltp = shellCreateStatic(&shell_cfg1, waShell, sizeof(waShell), NORMALPRIO);
    else if (chThdTerminated(shelltp)) {
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
    chEvtDispatch(evhndl, chEvtWaitOne(ALL_EVENTS));
  }
}
