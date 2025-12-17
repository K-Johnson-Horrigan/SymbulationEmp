#include <filesystem>
#include <fstream>
#include <string>

TEST_CASE("Correct data files are created", "[sgp]") {
  
  GIVEN("A World with no mutation"){
    emp::Random random(1);
    SymConfigSGP config;
    config.SEED(2);
    config.INTERACTION_MECHANISM(0);
    config.SYMBIONT_TYPE(1);
    config.MUTATION_RATE(0.0);
    config.MUTATION_SIZE(0.000);
    config.FILE_NAME("DataTest");
    config.CPU_TRANSFER_CHANCE(0.8);
    config.SYM_HORIZ_TRANS_RES(0);



    WHEN("The World is ran for 200 Updates with 1 host"){
    SGPWorld world(random, &config, LogicTasks);

    //Creates a host that only does NOT operations
    emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

    //Adds host to world
    world.AddOrgAt(host, 0);
    world.CreateDataFiles();
    for (int i = 0; i < 200; i++){
      world.Update();
    }
    
    world.WriteTaskCombinationsFile("EndingTaskCombinationsDataTest_SEED2.data");
    world.WriteOrgReproHistFile("OrgReproHistDataTest_SEED2.data");
    THEN("All correct data files are created"){
      REQUIRE(std::filesystem::exists("OrganismCountsDataTest_SEED2.data"));
      REQUIRE(std::filesystem::exists("TasksDataTest_SEED2.data"));
      REQUIRE(std::filesystem::exists("TransmissionRatesDataTest_SEED2.data"));

      REQUIRE(std::filesystem::exists("EndingTaskCombinationsDataTest_SEED2.data"));
      REQUIRE(std::filesystem::exists("OrgReproHistDataTest_SEED2.data"));
      REQUIRE(std::filesystem::exists("SymInstCountDataTest_SEED2.data"));
    }
    // Create a text string, which is used to output the text file
    std::ifstream file("OrganismCountsDataTest_SEED2.data");
    std::string str; 
    THEN("The OrganismCount file should contain 3 lines"){
      std::getline(file, str);
      THEN("The first should be a header"){
        REQUIRE(str == "update,count,hosted_syms");
      }
      std::getline(file, str);
      THEN("The second should be 1 host alive at update 0"){
        REQUIRE(str == "0,1,0");
      }
      std::getline(file, str);
      THEN("The third should be 1 host alive at update 100"){
        REQUIRE(str == "100,1,0");
      }
    }
    

    std::ifstream file3("TasksDataTest_SEED2.data");
    std::string str3; 
    THEN("The Task File should contain 3 lines"){
        std::getline(file3, str3);
        THEN("The first should be a header"){
          REQUIRE(str3 == "update,host_task_NOT,sym_task_NOT,host_task_NAND,sym_task_NAND,host_task_AND,sym_task_AND,host_task_ORN,sym_task_ORN,host_task_OR,sym_task_OR,host_task_ANDN,sym_task_ANDN,host_task_NOR,sym_task_NOR,host_task_XOR,sym_task_XOR,host_task_EQU,sym_task_EQU");
        }
        std::getline(file3, str3);
        THEN("The second should be all zeroes because at 0 updates in no tasks should have been completed"){
          REQUIRE(str3 == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
        std::getline(file3, str3);
        THEN("The third should contain 4 NOT host tasks because in 100 updates 1 host can perform their genome 4 times"){
          REQUIRE(str3 == "100,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }
    }

    std::ifstream file4("TransmissionRatesDataTest_SEED2.data");
    std::string str4; 
    THEN("The TransmissionRates File should contain 3 lines"){
        std::getline(file4, str4);
        THEN("The first should be a header"){
          REQUIRE(str4 == "update,attempts_horiztrans,successes_horiztrans,attempts_verttrans,successes_verttrans");
        }
        std::getline(file4, str4);
        THEN("The second should be all zeroes because there are no symbionts"){
          REQUIRE(str4 == "0,0,0,0,0");
        }
        std::getline(file4, str4);
        THEN("The third should be at 100 updates and the rest be zero because there are no symbionts"){
          REQUIRE(str4 == "100,0,0,0,0");
        }
    }

    std::ifstream file5("EndingTaskCombinationsDataTest_SEED2.data");
    std::string str5; 
    THEN("The EndingTaskCombinations File should contain 3 lines"){
        std::getline(file5, str5);
        THEN("The first should be a header"){
          REQUIRE(str5 == "task_completions,host_parent_count,sym_parent_count,host_count,symbiont_count");
        }
        std::getline(file5, str5);
        THEN("The second should be 1 host who completed NOT") {
          REQUIRE(str5 == "000000001,0,0,1,0");
        }
        std::getline(file5, str5);
        THEN("The third should be 1 host parent who completed nothing"){
          REQUIRE(str5 == "000000000,1,0,0,0");
        }
    }
  }

    WHEN("The World is ran for 200 Updates with 2 hosts and 1 symbiont"){
      SGPWorld world(random, &config, LogicTasks);

      //Creates a host and sym that only do NOT operations
      emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));
      emp::Ptr<SGPHost> host2 = emp::NewPtr<SGPHost>(&random, &world, &config, CreateNotProgram(100));

      emp::Ptr<SGPSymbiont> sym = emp::NewPtr<SGPSymbiont>(&random, &world, &config, CreateNotProgram(100));

      //Adds host to world and sym to host.
      world.AddOrgAt(host, 0);
      world.AddOrgAt(host2, 1);

      host->AddSymbiont(sym);

      world.CreateDataFiles();
      for (int i = 0; i < 200; i++){
        world.Update();
      }
      
      
      world.WriteTaskCombinationsFile("EndingTaskCombinationsDataTest_SEED2.data");
      world.WriteOrgReproHistFile("OrgReproHistDataTest_SEED2.data");
      THEN("All correct data files are created"){
        REQUIRE(std::filesystem::exists("OrganismCountsDataTest_SEED2.data"));
        REQUIRE(std::filesystem::exists("TasksDataTest_SEED2.data"));
        REQUIRE(std::filesystem::exists("TransmissionRatesDataTest_SEED2.data"));

        REQUIRE(std::filesystem::exists("EndingTaskCombinationsDataTest_SEED2.data"));
        REQUIRE(std::filesystem::exists("OrgReproHistDataTest_SEED2.data"));
      }
      // Create a text string, which is used to output the text file
      std::ifstream file("OrganismCountsDataTest_SEED2.data");
      std::string str; 
      THEN("The OrganismCount file should contain 3 lines"){
        std::getline(file, str);
        THEN("The first should be a header"){
          REQUIRE(str == "update,count,hosted_syms");
        }
        std::getline(file, str);
        THEN("The second should be 2 hosts and 1 sym alive at update 0"){
          REQUIRE(str == "0,2,1");
        }
        std::getline(file, str);
        THEN("The third should be 2 hosts and 2 sym alive at update 100"){
          REQUIRE(str == "100,2,2");
        }
      }
    

      std::ifstream file3("TasksDataTest_SEED2.data");
      std::string str3; 
      THEN("The Task File should contain 3 lines"){
        std::getline(file3, str3);
          THEN("The first should be a header"){
            REQUIRE(str3 == "update,host_task_NOT,sym_task_NOT,host_task_NAND,sym_task_NAND,host_task_AND,sym_task_AND,host_task_ORN,sym_task_ORN,host_task_OR,sym_task_OR,host_task_ANDN,sym_task_ANDN,host_task_NOR,sym_task_NOR,host_task_XOR,sym_task_XOR,host_task_EQU,sym_task_EQU");
          }
          std::getline(file3, str3);
          THEN("The second should be all zeroes because at 0 updates in no tasks should have been completed"){
            REQUIRE(str3 == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
          }
          std::getline(file3, str3);
          THEN("The third should contain 8 NOT host tasks and 7 NOT sym tasks"){
            REQUIRE(str3 == "100,8,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
          }
      }

      std::ifstream file4("TransmissionRatesDataTest_SEED2.data");
      std::string str4; 
      THEN("The TransmissionRates File should contain 3 lines"){
          std::getline(file4, str4);
          THEN("The first should be a header"){
            REQUIRE(str4 == "update,attempts_horiztrans,successes_horiztrans,attempts_verttrans,successes_verttrans");
          }
          std::getline(file4, str4);
          THEN("The second should be all zeroes because the syms have been unable to attempt transmissions yet"){
            REQUIRE(str4 == "0,0,0,0,0");
          }
          std::getline(file4, str4);
          THEN("The third should be at 100 updates with 7 attempts and 1 success"){
            REQUIRE(str4 == "100,7,1,0,0");
          }
      }

      std::ifstream file5("EndingTaskCombinationsDataTest_SEED2.data");
      std::string str5; 
      THEN("The EndingTaskCombinations File should contain 3 lines"){
        std::getline(file5, str5);
        THEN("The first should be a header"){
          REQUIRE(str5 == "task_completions,host_parent_count,sym_parent_count,host_count,symbiont_count");
        }
        std::getline(file5, str5);
        THEN("The second should be 1 symbiont parent, 2 hosts, and 2 symbionts who completed solely the NOT task"){
          REQUIRE(str5 == "000000001,0,1,2,2");
        }
        std::getline(file5, str5);
        THEN("The third should be 2 hosts and 1 symbiont whose parents completed nothing"){
          REQUIRE(str5 == "000000000,2,1,0,0");
        }
      }


      std::filesystem::remove("OrganismCountsDataTest_SEED2.data");
      std::filesystem::remove("TasksDataTest_SEED2.data");
      std::filesystem::remove("TransmissionRatesDataTest_SEED2.data");

      std::filesystem::remove("EndingTaskCombinationsDataTest_SEED2.data");
      std::filesystem::remove("OrgReproHistDataTest_SEED2.data");
      std::filesystem::remove("SymInstCountDataTest_SEED2.data");

      THEN("All previously created data files have been removed"){
        REQUIRE(!std::filesystem::exists("OrganismCountsDataTest_SEED2.data"));
        REQUIRE(!std::filesystem::exists("TasksDataTest_SEED2.data"));
        REQUIRE(!std::filesystem::exists("TransmissionRatesDataTest_SEED2.data"));

        REQUIRE(!std::filesystem::exists("EndingTaskCombinationsDataTest_SEED2.data"));
        REQUIRE(!std::filesystem::exists("OrgReproHistDataTest_SEED2.data"));
        REQUIRE(!std::filesystem::exists("SymInstCountDataTest_SEED2.data"));
      }

    }

  }
}

TEST_CASE("Death proportion file is created", "[sgp]") {
  emp::Random random(2);
  SymConfigSGP config;
  config.GRID_X(10);
  config.GRID_Y(10);
  config.DATA_INT(100);
  config.INTERACTION_MECHANISM(STRESS);
  config.SYMBIONT_TYPE(0);
  config.EXTINCTION_FREQUENCY(25);
  config.BASE_DEATH_CHANCE(0.5);
  config.MUTUALIST_DEATH_CHANCE(0);
  config.TRACK_EXTINCTION_DEATH_PROPORTION(1);

  SGPWorld world(random, &config, LogicTasks);

  std::string filename = "DataTest_StressDeathProportion.data";
  world.SetupDeathProportionFile(filename);
  THEN("The death proportion file is created") {
    REQUIRE(std::filesystem::exists(filename));
  }
  
  WHEN("Base death chance is 0.5 & there are no mutualists") {
    config.START_MOI(0);
    world.Setup();
    world.RunExperiment(false);
    THEN("Hosts die at consistent proportions"){
      std::ifstream file(filename);
      std::string str;
      THEN("The StressDeathProportion file should contain 5 lines") {
        std::getline(file, str);
        THEN("The first should be a header") {
          REQUIRE(str == "update,pre_ex_host_count,post_ex_host_count,death_proportion");
        }
        std::getline(file, str);
        THEN("The second should be (around) half the host population dying at update 25") {
          REQUIRE(str == "25,100,50,0.5");
        }
        std::getline(file, str);
        THEN("The third should be (around) half the host population dying at update 50") {
          REQUIRE(str == "50,50,22,0.56");
        }
        std::getline(file, str);
        THEN("The fourth should be (around) half the host population dying at update 75") {
          REQUIRE(str == "75,22,12,0.454545");
        }
        std::getline(file, str);
        THEN("The fifth should be (around) half the host population dying at update 100") {
          REQUIRE(str == "100,12,6,0.5");
        }
      }
    }
  }

  WHEN("Base death chance is 0.5 & there are mutualists") {
    config.START_MOI(1);
    world.Setup();
    world.RunExperiment(false);
    THEN("Hosts die rapidly at first, then their death proportions stabilize") {
      std::ifstream file(filename);
      std::string str;
      THEN("The StressDeathProportion file should contain 5 lines") {
        std::getline(file, str);
        THEN("The first should be a header") {
          REQUIRE(str == "update,pre_ex_host_count,post_ex_host_count,death_proportion");
        }
        std::getline(file, str);
        THEN("The second should be (around) half the host population dying at update 25") {
          REQUIRE(str == "25,100,50,0.5");
        }
        std::getline(file, str);
        THEN("The third should be none of the host population dying at update 50") {
          REQUIRE(str == "50,50,50,0");
        }
        std::getline(file, str);
        THEN("The fourth should be none of the host population dying at update 75") {
          REQUIRE(str == "75,50,50,0");
        }
        std::getline(file, str);
        THEN("The fifth should be none of the host population dying at update 100") {
          REQUIRE(str == "100,50,50,0");
        }
      }
    }
  }

  std::filesystem::remove(filename);
  THEN("The death proportion file is removed") {
    REQUIRE(!std::filesystem::exists(filename));
  } 
}

TEST_CASE("Death proportion file is used to run manual extinction experiment", "[sgp]") {
  emp::Random random(2);
  SymConfigSGP config;
  config.GRID_X(10);
  config.GRID_Y(10);
  config.DATA_INT(100);
  config.INTERACTION_MECHANISM(STRESS);
  config.SYMBIONT_TYPE(0);
  config.EXTINCTION_FREQUENCY(25);
  config.BASE_DEATH_CHANCE(0.5);
  config.MUTUALIST_DEATH_CHANCE(0);
  config.TRACK_EXTINCTION_DEATH_PROPORTION(1);

  SGPWorld world(random, &config, LogicTasks);

  std::string filename = "DataTest_StressDeathProportion.data";
  world.SetupDeathProportionFile(filename);
  THEN("The death proportion file is created") {
    REQUIRE(std::filesystem::exists(filename));
  }

  config.START_MOI(0);
  world.Setup();
  world.RunExperiment(false);
  THEN("Hosts die at consistent proportions") {
    std::ifstream file(filename);
    std::string str;
    THEN("The StressDeathProportion file should contain 5 lines") {
      std::getline(file, str);
      THEN("The first should be a header") {
        REQUIRE(str == "update,pre_ex_host_count,post_ex_host_count,death_proportion");
      }
      std::getline(file, str);
      THEN("The second should be (around) half the host population dying at update 25") {
        REQUIRE(str == "25,100,50,0.5");
      }
      std::getline(file, str);
      THEN("The third should be (around) half the host population dying at update 50") {
        REQUIRE(str == "50,50,22,0.56");
      }
      std::getline(file, str);
      THEN("The fourth should be (around) half the host population dying at update 75") {
        REQUIRE(str == "75,22,12,0.454545");
      }
      std::getline(file, str);
      THEN("The fifth should be (around) half the host population dying at update 100") {
        REQUIRE(str == "100,12,6,0.5");
      }
    }
  }
  
  
  config.INTERACTION_MECHANISM(STRESS_MANUAL_KILL);
  config.KILL_HOSTS_PER_EXTINCTION_FILE(1);
  config.SOURCE_EXTINCTION_PROPORTION_FILE_NAME(filename);

  // SOURCE_EXTINCTION_PROPORTION_FILE_NAME gets assigned to file pointer in SGPWorld constructor
  SGPWorld manual_extinction_world(random, &config, LogicTasks);
  
  
  
  std::string manual_extinction_filename = "DataTest_StressDeathProportion_manual_extinction_data.data";
  manual_extinction_world.SetupDeathProportionFile(manual_extinction_filename);
    
  manual_extinction_world.Setup();
  manual_extinction_world.RunExperiment(false);
  
  THEN("Survivor host counts are the exact same in the manual extinction experiment as they were in the stress experiment") {
    std::ifstream file(manual_extinction_filename);
    std::string str;
    THEN("The manual extinction StressDeathProportion file should contain 5 lines") {
      std::getline(file, str);
      THEN("The first should be a header") {
        REQUIRE(str == "update,pre_ex_host_count,post_ex_host_count,death_proportion");
      }
      std::getline(file, str);
      THEN("The second should be (around) half the host population dying at update 25") {
        REQUIRE(str == "25,100,50,0.5");
      }
      std::getline(file, str);
      THEN("The third should be (around) half the host population dying at update 50") {
        REQUIRE(str == "50,50,22,0.56");
      }
      std::getline(file, str);
      THEN("The fourth should be (around) half the host population dying at update 75") {
        REQUIRE(str == "75,22,12,0.454545");
      }
      std::getline(file, str);
      THEN("The fifth should be (around) half the host population dying at update 100") {
        REQUIRE(str == "100,12,6,0.5");
      }
    }
  }
  
  std::filesystem::remove(manual_extinction_filename);  
  std::filesystem::remove(filename);
  THEN("The death proportion file is removed") {
    REQUIRE(!std::filesystem::exists(filename));
    REQUIRE(!std::filesystem::exists(manual_extinction_filename));
  }
}

TEST_CASE("GetStressEscapeeOffspringAttemptCount", "[sgp]") {
  GIVEN("Stress is on, parasites are present, and an extinction event occurs") {
    emp::Random random(32);
    SymConfigSGP config;

    size_t stress_escapee_offspring_count = 3;
    config.EXTINCTION_FREQUENCY(1);
    config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(stress_escapee_offspring_count);
    config.TRACK_PARENT_TASKS(1);
    config.INTERACTION_MECHANISM(STRESS);
    config.SYMBIONT_TYPE(1);
    config.PARASITE_DEATH_CHANCE(1);

    SGPWorld world(random, &config, LogicTasks);

    emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    host->AddSymbiont(matching_symbiont);

    host->GetCPU().state.parent_tasks_performed->Set(1);
    matching_symbiont->GetCPU().state.parent_tasks_performed->Set(1); 

    world.AddOrgAt(host, 0);

    WHEN("Parasite offspring escape") {
      world.Update();
      REQUIRE(world.IsOccupied(0) == false);

      emp::DataMonitor<size_t>& escapee_attempts = world.GetStressEscapeeOffspringAttemptCount();
      THEN("The number of attempted stress escapees is recorded"){
        REQUIRE(escapee_attempts.GetTotal() == stress_escapee_offspring_count);
      }
    }
  }
}

TEST_CASE("GetStressEscapeeOffspringSuccessCount", "[sgp]") {
  GIVEN("Stress is on, parasites are present, and an extinction event occurs") {
    emp::Random random(32);
    SymConfigSGP config;

    size_t stress_escapee_offspring_count = 3;
    config.EXTINCTION_FREQUENCY(1);
    config.PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION(stress_escapee_offspring_count);
    config.TRACK_PARENT_TASKS(1);
    config.INTERACTION_MECHANISM(STRESS);
    config.SYMBIONT_TYPE(1);
    config.BASE_DEATH_CHANCE(0);
    config.PARASITE_DEATH_CHANCE(1);
    config.GRID(1);

    SGPWorld world(random, &config, LogicTasks);

    emp::Ptr<StressHost> host = emp::NewPtr<StressHost>(&random, &world, &config);
    emp::Ptr<SGPSymbiont> matching_symbiont = emp::NewPtr<SGPSymbiont>(&random, &world, &config);

    emp::Ptr<StressHost> vulnerable_host = emp::NewPtr<StressHost>(&random, &world, &config);

    host->AddSymbiont(matching_symbiont);

    host->GetCPU().state.parent_tasks_performed->Set(1);
    matching_symbiont->GetCPU().state.parent_tasks_performed->Set(1);
    matching_symbiont->GetCPU().state.tasks_performed->Set(0);
    vulnerable_host->GetCPU().state.parent_tasks_performed->Set(1);

    world.AddOrgAt(host, 0);
    world.AddOrgAt(vulnerable_host, 1);

    WHEN("Parasite offspring escape") {
      world.Update();
      REQUIRE(world.IsOccupied(0) == false);
      REQUIRE(world.IsOccupied(1) == true);
      REQUIRE(vulnerable_host->HasSym());
      REQUIRE(vulnerable_host->GetSymbionts().at(0).DynamicCast<SGPSymbiont>()->GetCPU().state.parent_tasks_performed->Get(0) == 1);
      emp::DataMonitor<size_t>& escapee_successes = world.GetStressEscapeeOffspringSuccessCount();
      THEN("The number of attempted stress escapees is recorded") {
        REQUIRE(escapee_successes.GetTotal() == 1);
      }
    }
  }
}