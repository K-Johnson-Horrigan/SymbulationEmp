#include "emp/math/Random.hpp"

#include "../../test_utils.h"

#include "../../../default_mode/SymWorld.h"
#include "../../../default_mode/WorldSetup.cc"
#include "../../../default_mode/DataNodes.h"
#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPWorld.cc"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/SGPWorldData.cc"
#include "../../../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../../../sgp_mode/SGPW_TaskProfileSetup.cc"
#include "../../../sgp_mode/ProgramBuilder.h"
#include "../../../sgp_mode/events/EventManager.h"

#include "../../../catch/catch.hpp"

using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;


TEST_CASE("Nonmatching stress events with target host count larger than host population size", "[sgp][events]"){
  GIVEN("Recurring nonmatching stress events"){
    emp::Random random(2);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/stress-events-cfg/nonmatching-stress-recur-event.json");
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    size_t num_hosts = 7;
    config.INIT_POP_SIZE(num_hosts);
    config.CYCLES_PER_UPDATE(1);
    config.HOST_REPRO_RES(10000);
    config.SYM_HORIZ_TRANS_RES(10000);


    world_t world(random, &config);
    world.Setup();
    auto& builder = world.GetProgramBuilder();
    auto& event_manager = world.GetEventManager();

    // Recurring even is loaded correctly
    auto& recurring_events = event_manager.GetRecurringEvents();
    REQUIRE(event_manager.GetOneTimeEvents().size() == 0);
    REQUIRE(event_manager.GetRecurringEvents().size() == 1);
    REQUIRE(recurring_events[0]->GetEventType() == "nonmatching_stress_event");
    REQUIRE(recurring_events[0]->GetStartUpdate() == 1);
    REQUIRE(recurring_events[0]->GetEndUpdate() == 5);
    REQUIRE(recurring_events[0]->GetFrequency() == 3);

    THEN("Host population size remains constant throughout the run") {
      for(int i = 0; i < 5; i ++){
        world.Update();
        REQUIRE(world.GetNumOrgs() == num_hosts);
      }
    }
  }
}

TEST_CASE("Nonmatching stress events with target host count smaller than host population size", "[sgp][events]"){
  GIVEN("Recurring nonmatching stress events"){
    emp::Random random(2);
    sgpmode::SymConfigSGP config;
    config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
    config.EVENTS_CFG_PATH("source/test/sgp_mode_test/stress-events-cfg/nonmatching-stress-recur-event.json");
    test_utils::SetWellMixed(config, 10, 0);
    config.TASK_IO_BANK_SIZE(10);
    config.SYM_ONLY_FIRST_TASK_CREDIT(1);
    config.HOST_ONLY_FIRST_TASK_CREDIT(1);
    config.INIT_POP_SIZE(10);
    config.CYCLES_PER_UPDATE(52);
    config.HOST_REPRO_RES(1);
    config.SYM_HORIZ_TRANS_RES(1);

    world_t world(random, &config);
    world.Setup();
    auto& builder = world.GetProgramBuilder();
    auto& event_manager = world.GetEventManager();

    // Recurring even is loaded correctly
    auto& recurring_events = event_manager.GetRecurringEvents();
    REQUIRE(event_manager.GetOneTimeEvents().size() == 0);
    REQUIRE(event_manager.GetRecurringEvents().size() == 1);
    REQUIRE(recurring_events[0]->GetEventType() == "nonmatching_stress_event");
    REQUIRE(recurring_events[0]->GetStartUpdate() == 1);
    REQUIRE(recurring_events[0]->GetEndUpdate() == 5);
    REQUIRE(recurring_events[0]->GetFrequency() == 3);

    THEN("Host population size remains constant throughout the run") {
      // update 0
      world.Update();
      
      // update 1, 10 hosts when events are called, 7 after
      REQUIRE(world.GetNumOrgs() == 10);
      world.Update();
      REQUIRE(world.GetNumOrgs() == 7);

      // updates 2, 3
      world.Update();
      world.Update();

      // update 4, > 7 hosts when events are called, 7 after
      REQUIRE(world.GetNumOrgs() > 7);
      world.Update();
      REQUIRE(world.GetNumOrgs() == 7);

      // update 5
      world.Update();
    }
  }
}