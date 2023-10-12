#ifndef METAPOP_WORLD_H
#define METAPOP_WORLD_H

#include "../sgp_mode/SGPWorld.h"
#include "../sgp_mode/SGPWorldSetup.cc"
//TODO data node tests
class MetapopWorld : public emp::World<SGPWorld> {
 protected:
  /**
   *
   * Purpose: Represents the configuration settings for a particular run.
   *
   */
  emp::Ptr<SymConfigBase> my_config = NULL;

  /**
   *
   * Purpose: Data nodes which collect information pertaining to this
   * experiment.
   *
   */
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;

 public:
  /**
   * Input: The world's random seed
   *
   * Output: None
   *
   * Purpose: To construct an instance of MetapopWorld
   */
  MetapopWorld(emp::Random& _random, emp::Ptr<SymConfigBase> _config)
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
  ~MetapopWorld() {
    if (data_node_symcount) data_node_symcount.Delete();
    if (data_node_hostcount) data_node_hostcount.Delete();
  }

  /**
   * Definition of a setup functions, expanded in MetapopWorldSetup.cc
   */
  void Populate();

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
    // fully evolve each world
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
    for (size_t i : schedule) {
      if (IsOccupied(i)) {
        pop[i]->RunExperiment(false);
      }
    }

    // run emp world update to get all the data file stuff
    // do this AFTER the experiments in order to get the result of the
    // updateth set of world evolutions before we sample for
    // the next generation of worlds
    emp::World<SGPWorld>::Update();

    // select next generation of worlds
    // can use SerialTransfer() method from emp for rand select but want to be
    // sure to resize it the way I want (fixed pop size)
    // serial transfer asks for proportion, elite etc in world_select
    // asks for counts (?)
    schedule = emp::GetPermutation(GetRandom(), GetSize());
    for (size_t i : schedule) {
      if (IsOccupied(i)) {
        // TODO CHANGE THIS TO A VARIABLE (cur 0.01 of 10k = 100)
        pop[i]->SerialTransfer(0.01);
        pop[i]->Resize(my_config->GRID_X(), my_config->GRID_Y());
      }
    }
  }

  /**
   * Data node methods expanded in MetapopDataNodes.h
   */
  emp::DataFile& CreateDataFiles();
  emp::DataMonitor<int>& GetSymCountDataNode();
  emp::DataMonitor<int>& GetHostCountDataNode();

};  // MetapopWorld class
#endif
