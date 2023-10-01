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
void MetapopWorld::Populate(int num_worlds) {
  std::cout << "hi!" << std::endl;

//   //     // inject subworlds
//   for (int i = 0; i < num_worlds; i++) {
//     TaskSet task_set = LogicTasks;
//     if (my_config->TASK_TYPE() == 0) {
//       task_set = SquareTasks;
//     } else if (my_config->TASK_TYPE() == 1) {
//       task_set = LogicTasks;
//     }
//     SGPWorld sgp_world(*random_ptr, my_config, task_set);
//     sgp_world.Populate();

//     // todo: control for world_size
//     pop.push_back(&sgp_world);
//     num_orgs++;

//     // todo: use aoa (can't bc aoa tries to call empirical macro setup)
//     // AddOrgAt(&sgp_world, num_orgs);
//   }


  //     SGPWorld sgp_world(random, &config, task_set);

  //     sgp_world.Populate();
  //     AddOrgAt(&sgp_world, world->GetNumOrgs());
  //   }
}

#endif