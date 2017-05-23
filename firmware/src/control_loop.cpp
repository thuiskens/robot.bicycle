#include <cmath>
#include <numeric>
#include "control_loop.h"
#include "SystemState.h"

namespace hardware {

WORKING_AREA(ControlLoop::waControlThread, 4096);
Thread * ControlLoop::tp_control_ = 0;
ControlLoop * ControlLoop::instance_ = 0;

ControlLoop::ControlLoop()
  : startup_{true}, front_wheel_encoder_{STM32_TIM4, 800},
  acc_y_thresh_{constants::rad_per_degree}
{
  front_wheel_encoder_.set_count(0);
  STM32_TIM5->CNT = 0;
  instance_ = this;
}

// Caller: shell thread
msg_t ControlLoop::start(const char * filename)
{
  tp_control_ = chThdCreateStatic(waControlThread,
                                  sizeof(waControlThread),
                                  chThdGetPriority() + 3,
                                  ControlLoop::thread_function, 
                                  const_cast<char *>(filename));
  if (!tp_control_)
    return 1;

  return 0;
}

// Caller: shell thread
msg_t ControlLoop::stop()
{
  chThdTerminate(tp_control_);
  msg_t m = chThdWait(tp_control_);
  tp_control_ = 0;
  return m;
}

// Caller: shell thread
void ControlLoop::shell_command(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc > 1 || argc < 0) {
    chprintf(chp, "Invalid usage.\r\n");
    return;
  }
  
  msg_t m;
  if (tp_control_) { // control thread is running
    m = stop();
    if (argc == 0) {
      chprintf(chp, "Data collection and control terminated with %d errors.\r\n", m);
      return;
    }
  } else {           // control thread is not running
    if (argc == 0)
      m = start("samples.dat");
    else
      m = start(argv[0]);

    if (m == 0) {
      chprintf(chp, "Data collection and control initiated.\r\n");
      return;
    }
  }
  chprintf(chp, "Errors starting threads with error:  %d.\r\n", m);
}

// Caller: initially called by OS when thread is created.
// This is the highest priority thread
msg_t ControlLoop::thread_function(void * arg)
{
  chRegSetThreadName("control");
  ControlLoop loop;

  return loop.exec(static_cast<const char *>(const_cast<const void *>(arg)));
}

void ControlLoop::set_gyro_lean(Sample& s)
{
  s.has_gyro_lean = true;
  static std::array<float, 100> lean_array {{}};
  static int lean_i = 0;
  static uint32_t system_time_prev = 0;

  // first pass, use static lean value
  s.gyro_lean.startup = startup_;
  if (startup_) {
    float ax = s.mpu6050.accelerometer_x;
    float ay = s.mpu6050.accelerometer_y;
    float az = s.mpu6050.accelerometer_z;
    float accel_mag = std::sqrt(ax*ax + ay*ay + az*az);
    float lean_static = std::asin(ay / accel_mag);

    lean_array[lean_i] = lean_static;
    lean_i = (lean_i + 1)  % lean_array.size();

    // after lean_array.size() samples, set the average value
    if (lean_i == 0) {
      float lean_avg = (std::accumulate(lean_array.begin(),
                                        lean_array.end(), 0.0f) /
                        lean_array.size());
      lean_array[0] = lean_avg;
      s.gyro_lean.angle = lean_avg;
      s.bike_state = BikeState::COLLECT;
      startup_ = false;
    } else {
      s.gyro_lean.angle = lean_static;
    }
    system_time_prev = s.system_time;
    return;
  }

  // after setting average static lean, use gyro lean
  float gyro_lean;
  float dt = ((s.system_time - system_time_prev) *
              constants::system_timer_seconds_per_count);
  gyro_lean = lean_array[0] + s.mpu6050.gyroscope_x * dt;

  lean_array[0] = gyro_lean;
  s.gyro_lean.angle = gyro_lean;
  system_time_prev = s.system_time;
  return;
}

void ControlLoop::illuminate_lean_steer(const Sample & s)
{
  uint32_t lean_led = 0; // green
  uint32_t steer_led = 0; // yellow
  if (s.bike_state == BikeState::STARTUP) {
    lean_led = 1;
    steer_led = 1;
  } else if (s.bike_state == BikeState::COLLECT) {
    const float mag = std::sqrt(std::pow(s.mpu6050.accelerometer_x, 2.0f)
                              + std::pow(s.mpu6050.accelerometer_y, 2.0f)
                              + std::pow(s.mpu6050.accelerometer_z, 2.0f));
    lean_led  = std::abs(s.mpu6050.accelerometer_y / mag) <
      instance_->acc_y_thresh_;
    steer_led = std::abs(s.encoder.steer) < 1.0f * constants::rad_per_degree;
  } else if (s.bike_state == BikeState::RUNNING) {
    // turn on led within 5% of setpoint
    lean_led = std::abs(s.encoder.rear_wheel_rate - s.set_point.theta_R_dot) <
        std::abs(0.05 * s.set_point.theta_R_dot);
    steer_led = std::abs(s.estimate.yaw_rate - s.set_point.yaw_rate) < 0.05f;
  } else if (s.bike_state == BikeState::RAMPDOWN) {
    lean_led = (s.loop_count / 20) % 2;
    steer_led = !lean_led;
  }

  MEM_ADDR(BITBAND(reinterpret_cast<uint32_t>(&(GPIOF->ODR)),
                   GPIOF_LEAN_LED)) = lean_led;
  MEM_ADDR(BITBAND(reinterpret_cast<uint32_t>(&(GPIOF->ODR)),
                   GPIOF_STEER_LED)) = steer_led;
}

// Caller: Control thread
msg_t ControlLoop::exec(const char * file_name)
{
  logging::SampleBuffer sample_buffer(file_name);
  Sample s;
  memset(&s, 0, sizeof(s));
  s.bike_state = BikeState::STARTUP;

  systime_t time = chTimeNow();     // Initial time
  systime_t sleep_time;
  for (uint32_t i = 0; !chThdShouldTerminate(); ++i) {
    time += MS2ST(constants::loop_period_ms);       // Next deadline

    // Begin pre control data collection
    s.system_time = STM32_TIM5->CNT;
    s.loop_count = i;
    imu_.acquire_data(s);
    s.encoder.front_wheel = front_wheel_encoder_.get_angle();
    s.system_state |= systemstate::CollectionEnabled;
    set_gyro_lean(s);
    // End pre control data collection

    // Begin control
    rear_motor_controller_.update(s); // must be called prior to fork update since
    fork_motor_controller_.update(s); // rear wheel rate is needed for gain scheduling
    // End control

    // Put the sample in to the buffer
    bool encode_failure = !sample_buffer.insert(s);

//    // Illuminate the lean and steer LED's based on latest sample
//    illuminate_lean_steer(s);

    // Clear the sample for the next iteration
    // The first time through the loop, computation_time will be logged as zero,
    // subsequent times will be accurate but delayed by one sample period. This
    // is done to ensure that encoding computation is part of timing
    // measurement.
    uint32_t ti = s.system_time;
    uint32_t bike_state = s.bike_state;
    memset(&s, 0, sizeof(s));
    s.bike_state = bike_state;
    s.computation_time = STM32_TIM5->CNT - ti;
    // Similarly, encode failures will be delayed by one sample.
    if (encode_failure)
      s.system_state |= systemstate::SampleBufferEncodeError;

    // set hardware button set
    if (hw_button_enabled())
      s.system_state |= systemstate::HWButton;

    // Go to sleep until next interval
    chSysLock();
    sleep_time = time - chTimeNow();
    if (static_cast<int32_t>(sleep_time) > 0)
      chThdSleepS(sleep_time);
    chSysUnlock();
  } // for

  return sample_buffer.flush_and_close();
}

void ControlLoop::set_lean_threshold_shell(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc == 1) {
    if (instance_) {
      instance_->acc_y_thresh_ = tofloat(argv[0]) * constants::rad_per_degree;
      chprintf(chp, "Lean thresh hold set to %f.\r\n", instance_->acc_y_thresh_);
    } else {
      chprintf(chp, "Start collection first.\r\n");
    }
  } else {
    chprintf(chp, "Invalid usage.\r\n");
  }
}

}
