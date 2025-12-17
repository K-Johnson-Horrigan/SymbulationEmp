#ifndef SGPWORLD_H
#define SGPWORLD_H

#include <filesystem> 
#include "../default_mode/SymWorld.h"
#include "Tasks.h"
#include "emp/Evolve/World_structure.hpp"
#include "emp/data/DataNode.hpp"
#include "SGPConfigSetup.h"

struct StressEscapeeOffspring{
  emp::Ptr<Organism> escapee_offspring;
  size_t parent_pos;
  const emp::BitSet<CPU_BITSET_LENGTH> infection_tasks;

  StressEscapeeOffspring(
    emp::Ptr<Organism> sym,
    size_t loc,
    const emp::BitSet<CPU_BITSET_LENGTH> _tasks
  ) :
    escapee_offspring(sym),
    parent_pos(loc),
    infection_tasks(_tasks)
  {
  }
};

class SGPWorld : public SymWorld {
private:
  TaskSet task_set;

  size_t data_var_pre_extinction_host_count;

  emp::Ptr<emp::DataMonitor<int>> data_node_steal_count;
  emp::Ptr<emp::DataMonitor<int>> data_node_donate_count;
  emp::Ptr<emp::DataMonitor<size_t>> data_node_extinction_dead_host_count;
  emp::Ptr<emp::DataMonitor<size_t>> data_node_stress_escapee_offspring_attempt_count;
  emp::Ptr<emp::DataMonitor<size_t>> data_node_stress_escapee_offspring_success_count;
  emp::vector<emp::DataMonitor<size_t>> data_node_host_tasks;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_tasks;
  
  emp::Ptr<emp::DataFile> death_proportion_data_file;
  emp::Ptr<std::ifstream> source_extinction_proportion_file;

  /**
  *
  * Purpose: Holds all configuration settings and points to same configuration
  * object as my_config from superclass, but with the correct subtype.
  *
  */
  emp::Ptr<SymConfigSGP> sgp_config = NULL;
public:
  emp::vector<emp::Ptr<Organism>> to_reproduce;
  emp::vector<StressEscapeeOffspring> symbiont_stress_escapee_offspring;

  // The task profile retriever function
  std::function<const emp::BitSet<CPU_BITSET_LENGTH>& (const emp::Ptr<Organism>)> fun_get_task_profile;

  SGPWorld(emp::Random &r, emp::Ptr<SymConfigSGP> _config, TaskSet task_set)
      : SymWorld(r, _config),
    task_set(task_set) {
    sgp_config = _config;

    SetupTaskProfileFun();

    if (sgp_config->INTERACTION_MECHANISM() == STRESS_MANUAL_KILL) {
      emp_assert(std::filesystem::exists(sgp_config->SOURCE_EXTINCTION_PROPORTION_FILE_NAME()));
      source_extinction_proportion_file = emp::NewPtr<std::ifstream>(sgp_config->SOURCE_EXTINCTION_PROPORTION_FILE_NAME());
      std::string str;
      std::getline(*source_extinction_proportion_file, str); // skip the header 
    }
  }

  ~SGPWorld() {
    // data node deletes 
    if (data_node_steal_count) data_node_steal_count.Delete();
    if (data_node_donate_count) data_node_donate_count.Delete();
    if (data_node_extinction_dead_host_count) data_node_extinction_dead_host_count.Delete();
    if (data_node_stress_escapee_offspring_attempt_count) data_node_stress_escapee_offspring_attempt_count.Delete();
    if (data_node_stress_escapee_offspring_success_count) data_node_stress_escapee_offspring_success_count.Delete();

    // The vectors will delete themselves automatically
    for (auto escapee_data : symbiont_stress_escapee_offspring) {
      escapee_data.escapee_offspring.Delete();
    }
    if (death_proportion_data_file) death_proportion_data_file.Delete();
    if (source_extinction_proportion_file) source_extinction_proportion_file.Delete();
  }

  /**
   * Input: None
   *
   * Output: The task set used for this world.
   *
   * Purpose: Allows accessing the world's task set.
   */
  TaskSet &GetTaskSet() { return task_set; }

  /**
   * Input: None
   *
   * Output: The sgp configuration used for this world.
   *
   * Purpose: Allows accessing the world's sgp config.
   */
  const emp::Ptr<SymConfigSGP> GetConfig() const { return sgp_config; }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Runs manual extinctions, killing a number of hosts
   * as determined by an input file set up on construction
   */
  void DoManualExtinctionEvent() {
    // it is possible for some hosts to be born in an extinction update,
    // so the count of post-extinction hosts neq pre - dead 
    // in this method, we want to set our population size to 
    // the number of host - the dead hosts (i.e. our population will then be 
    // the same "pre-birth" size) 
    // 650,9994,8064,1933 <- e.g. here, it looks like 3 hosts were born 

    std::string str; 
    size_t dead_count;
    size_t pre_extinction_count; 
    for (int i = 0; i < 3; i++) {
      std::getline(*source_extinction_proportion_file, str, ',');
      if (i == 1) {
        std::stringstream sstream(str);
        sstream >> pre_extinction_count;
      }
    }
    std::getline(*source_extinction_proportion_file, str, '\n');
    std::stringstream sstream(str);
    sstream >> dead_count;
    
    size_t target_pop_size = pre_extinction_count - dead_count;
    // leave target_pop_size random hosts alive, kill the rest
    if (GetNumOrgs() > target_pop_size) {
      size_t to_make_dead_count = GetNumOrgs() - target_pop_size;
      GetExtinctionDeadHostCount().AddDatum(to_make_dead_count);
      emp::vector<size_t> occupied_cells = GetValidOrgIDs();
      emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), occupied_cells.size());
      for (size_t i = 0; i < to_make_dead_count; i++) {
        DoDeath(occupied_cells[schedule[i]]);
      }
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To simulate a timestep in the world, which includes calling the
   * process functions for hosts and symbionts and updating the data nodes.
   */
  void Update() override {
    if ((sgp_config->INTERACTION_MECHANISM() == STRESS || sgp_config->INTERACTION_MECHANISM() == STRESS_MANUAL_KILL) && 
      sgp_config->TRACK_EXTINCTION_DEATH_PROPORTION() &&
      GetUpdate() % sgp_config->EXTINCTION_FREQUENCY() == 0) {
      data_var_pre_extinction_host_count = GetNumOrgs(); // assumes no free living syms
    }
    
    if (sgp_config->INTERACTION_MECHANISM() == STRESS_MANUAL_KILL && GetUpdate() % sgp_config->EXTINCTION_FREQUENCY() == 0) {
      DoManualExtinctionEvent();
    }

    // These must be done here because we don't call SymWorld::Update()
    // That may change in the future
    emp::World<Organism>::Update();
    if (sgp_config->PHYLOGENY())
      sym_sys->Update();
    // Handle resource inflow
    if (total_res != -1) {
      total_res += sgp_config->LIMITED_RES_INFLOW();
    }

    // emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
    // for(size_t i : schedule) {
    //   if (IsOccupied(i) == false) continue;

    //   //else
    //   pop[i]->Process(i);
    //   if (pop[i]->GetDead()) DoDeath(i);
    // }

    for (size_t id = 0; id < GetSize(); id++) {
      if (IsOccupied(id)) {
        auto & org = pop[id];
        org->Process(id);
        if (org->GetDead()) DoDeath(id);
      }
    }

    ProcessReproductionQueue();
    
    ProcessStressEscapeeOffspring();
    
    CleanupGraveyard();

    if ((sgp_config->INTERACTION_MECHANISM() == STRESS || sgp_config->INTERACTION_MECHANISM() == STRESS_MANUAL_KILL) &&
      sgp_config->TRACK_EXTINCTION_DEATH_PROPORTION() &&
      GetUpdate() % sgp_config->EXTINCTION_FREQUENCY() == 0) {
      death_proportion_data_file->Update();
    }
  }

  // Prototypes for setup methods
  void SetupHosts(long unsigned int *POP_SIZE) override;
  void SetupSymbionts(long unsigned int *total_syms) override;
  void SetupTaskProfileFun();

  // Update helper methods
  void ProcessReproductionQueue();

  // Prototypes for reproduction handling methods
  emp::WorldPosition SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) override;
  int GetNeighborHost(size_t source_id, const emp::BitSet<CPU_BITSET_LENGTH>& symbiont_tasks);
  bool TaskMatchCheck(const emp::BitSet<CPU_BITSET_LENGTH>& symbiont_tasks, const emp::BitSet<CPU_BITSET_LENGTH>& host_tasks);
  bool PreferentialOustingAllowed(const emp::BitSet<CPU_BITSET_LENGTH>& incoming_sym_tasks, emp::Ptr<Organism> host);

  // Prototypes for symbiont placement
  emp::WorldPosition PlaceSymbiontInHost(emp::Ptr<Organism> symbiont, const emp::BitSet<CPU_BITSET_LENGTH>& symbiont_infection_tasks, size_t source_pos);

  // Prototypes for sym transferring
  emp::WorldPosition SymFindHost(emp::Ptr<Organism> symbiont, emp::WorldPosition cur_pos);
  void ProcessStressEscapeeOffspring();

  // Prototype for graveyard handling method
  void SendToGraveyard(emp::Ptr<Organism> org) override;

  // Prototypes for data node methods
  emp::DataMonitor<int> &GetStealCount();
  emp::DataMonitor<int> &GetDonateCount();
  emp::DataMonitor<size_t>& GetExtinctionDeadHostCount();
  emp::DataMonitor<size_t>& GetStressEscapeeOffspringAttemptCount();
  emp::DataMonitor<size_t>& GetStressEscapeeOffspringSuccessCount();

  void SetupTasksNodes();

  emp::DataFile &SetUpOrgCountFile(const std::string &filename);
  emp::DataFile &SetupSymInstFile(const std::string &filename);
  emp::DataFile &SetupTasksFile(const std::string &filename);
  void SetupTransmissionFileColumns(emp::DataFile& file);
  void WriteTaskCombinationsFile(const std::string& filename);
  void WriteOrgReproHistFile(const std::string& filename);
  void SetupDeathProportionFile(const std::string& filename);

  void CreateDataFiles() override;
};

#endif
