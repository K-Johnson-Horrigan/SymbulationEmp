#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../../Empirical/include/emp/config/ArgManager.hpp"
#include "../ConfigSetup.h"
#include "../default_mode/DataNodes.h"
#include "../default_mode/WorldSetup.cc"
#include "../metapop_mode/MetapopWorld.h"
#include "../metapop_mode/MetapopDataNodes.h"
#include "../metapop_mode/MetapopWorldSetup.cc"
#include "../sgp_mode/SGPDataNodes.h"
#include "../sgp_mode/SGPWorldSetup.cc"
#include "../sgp_mode/Tasks.cc"
#include "symbulation.h"

// This is the main function for the NATIVE version of this project.
int symbulation_main(int argc, char* argv[]) {
  SymConfigBase config;
  CheckConfigFile(config, argc, argv);

  config.Write(std::cout);
  emp::Random random(config.SEED());

  config.GENERATIONS(2);
  config.NUM_POPULATIONS(3);

  MetapopWorld world(random, &config);
  world.Populate();
  world.CreateDataFiles();

  for (size_t i = 0; i < config.GENERATIONS(); i++) {
    if (i % config.METAPOP_DATA_INT() == 0) {
      std::cout << "Update: " << i << std::endl;
    }
    world.Update();
  }
  return 0;
}

/*
This definition guard prevents main from being defined twice during testing.
In testing, Catch will define a main function which will initiate tests
(including testing the symbulation_main function above).
*/
#ifndef CATCH_CONFIG_MAIN
int main(int argc, char* argv[]) { return symbulation_main(argc, argv); }
#endif
