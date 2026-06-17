#include "../../../sgp_mode/SGPWorld.h"
#include "../../../sgp_mode/SGPHost.h"
#include "../../../sgp_mode/SGPWorldSetup.cc"
#include "../../../sgp_mode/ProgramBuilder.h"

#include "../../../catch/catch.hpp"

/**
 * This file is dedicated to functional tests for SGP phylogenies
 */

TEST_CASE("Host phylogenies track correctly when PHYLOGENY_TAXON_TYPE is task profiles (4)", "[sgp]"){
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  /**
  * Test that: 
  * Host are added from phylogeny
  * - on setup
  * - on birth
  * 
  * Host are removed from phylogeny
  * - on death/deletion
  * 
  * Offspring taxa correctly
  * - track their parent taxon
  * - are iniated upon new task profile 
  */
  
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.POP_SIZE(4);
  config.START_MOI(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  config.TASK_PROFILE_MODE("parent-all");

  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE(4);
  config.STORE_EXTINCT(0);

  GIVEN("A world full of hosts"){
    world_t world(random, &config);
    world.Setup();

    emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::HostTaxonData>> host_sys = world.GetHostSys();

    THEN("Taxa are setup for all initiating hosts"){
      REQUIRE(host_sys->GetNumActive() == 4);
    }

    WHEN("A host dies "){
      emp::Ptr<Organism> host = world.GetOrgPtr(0);

      host->SetDead();
      world.Update();

      THEN("Its taxon is removed from the host systematic"){
        REQUIRE(world.GetNumOrgs() == 3);
        REQUIRE(host_sys->GetNumActive() == 3);
      }
    }
    
    WHEN("A host is born and it has a different task profile than its parent"){
      sgp_host_t& host = static_cast<sgp_host_t&>(world.GetOrg(0));
      host.GetHardware().GetCPUState().ResetTaskPerformance(1);
      
      emp::Ptr<Organism> host_offspring = host.Reproduce();

      world.AddOrgAt(host_offspring, 1, 0);

      emp::Ptr<taxon_t::base_taxon_t> host_taxon = host.GetTaxon();
      emp::Ptr<taxon_t::base_taxon_t> host_offspring_taxon = host_offspring->GetTaxon();
      
      world.Update();

      THEN("The child host's taxon is different than the parent's"){
        REQUIRE(host_taxon->GetInfo() != host_offspring_taxon->GetInfo());
        REQUIRE(host_taxon->GetID() != host_offspring_taxon->GetID());
      }

      THEN("The child taxon tracks the parent taxon"){
        REQUIRE(host_offspring_taxon->GetParent() == host_taxon);
      }
    }
    
    WHEN("A host is born and it has the same task profile as its parent"){
      sgp_host_t& host = static_cast<sgp_host_t&>(world.GetOrg(0));
      for(int i = 0; i < host.GetHardware().GetCPUState().GetNumTasks(); i++){
        if(host.GetHardware().GetCPUState().GetParentTaskPerformed(i)){
          host.GetHardware().GetCPUState().MarkTaskPerformed(i);
        }
      }

      emp::Ptr<Organism> host_offspring = host.Reproduce();
      world.AddOrgAt(host_offspring, 1, 0);

      emp::Ptr<taxon_t::base_taxon_t> host_taxon = host.GetTaxon();
      emp::Ptr<taxon_t::base_taxon_t> host_offspring_taxon = host_offspring->GetTaxon();
      
      world.Update();

      THEN("The child host's taxon is the same as the parent's"){
        REQUIRE(host_taxon->GetInfo() == host_offspring_taxon->GetInfo());
        REQUIRE(host_taxon->GetID() == host_offspring_taxon->GetID());
      }
    }
  }
}

TEST_CASE("Symbiont phylogenies track correctly when PHYLOGENY_TAXON_TYPE is task profiles (4)", "[sgp]"){
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.POP_SIZE(4);
  config.START_MOI(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  config.TASK_PROFILE_MODE("parent-all");

  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE(4);
  config.STORE_EXTINCT(0);

  GIVEN("A world full of hosted symbionts"){
    world_t world(random, &config);
    world.Setup();

    emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::SymbiontTaxonData>> sym_sys = world.GetSymSys();
    
    THEN("Taxa are setup for all initiating symbionts"){
      REQUIRE(sym_sys->GetNumActive() == 4);
    }
    
    WHEN("A symbiont dies "){
      emp::Ptr<Organism> host = world.GetOrgPtr(0);
      emp::Ptr<Organism> symbiont = host->GetSymbionts().at(0);

      symbiont->SetDead();
      world.Update();

      THEN("Its taxon is removed from the symbiont systematic"){
        REQUIRE(sym_sys->GetNumActive() == 3);
      }
    }
    
    WHEN("A symbiont is born and it has a different task profile than its parent"){
      sgp_sym_t& symbiont = static_cast<sgp_sym_t&>(*world.GetOrgPtr(0)->GetSymbionts().at(0));
      symbiont.GetHardware().GetCPUState().ResetTaskPerformance(1);
      
      
      emp::Ptr<Organism> symbiont_offspring = symbiont.Reproduce();
      emp::Ptr<taxon_t::base_taxon_t> symbiont_taxon = symbiont.GetTaxon();
      emp::Ptr<taxon_t::base_taxon_t> symbiont_offspring_taxon = symbiont_offspring->GetTaxon();
      
      world.Update();

      THEN("The child symbiont's taxon is different than the parent's"){
        REQUIRE(symbiont_taxon->GetInfo() != symbiont_offspring_taxon->GetInfo());
        REQUIRE(symbiont_taxon->GetID() != symbiont_offspring_taxon->GetID());
      }

      THEN("The child taxon tracks the parent taxon"){
        REQUIRE(symbiont_offspring_taxon->GetParent() == symbiont_taxon);
      }

      symbiont_offspring.Delete();  
    }
    
    WHEN("A host is born and it has the same task profile as its parent"){
      sgp_sym_t& symbiont = static_cast<sgp_sym_t&>(*world.GetOrgPtr(0)->GetSymbionts().at(0));
      
      for(int i = 0; i < symbiont.GetHardware().GetCPUState().GetNumTasks(); i++){
        if(symbiont.GetHardware().GetCPUState().GetParentTaskPerformed(i)){
          symbiont.GetHardware().GetCPUState().MarkTaskPerformed(i);
        }
      }
      
      emp::Ptr<Organism> symbiont_offspring = symbiont.Reproduce();

      emp::Ptr<taxon_t::base_taxon_t> symbiont_taxon = symbiont.GetTaxon();
      emp::Ptr<taxon_t::base_taxon_t> symbiont_offspring_taxon = symbiont_offspring->GetTaxon();
      
      world.Update();

      THEN("The child symbiont's taxon is the same as the parent's"){
        REQUIRE(symbiont_taxon->GetInfo() == symbiont_offspring_taxon->GetInfo());
        REQUIRE(symbiont_taxon->GetID() == symbiont_offspring_taxon->GetID());
      }

      symbiont_offspring.Delete();  
    }
  }
}

TEST_CASE("Host phylogenies track correctly when PHYLOGENY_TAXON_TYPE is individual and tracking task profiles (5)", "[sgp]"){
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;

  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.POP_SIZE(4);
  config.START_MOI(0);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");

  config.TASK_PROFILE_MODE("self-all");

  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE(5);
  config.STORE_EXTINCT(0);

  GIVEN("A world full of hosts"){
    world_t world(random, &config);
    world.Setup();

    emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::HostTaxonData>> host_sys = world.GetHostSys();

    THEN("Taxa are setup for all initiating hosts"){
      REQUIRE(host_sys->GetNumActive() == 4);
    }

    WHEN("A host dies "){
      emp::Ptr<Organism> host = world.GetOrgPtr(0);

      host->SetDead();
      world.Update();

      THEN("Its taxon is removed from the host systematic"){
        REQUIRE(world.GetNumOrgs() == 3);
        REQUIRE(host_sys->GetNumActive() == 3);
      }
    }
    
    WHEN("A host completes a new task"){
      sgp_host_t& host = static_cast<sgp_host_t&>(world.GetOrg(0));
      host.GetHardware().GetCPUState().MarkTaskPerformed(0);

      emp::Ptr<taxon_t::base_taxon_t> host_taxon = host.GetTaxon();
      world.Update();

      THEN("Its taxon data is updated to reflect the new task completion"){
        REQUIRE(host_taxon->GetData().GetIntVal() == 1);
      }
    }

    WHEN("A host is born and it has a different task profile than its parent"){
      sgp_host_t& host = static_cast<sgp_host_t&>(world.GetOrg(0));
      host.GetHardware().GetCPUState().MarkTaskPerformed(1);
      
      emp::Ptr<Organism> host_offspring = host.Reproduce();

      world.AddOrgAt(host_offspring, 1, 0);

      emp::Ptr<taxon_t::base_taxon_t> host_taxon = host.GetTaxon();
      emp::Ptr<taxon_t::base_taxon_t> host_offspring_taxon = host_offspring->GetTaxon();
      
      world.Update();

      THEN("The child host's taxon is different than the parent's"){
        REQUIRE(host_taxon->GetInfo() != host_offspring_taxon->GetInfo());
        REQUIRE(host_taxon->GetID() != host_offspring_taxon->GetID());
      }

      THEN("The child taxon tracks the parent taxon"){
        REQUIRE(host_offspring_taxon->GetParent() == host_taxon);
      }

      THEN("Both the child and parent taxon track task performance correctly"){
        REQUIRE(host_taxon->GetData().GetIntVal() == 2);
        REQUIRE(host_offspring_taxon->GetData().GetIntVal() == 0);
      }
    }
    
    WHEN("A host is born and it has the same task profile as its parent"){
      sgp_host_t& host = static_cast<sgp_host_t&>(world.GetOrg(0));

      emp::Ptr<Organism> host_offspring = host.Reproduce();
      world.AddOrgAt(host_offspring, 1, 0);

      emp::Ptr<taxon_t::base_taxon_t> host_taxon = host.GetTaxon();
      emp::Ptr<taxon_t::base_taxon_t> host_offspring_taxon = host_offspring->GetTaxon();

      world.Update();
      
      THEN("The child host's taxon is different than the parent's"){
        REQUIRE(host_taxon->GetInfo() != host_offspring_taxon->GetInfo());
        REQUIRE(host_taxon->GetID() != host_offspring_taxon->GetID());
      }

      THEN("The child taxon tracks the parent taxon"){
        REQUIRE(host_offspring_taxon->GetParent() == host_taxon);
      }

      THEN("Both the child and parent taxon track task performance correctly"){
        REQUIRE(host_taxon->GetData().GetIntVal() == 0);
        REQUIRE(host_offspring_taxon->GetData().GetIntVal() == 0);
      }
    }
  }
}


TEST_CASE("Symbiont phylogenies track correctly when PHYLOGENY_TAXON_TYPE is individual and tracking task profiles (5)", "[sgp]"){
  using world_t = sgpmode::SGPWorld;
  using cpu_state_t = sgpmode::CPUState<world_t>;
  using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
  using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
  
  emp::Random random(61);
  sgpmode::SymConfigSGP config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.POP_SIZE(4);
  config.START_MOI(1);
  config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");
  config.OUSTING(1);

  config.TASK_PROFILE_MODE("self-all");

  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE(5);
  config.STORE_EXTINCT(0);

  GIVEN("A world full of hosted symbionts"){
    world_t world(random, &config);
    world.Setup();

    emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::SymbiontTaxonData>> sym_sys = world.GetSymSys();
    
    THEN("Taxa are setup for all initiating symbionts"){
      REQUIRE(sym_sys->GetNumActive() == 4);
    }
    
    WHEN("A symbiont dies "){
      emp::Ptr<Organism> host = world.GetOrgPtr(0);
      emp::Ptr<Organism> symbiont = host->GetSymbionts().at(0);

      symbiont->SetDead();
      world.Update();

      THEN("Its taxon is removed from the symbiont systematic"){
        REQUIRE(sym_sys->GetNumActive() == 3);
      }
    }

    WHEN("A symbiont completes a new task"){
      sgp_sym_t& symbiont = static_cast<sgp_sym_t&>(*world.GetOrgPtr(0)->GetSymbionts().at(0));
      symbiont.GetHardware().GetCPUState().MarkTaskPerformed(2);
      symbiont.GetHardware().GetCPUState().MarkTaskPerformed(1);
      symbiont.GetHardware().GetCPUState().MarkTaskPerformed(0);

      emp::Ptr<taxon_t::base_taxon_t> symbiont_taxon = symbiont.GetTaxon();
      world.Update();

      THEN("Its taxon data is updated to reflect the new task completion"){
        REQUIRE(symbiont_taxon->GetData().GetIntVal() == 7);
      }
    }

    WHEN("A symbiont is born and it has a different task profile than its parent"){
      sgp_sym_t& symbiont = static_cast<sgp_sym_t&>(*world.GetOrgPtr(0)->GetSymbionts().at(0));
      symbiont.GetHardware().GetCPUState().ResetTaskPerformance(1);
      
      emp::Ptr<Organism> symbiont_offspring = symbiont.Reproduce();
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateReproProgram(100));
      
      symbiont.GetHardware().GetCPUState().MarkTaskPerformed(0);
      static_cast<sgp_sym_t&>(*symbiont_offspring).GetHardware().GetCPUState().MarkTaskPerformed(2);

      emp::Ptr<taxon_t::base_taxon_t> symbiont_taxon = symbiont.GetTaxon();
      emp::Ptr<taxon_t::base_taxon_t> symbiont_offspring_taxon = symbiont_offspring->GetTaxon();
      
      host->AddSymbiont(symbiont_offspring);
      world.AddOrgAt(host, 1);

      world.Update();

      THEN("The child symbiont's taxon is different than the parent's"){
        REQUIRE(symbiont_taxon->GetInfo() != symbiont_offspring_taxon->GetInfo());
        REQUIRE(symbiont_taxon->GetID() != symbiont_offspring_taxon->GetID());
      }

      THEN("The child taxon tracks the parent taxon"){
        REQUIRE(symbiont_offspring_taxon->GetParent() == symbiont_taxon);
      }

      THEN("Both the child and parent taxon track task performance correctly"){
        REQUIRE(symbiont_taxon->GetData().GetIntVal() == 1);
        REQUIRE(symbiont_offspring_taxon->GetData().GetIntVal() == 4);
      }
    }
    
    WHEN("A host is born and it has the same task profile as its parent"){
      sgp_sym_t& symbiont = static_cast<sgp_sym_t&>(*world.GetOrgPtr(0)->GetSymbionts().at(0));
      symbiont.GetHardware().GetCPUState().ResetTaskPerformance(1);

      emp::Ptr<Organism> symbiont_offspring = symbiont.Reproduce();
      emp::Ptr<sgp_host_t> host = emp::NewPtr<sgp_host_t>(&random, &world, &config, world.GetProgramBuilder().CreateReproProgram(100));

      emp::Ptr<taxon_t::base_taxon_t> symbiont_taxon = symbiont.GetTaxon();
      emp::Ptr<taxon_t::base_taxon_t> symbiont_offspring_taxon = symbiont_offspring->GetTaxon();
      
      host->AddSymbiont(symbiont_offspring);
      world.AddOrgAt(host, 1);

      world.Update();

      THEN("The child symbiont's taxon is different than the parent's"){
        REQUIRE(symbiont_taxon->GetInfo() != symbiont_offspring_taxon->GetInfo());
        REQUIRE(symbiont_taxon->GetID() != symbiont_offspring_taxon->GetID());
      }

      THEN("The child taxon tracks the parent taxon"){
        REQUIRE(symbiont_offspring_taxon->GetParent() == symbiont_taxon);
      }

      THEN("Both the child and parent taxon track task performance correctly"){
        REQUIRE(symbiont_taxon->GetData().GetIntVal() == 0);
        REQUIRE(symbiont_offspring_taxon->GetData().GetIntVal() == 0);
      }  
    }
  }
}