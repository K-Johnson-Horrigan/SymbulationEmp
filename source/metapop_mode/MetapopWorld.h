#ifndef METAPOP_WORLD_H
#define METAPOP_WORLD_H


#include "../sgp_mode/SGPWorld.h"


class MetapopWorld : public emp::World<SGPWorld>{
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
  MetapopWorld(emp::Random & _random, emp::Ptr<SymConfigBase> _config) : emp::World<SGPWorld>(_random) {
    my_config = _config;
  }


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the objects belonging to MetapopWorld to conserve memory.
   */
  ~MetapopWorld() {
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
  void Update(){
    std::cout << "Under construction" << std::endl;
  }

};// MetapopWorld class
#endif
