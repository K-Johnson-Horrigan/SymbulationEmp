#ifndef METAPOP_WORLD_H
#define METAPOP_WORLD_H

#include "../sgp_mode/SGPWorld.h"
#include "../sgp_mode/SGPWorldSetup.cc"

class MetapopWorld : public emp::World<SGPWorld> {
 protected:
  /**
   *
   * Purpose: Represents the configuration settings for a particular run.
   *
   */
  emp::Ptr<SymConfigBase> my_config = NULL;

 public:
  /**
   * Input: The world's random seed
   *
   * Output: None
   *
   * Purpose: To construct an instance of MetapopWorld
   */
  MetapopWorld(emp::Random &_random, emp::Ptr<SymConfigBase> _config)
      : emp::World<SGPWorld>(_random) {
    my_config = _config;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the objects belonging to MetapopWorld to conserve
   * memory.
   */
  ~MetapopWorld() { std::cout << "so wassa problem here" << std::endl; }

  /**
   * Definition of a setup functions, expanded in MetapopWorldSetup.cc
   */
  void Populate(int num_worlds);

  /**
   * Input: The number of SGP world to fill the metapopulation with.
   *
   * Output: None
   *
   * Purpose: To set up the populations that will be sampled.
   */
  void Setup(int num_worlds) {
    std::cout << "hi!" << std::endl;

    //     // inject subworlds
    for (int i = 0; i < num_worlds; i++) {
      TaskSet task_set = LogicTasks;
      if (my_config->TASK_TYPE() == 0) {
        task_set = SquareTasks;
      } else if (my_config->TASK_TYPE() == 1) {
        task_set = LogicTasks;
      }
      SGPWorld sgp_world(*random_ptr, my_config, task_set);
      // if I run update here it's fine, but then if I run it later it runs symworld::update
      //sgp_world.Update();

      //sgp_world.Populate()
      AddOrgAt(&sgp_world, num_orgs);
      // todo: control for world_size
      // pop.push_back(&sgp_world);
      // num_orgs++;
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To simulate a generation in the world, which includes letting
   * all of the subworld develop, then sampling the highest-performing to
   * produce the next generation.
   */  
  void Update() {
    //std::cout << pop[0] << std::endl;
    pop[0]->Update();
    std::cout << "Under construction" << std::endl;
  }

};  // MetapopWorld class
#endif
