#ifndef METAPOP_WORLD_H
#define METAPOP_WORLD_H

#include "../sgp_mode/SGPWorld.h"
#include "../sgp_mode/SGPWorldSetup.cc"
// TODO data node tests
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
  emp::vector<emp::DataMonitor<size_t>> data_node_host_tasks;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_tasks;

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

    // define what is "fit"
    fun_calc_fitness_t fit_fun = [&](SGPWorld& population) {
      // reference to an organism and return a fitness value of type double.
      double fitness_val = population.GetNumOrgs();
      return fitness_val;
    };
    SetFitFun(fit_fun);
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
   * Purpose: To randomly select the next generation of organisms for each
   * population.
   */
  void SampleRandom() {
    emp_assert(my_config->SELECTION_SCHEME() == 0);

    if (my_config->RANDOM_SELECTION_SCHEME() == 0) {
      // using SerialTransfer() method from emp for rand select
      emp::vector<size_t> schedule =
          emp::GetPermutation(GetRandom(), GetSize());
      for (size_t i : schedule) {
        if (IsOccupied(i)) {
          pop[i]->to_reproduce.clear();
          pop[i]->SerialTransfer(my_config->SAMPLE_PROPORTION());
          pop[i]->Resize(my_config->GRID_X(), my_config->GRID_Y());
        }
      }
    } else if (my_config->RANDOM_SELECTION_SCHEME() == 1) {
      // each world samples itself randomly
      for (size_t i = 0; i < size(); i++) {
        SampleSelf(i);
      }
    } else if (my_config->RANDOM_SELECTION_SCHEME() == 2) {
      // each world is sampled from one random world
      size_t source_pop = GetRandomOrgID();
      for (size_t i = 0; i < size(); i++) {
        if (i == source_pop) continue;  // skip the source world
        SampleSource(i, source_pop);
      }
      SampleSelf(source_pop);
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To truncate select the next generation of organisms for each
   * population.
   */
  void SampleTruncate() {
    // NOTE: use the fitness map in world_select to get multiple best worlds
    // pair: fitness, index
    std::pair<double, size_t> best_pop(-1, -1);
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
    for (size_t i : schedule) {
      emp_assert(IsOccupied(i));  // there should be no empty cells in metapop world
      double fitness = CalcFitnessID(i);
      if (fitness > best_pop.first) {
        best_pop = std::make_pair(fitness, i);
      }
    }
    size_t best_i = best_pop.second;

    // empty all not-best worlds & inject best orgs into the
    for (size_t i = 0; i < size(); i++) {
      if (i == best_i) continue;  // skip the best world
      SampleSource(i, best_i);
    }

    // sample best world
    SampleSelf(best_i);
  }

  /**
   * Input: The position of the population from which to copy a random host,
   * the position of the world which the new organisms (and their cpus) should
   * point to.
   *
   * Output: A copied random host
   *
   * Purpose: To get a copy of a random host from a particular population
   */
  emp::Ptr<Organism> CopyRandHost(size_t source_pos, size_t dest_pos) {
    Organism* host = &pop[source_pos]->GetRandomOrg();
    emp::Ptr<SGPHost> old_host = dynamic_cast<SGPHost*>(host);
    emp::Ptr<Organism> new_host = old_host->MakeNew(pop[dest_pos]);
    emp::vector<emp::Ptr<Organism>> syms = old_host->GetSymbionts();
    for (size_t k = 0; k < syms.size(); k++) {
      new_host->AddSymbiont(syms[k].DynamicCast<SGPSymbiont>()->MakeNew(pop[dest_pos]));
    }
    return new_host;
  }

  /**
   * Input: The position of the population to clean up
   *
   * Output: None
   *
   * Purpose: To clear, resize, and halt reproduction in a population
   */
  void WipePop(size_t pop_pos) {
    pop[pop_pos]->to_reproduce.clear();
    pop[pop_pos]->Clear();
    pop[pop_pos]->Resize(my_config->GRID_X(), my_config->GRID_Y());
  }

  /**
   * Input: The position of the population to sample both from and to
   *
   * Output: None
   *
   * Purpose: For the next generation of this population to be sampled
   * from itself
   */
  void SampleSelf(size_t pop_pos) {
    emp_assert(IsOccupied(pop_pos));
    int sample_size =
        pop[pop_pos]->GetNumOrgs() * my_config->SAMPLE_PROPORTION();
    emp::vector<emp::Ptr<Organism>> best_orgs;
    for (int j = 0; j < sample_size; j++) {
      // orgs in pop_pos are sampled to pop_pos
      best_orgs.push_back(CopyRandHost(pop_pos, pop_pos));
    }
    WipePop(pop_pos);
    for (int j = 0; j < sample_size; j++) {
      pop[pop_pos]->AddOrgAt(best_orgs[j], pop[pop_pos]->GetNumOrgs());
    }
  }

  /**
   * Input: The position of the population to sample from and the position
   * of the population to sample to
   *
   * Output: None
   *
   * Purpose: For the next generation of this population to be sampled
   * from the passed population
   */
  void SampleSource(size_t self_pos, size_t source_pos) {
    emp_assert(source_pos != self_pos);
    emp_assert(IsOccupied(self_pos) && IsOccupied(source_pos));
    int sample_size =
        pop[source_pos]->GetNumOrgs() * my_config->SAMPLE_PROPORTION();
    WipePop(self_pos);
    for (int j = 0; j < sample_size; j++) {
      pop[self_pos]->AddOrgAt(CopyRandHost(source_pos, self_pos),
                              pop[self_pos]->GetNumOrgs());
    }
  }

  // TODO fix the seg fault that happens when there are living orgs at the end
  // of the run
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
    // make selection scheme a func or something that gets
    // defined on initialization--this if always has the same result for
    // each exp.
    if (my_config->SELECTION_SCHEME() == 0) {
      SampleRandom();
    } else if (my_config->SELECTION_SCHEME() == 1) {
      SampleTruncate();
    }
  }

  /**
   * Data node methods expanded in MetapopDataNodes.h
   */
  void CreateDataFiles();
  emp::DataFile& CreateMeansDataFile(const std::string &file_ending);
  emp::DataFile& CreateOrgCountsDataFile(const std::string &file_ending);
  emp::DataMonitor<int>& GetSymCountDataNode();
  emp::DataMonitor<int>& GetHostCountDataNode();
  void SetupTasksNodes();
};  // MetapopWorld class
#endif
