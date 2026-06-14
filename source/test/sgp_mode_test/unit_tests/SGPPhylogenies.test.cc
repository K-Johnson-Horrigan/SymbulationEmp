#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to unit tests for SGP phylogenies
 */

TEST_CASE("SGP GetCalcHostInfoFun function calculates correctly when PHYLOGENY_TAXON_TYPE is task profiles (4)", "[sgp][sgp-unit]"){
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.POP_SIZE(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE(4);

  world_t world(random, &config);
  world.Setup();
  
  auto& prog_builder = world.GetProgramBuilder();
  sgp_host_t host = sgp_host_t(&random, &world, &config, prog_builder.CreateReproProgram(100));

  GIVEN("A NOT-only host"){
    size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    host.GetHardware().GetCPUState().MarkTaskPerformed(not_task_id);

    WHEN("GetCalcHostInfoFun function is passed a host"){
      std::function<double(Organism&)> calc_info_fun = world.GetCalcHostInfoFun();
      double info = calc_info_fun(host);

      THEN("The correct taxon information is calculated"){
        REQUIRE(info == 0b000000010);
      }
    }
  }

  GIVEN("An ORN & EQU host"){
    size_t orn_task_id = world.GetTaskEnv().GetTaskSet().GetID("OR_NOT");
    size_t equ_task_id = world.GetTaskEnv().GetTaskSet().GetID("EQU");

    host.GetHardware().GetCPUState().MarkTaskPerformed(orn_task_id);
    host.GetHardware().GetCPUState().MarkTaskPerformed(equ_task_id);

     WHEN("GetCalcHostInfoFun function is passed a host"){
      std::function<double(Organism&)> calc_info_fun = world.GetCalcHostInfoFun();
      double info = calc_info_fun(host);

      THEN("The correct taxon information is calculated"){
        REQUIRE(info == 0b100000100);
      }
    }
  }
}

TEST_CASE("SGP GetCalcSymInfoFun when PHYLOGENY_TAXON_TYPE is task profiles (4)", "[sgp][sgp-unit]"){
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.POP_SIZE(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE(4);

  world_t world(random, &config);
  world.Setup();

  auto& prog_builder = world.GetProgramBuilder();
  emp::Ptr<sgp_sym_t> symbiont = emp::NewPtr<sgp_sym_t>(&random, &world, &config, prog_builder.CreateReproProgram(100));

  GIVEN("A NOT-only symbiont"){
    size_t not_task_id = world.GetTaskEnv().GetTaskSet().GetID("NOT");
    symbiont->GetHardware().GetCPUState().MarkTaskPerformed(not_task_id);

    WHEN("GetCalcSymInfoFun function is passed a symbiont"){
      std::function<double(Organism&)> calc_info_fun = world.GetCalcSymInfoFun();
      double info = calc_info_fun(*symbiont);

      THEN("The correct taxon information is calculated"){
        REQUIRE(info == 0b000000010);
      }
    }
  }

  GIVEN("An ORN & EQU symbiont"){
    size_t orn_task_id = world.GetTaskEnv().GetTaskSet().GetID("OR_NOT");
    size_t equ_task_id = world.GetTaskEnv().GetTaskSet().GetID("EQU");

    symbiont->GetHardware().GetCPUState().MarkTaskPerformed(orn_task_id);
    symbiont->GetHardware().GetCPUState().MarkTaskPerformed(equ_task_id);

     WHEN("GetCalcSymInfoFun function is passed a symbiont"){
      std::function<double(Organism&)> calc_info_fun = world.GetCalcSymInfoFun();
      double info = calc_info_fun(*symbiont);

      THEN("The correct taxon information is calculated"){
        REQUIRE(info == 0b100000100);
      }
    }
  }
  
  // don't try to delete nonexistent taxon in symbiont deconstructor
  config.PHYLOGENY(0);
  symbiont.Delete();

  // delete existing symbiont systematics object
  config.PHYLOGENY(1);
}