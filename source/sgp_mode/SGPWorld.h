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
  double data_var_extinction_death_proportion;

  emp::Ptr<emp::DataMonitor<int>> data_node_steal_count;
  emp::Ptr<emp::DataMonitor<int>> data_node_donate_count;
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

    if (sgp_config->KILL_HOSTS_PER_EXTINCTION_FILE()) {
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

  void DoManualExtinctionEvent() {
    std::string str;
    size_t extinction_survivor_count;
    for (int i = 0; i < 3; i++) {
      std::getline(*source_extinction_proportion_file, str, ',');
      if (i == 2) {
        std::stringstream sstream(str);
        sstream >> extinction_survivor_count;
      }
    }
    std::getline(*source_extinction_proportion_file, str, '\n');

    // leave extinction_survivor_count random hosts alive, kill the rest
    if (GetNumOrgs() > extinction_survivor_count) {
      size_t kill_count = GetNumOrgs() - extinction_survivor_count;
      emp::vector<size_t> occupied_cells = GetValidOrgIDs();
      emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), occupied_cells.size());
      for (size_t i = 0; i < kill_count; i++) {
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
      data_var_extinction_death_proportion = 1 - ((double)GetNumOrgs() / (double)data_var_pre_extinction_host_count);
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
