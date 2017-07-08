#include <cstdint>
#include <cmath>
#include "ch.h"
#include "chprintf.h"
#include "control_loop.h"
#include "constants.h"
#include "rear_motor_controller.h"
#include "SystemState.h"
#include "textutilities.h"

namespace hardware {

const uint8_t ccr_channel = 1;              // PWM Channel 1
const float max_current = 24.0f;            // Copley Controls ACJ-090-36
                                            // Configured 24.0A peak, 12.0A
                                            // continuous
const float torque_constant = 6.654987675770698f;  // Experimentally determined, N*m/A
const float max_torque = max_current * torque_constant;
const float d0 = 10.0f * constants::two_pi; // filter pole at -d0 rad / s
const float n0 = d0;
const float n1 = 0.0f;

RearMotorController::RearMotorController()
  : MotorController{"rear wheel"},
  e_{STM32_TIM8, constants::wheel_counts_per_revolution},
  m_{GPIOF, GPIOF_RW_DIR, GPIOF_RW_ENABLE, GPIOF_RW_FAULT,
     STM32_TIM1, ccr_channel, max_current, torque_constant, true},
  theta_R_dot_command_{0.0f}, integrator_state_{0.0f},
  K_{50.0f}, Ti_{2000.0f},
  rear_wheel_rate_prev_{0.0f},
  system_time_prev_{0}, rear_wheel_count_prev_{0},
  low_pass_filter_{n0, n1, d0, constants::loop_period_s},
  dthetadt_array_{{}}, dthetadt_elem_{0},
  distance_{0.0f}, distance_limit_{0.0f}, pi_controller_on_{true}
{
  instances[rear_wheel] = this;
  e_.set_count(0);
}

RearMotorController::~RearMotorController()
{
  instances[rear_wheel] = 0;
}

void RearMotorController::set_reference(float speed)
{
  pi_controller_on_ = true; // turn on the PI controller
  float theta_R_dot_command_new = speed / -constants::wheel_radius;
  theta_R_dot_command_ = theta_R_dot_command_new;
}

void RearMotorController::disable()
{
  m_.disable();
}

void RearMotorController::enable()
{
  m_.enable();
}

void RearMotorController::update(Sample & s)
{
  s.encoder.rear_wheel_count = e_.get_count();
  s.encoder.rear_wheel = e_.get_angle();
  s.set_point.theta_R_dot = theta_R_dot_command_;

  if (s.set_point.theta_R_dot == 0.0f) {
    s.bike_state = BikeState::COLLECT;
  } else {
    if (s.bike_state != BikeState::RAMPDOWN)
      s.bike_state = BikeState::RUNNING;
  }

  // moving average for rear_wheel_rate
  auto& now = dthetadt_array_[dthetadt_elem_];
  now.first = s.system_time;
  now.second = s.encoder.rear_wheel_count;

  dthetadt_elem_ = (dthetadt_elem_ + 1) % dthetadt_array_.size();
  auto& prev = dthetadt_array_[dthetadt_elem_];

  const float dt = (now.first - prev.first) * constants::system_timer_seconds_per_count;
  const float dthetadt = static_cast<int16_t>(now.second - prev.second) *
                            e_.get_rad_per_count() / dt;

  low_pass_filter_.update(dthetadt);
  s.encoder.rear_wheel_rate = low_pass_filter_.output(dthetadt);

  // Update the torque if tracking the commanded speed and log the desired and saturated torque
  if (pi_controller_on_) // tracking commanded speed using PI controller
  {
    const float error = theta_R_dot_command_ - s.encoder.rear_wheel_rate;
    s.motor_torque.desired_rear_wheel = K_ * error + integrator_state_;

    m_.set_torque(s.motor_torque.desired_rear_wheel);         // desired torque
    s.motor_torque.rear_wheel = m_.get_torque();              // saturated torque

    // update integrator state if torque not saturating
    if (s.motor_torque.rear_wheel == s.motor_torque.desired_rear_wheel)
      integrator_state_ += K_ / Ti_ * error * dt;
  }
  else                   // applying constant torque commanded by user
  {
    // Log the desired and saturated torque
    s.motor_torque.desired_rear_wheel = 0.0f; // desired torque not available here
    s.motor_torque.rear_wheel = m_.get_torque();              // saturated torque
  }

  // Illuminate the front (yellow) LED when the rear torque is directed forwards
  MEM_ADDR(BITBAND(reinterpret_cast<uint32_t>(&(GPIOF->ODR)),
                   GPIOF_STEER_LED)) = m_.current_direction();

  // Illuminate the rear (green) LED when the rear torque is directed backwards
  MEM_ADDR(BITBAND(reinterpret_cast<uint32_t>(&(GPIOF->ODR)),
                   GPIOF_LEAN_LED)) = !m_.current_direction();

  // update distance travelled
  distance_ += static_cast<int16_t>(s.encoder.rear_wheel_count -
      rear_wheel_count_prev_) * e_.get_rad_per_count() *
    -constants::wheel_radius;
  // step down speed setpoint if distance limit has been reached and
  // distance limit is positive
  if (distance_limit_ > 0.0f && distance_ > distance_limit_) {
    distance_limit_ = 0.0f;
    // Set the reference speed to be a small value that allows a person to
    // catch up to the bike but large enough such that the bike balances.
    set_reference(1.0f);
    s.bike_state = BikeState::RAMPDOWN;
  }

  system_time_prev_ = s.system_time;
  rear_wheel_count_prev_ = s.encoder.rear_wheel_count;
  rear_wheel_rate_prev_ = s.encoder.rear_wheel_rate;

  if (e_.rotation_direction())
    s.system_state |= systemstate::RearWheelEncoderDir;
  if (m_.is_enabled())
    s.system_state |= systemstate::RearWheelMotorEnable;
  if (m_.has_fault())
    s.system_state |= systemstate::RearWheelMotorFault;
  if (m_.current_direction())
    s.system_state |= systemstate::RearWheelMotorCurrentDir;

  //s.wheel_rate_pi.x = theta_R_dot_command_;
  //s.wheel_rate_pi.e = e;
  //s.wheel_rate_pi.e_s = e_s;
  //s.wheel_rate_pi.K = K_;
  //s.wheel_rate_pi.Ti = Ti_;
  //s.wheel_rate_pi.Tt = Tt_;
  //s.wheel_rate_pi.dt = dt;
  //s.has_wheel_rate_pi = true;
}

void RearMotorController::set_distance_limit(float distance)
{
  distance_limit_ = distance;
}

void RearMotorController::speed_limit_shell(BaseSequentialStream *chp,
                                            int argc, char *argv[])
{
  if (argc == 2) {
    RearMotorController* fmc = reinterpret_cast<RearMotorController*>(
                                        instances[rear_wheel]);
    fmc->set_reference_shell(chp, argc - 1, argv);
    fmc->set_distance_limit(tofloat(argv[1]));
  } else {
    chprintf(chp, "Invalid usage.\r\n");
  }
}

// torque_shell - command a constant rear motor torque
//  How to use (in PuTTY or another serial program):
//    torque = disable the rear motor
//    torque [-|+][x...][.][y...] = command a rear motor torque [N*m], examples:
//      torque 5 = command a rear motor torque of 5 N*m (backwards)
//      torque -5.6 = command a rear motor torque of -5.6 N*m (forwards)
void RearMotorController::torque_shell(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc == 0)      // 0 input arguments
  {
    // Retrieve a pointer to the RearMotorController object
    RearMotorController* fmc = reinterpret_cast<RearMotorController*>(instances[rear_wheel]);

    // Disable the rear motor and send a message to the user
    fmc->disable();
    chprintf(chp, "Rear wheel motor disabled.\r\n");
  }
  else if (argc == 1) // 1 input argument
  {
    // Retrieve a pointer to the RearMotorController object
    RearMotorController* fmc = reinterpret_cast<RearMotorController*>(instances[rear_wheel]);

    // Turn off the PI controller
    fmc->pi_controller_on_ = false;

    // Set torque commanded by user
    fmc->enable();                             // enable the rear motor
    float commanded_torque = tofloat(argv[0]); // convert the char* input to a float
    fmc->m_.set_torque(commanded_torque);      // set the commanded torque

    // Send a message to the user
    float saturated_torque = fmc->m_.get_torque();
    chprintf(chp, "Rear wheel motor torque set to %f\r\n", saturated_torque);
  }
  else                // more than 1 input argument, thus invalid use
  {
    chprintf(chp, "Invalid usage.\r\n");
  }
}

// gains_shell - change the rear motor controller gains
//  How to use (in PuTTY or another serial program):
//    gains [-|+][x...][.][y...] = change the K gain, for example:
//      gains 50 = change K to 50
//    gains [-|+][x...][.][y...] [-|+][x...][.][y...] = change the K and Ti gains, e.g.:
//      gains 50 2000 = change K to 50 and Ti to 2000
void RearMotorController::gains_shell(BaseSequentialStream *chp, int argc, char *argv[])
{
  if ((argc == 1) || (argc == 2)) // 1 or 2 input arguments -> change the K gain
  {
    // Retrieve a pointer to the RearMotorController object
    RearMotorController* rmc = reinterpret_cast<RearMotorController*>(instances[rear_wheel]);
    
    // Save the previous K gain and change it to the specified value
    float K_old = rmc->K_;
    float K_new = tofloat(argv[0]);
    rmc->K_ = K_new;
    
    // Send a message to the user
    chprintf(chp, "Rear motor controller gain K changed from %f to %f", K_old, K_new);
    
    // Also change the Ti gain if a second argument is given
    if (argc == 2) // 2 input arguments -> also change the Ti gain
    {
      // Save the previous Ti gain and change it to the specified value
      float Ti_old = rmc->Ti_;
      float Ti_new = tofloat(argv[1]);
      rmc->Ti_ = Ti_new;
      
      // Send a message to the user
      chprintf(chp, "Rear motor controller gain Ti changed from %f to %f", Ti_old, Ti_new);
    } // if (argc == 2)
  } // if ((argc == 1) || (argc == 2))
  else // not 1 or 2 input arguments, thus invalid use
  {
    chprintf(chp, "Invalid usage.\r\n");
  }
}

} // namespace hardware

