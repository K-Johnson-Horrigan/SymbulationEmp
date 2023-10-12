#ifndef METAPOP_WORLD_SETUP_C
#define METAPOP_WORLD_SETUP_C

#include "../ConfigSetup.h"
#include "../sgp_mode/SGPWorld.h"
#include "MetapopWorld.h"

/**
 * Input: The number of SGP world to fill the metapopulation with.
 *
 * Output: None
 *
 * Purpose: To set up the populations that will be sampled.
 */
void MetapopWorld::Populate() {
  for (size_t i = 0; i < my_config->NUM_POPULATIONS(); i++) {
    TaskSet task_set = LogicTasks;
    if (my_config->TASK_TYPE() == 0) {
      task_set = SquareTasks;
    } else if (my_config->TASK_TYPE() == 1) {
      task_set = LogicTasks;
    }
    emp::Ptr<SGPWorld> sgp_world = emp::NewPtr<SGPWorld>(*random_ptr, my_config, task_set);

    sgp_world->Populate();
    AddOrgAt(sgp_world, num_orgs);
  }
}

#endif