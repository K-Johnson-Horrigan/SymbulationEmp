#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "Scheduler.h"
#include "Tasks.h"
#include "emp/Evolve/World_structure.hpp"
#include "emp/data/DataNode.hpp"

/// Helper which synchronizes access to the DataMonitor with a mutex
template <typename T, emp::data... MODS> class SyncDataMonitor {
  std::mutex mutex;
  emp::DataMonitor<T, MODS...> monitor;

public:
  /**
   * Input: None
   *
   * Output: Reference to the underlying DataMonitor, without synchronization.
   *
   * Purpose: Accesses the underlying monitor without synchronization, should
   * only be used when you're sure there's no multithreading going on.
   */
  emp::DataMonitor<T, MODS...> &UnsynchronizedGetMonitor() { return monitor; }

  /**
   * Input: An action to perform with the DataMonitor.
   *
   * Output: None
   *
   * Purpose: Calls the provided callback with the DataMonitor while holding the
   * mutex, releasing it when it returns.
   */
  template <typename F> void WithMonitor(F f) {
    std::lock_guard lock(mutex);
    f(monitor);
  }
};

class SGPWorld : public SymWorld {
private:
  Scheduler scheduler;
  TaskSet task_set;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_donated;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_stolen;
  emp::Ptr<SyncDataMonitor<double>> data_node_sym_earned;
  emp::vector<emp::DataMonitor<size_t>> data_node_host_tasks;
  emp::vector<emp::DataMonitor<size_t>> data_node_sym_tasks;
  emp::vector<int> host_task_counts;
  emp::vector<int> sym_task_counts;

public:
  std::map<uint32_t, size_t> data_node_host_squares;
  std::map<uint32_t, size_t> data_node_sym_squares;
  std::mutex squares_mutex;

  emp::vector<std::pair<emp::Ptr<Organism>, emp::WorldPosition>> to_reproduce;

  SGPWorld(emp::Random &r, emp::Ptr<SymConfigBase> _config, TaskSet task_set)
      : SymWorld(r, _config), scheduler(*this, _config->THREAD_COUNT()),
        task_set(task_set) {

          // set up the whole-experiment task count vector
          host_task_counts.assign(task_set.NumTasks(), 0);
          sym_task_counts.assign(task_set.NumTasks(), 0);
  }

  virtual ~SGPWorld() {
    if(data_node_sym_donated) data_node_sym_donated.Delete();
    if(data_node_sym_stolen) data_node_sym_stolen.Delete();
    if(data_node_sym_earned) data_node_sym_earned.Delete();
    // The vectors will delete themselves automatically
  }

  /**
   * Input: None
   *
   * Output: A vector containing the number of time each task has been 
   * executed by hosts over the course of the experiment
   *
   * Purpose: Allows indentification of how many population-and individual
   * level tasks have been accomplished.
   */
  emp::vector<int>* GetHostTaskCounts(){ return &host_task_counts; }

    /**
   * Input: None
   *
   * Output: A vector containing the number of time each task has been 
   * executed by symbionts over the course of the experiment
   *
   * Purpose: Allows indentification of how many tasks have been
   * performed by symbionts while hosts worked toward ind/pop tasks
   */
  emp::vector<int>* GetSymTaskCounts(){ return &sym_task_counts; }
  
  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Set the population and individual level trackers back to 0. 
   */
  void ResetTaskCounts(){
    for(size_t i = 0; i < host_task_counts.size(); i++){
      host_task_counts[i] = 0;
      sym_task_counts[i] = 0;
    }
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
   * Output: None
   *
   * Purpose: To simulate a timestep in the world, which includes calling the
   * process functions for hosts and symbionts and updating the data nodes.
   */
  void Update() {
    // These must be done here because we don't call SymWorld::Update()
    // That may change in the future
    emp::World<Organism>::Update();
    if (my_config->PHYLOGENY())
      sym_sys->Update();
    // Handle resource inflow
    if (total_res != -1) {
      total_res += my_config->LIMITED_RES_INFLOW();
    }

    scheduler.ProcessOrgs([&](emp::WorldPosition pos, Organism &org) {
      if (org.IsHost()) {
        org.Process(pos);
        if (org.GetDead()) DoDeath(pos);
      }
      else {
        //have to check for death first, because it might have moved
       // process takes worldposition, dosymdeath takes popid
        if (org.GetDead()) DoSymDeath(pos.GetPopID());
        else org.Process(pos);
        if(IsSymPopOccupied(pos.GetPopID()) && org.GetDead()) DoSymDeath(pos.GetPopID());
        
      }
    });

    for (auto org : to_reproduce) {
      if (!org.second.IsValid() || org.first->GetDead())
        continue;
      emp::Ptr<Organism> child = org.first->Reproduce();
      if (child->IsHost()) {
        // Host::Reproduce() doesn't take care of vertical transmission, that
        // happens here
        for (auto &sym : org.first->GetSymbionts()) {
          sym->VerticalTransmission(child);
        }
        DoBirth(child, org.second);
      } else {
        emp::WorldPosition new_pos = SymDoBirth(child, org.second);
        // Because we're not calling HorizontalTransmission, we need to adjust
        // these data nodes here
        emp::DataMonitor<int> &data_node_attempts_horiztrans =
            GetHorizontalTransmissionAttemptCount();
        data_node_attempts_horiztrans.AddDatum(1);

        emp::DataMonitor<int> &data_node_successes_horiztrans =
            GetHorizontalTransmissionSuccessCount();
        if (new_pos.IsValid()) {
          data_node_successes_horiztrans.AddDatum(1);
        }
      }
    }
    to_reproduce.clear();
  }

  // Prototypes for setup methods
  void SetupHosts(long unsigned int *POP_SIZE) override;
  void SetupSymbionts(long unsigned int *total_syms) override;

  
  emp::WorldPosition SymDoBirth(emp::Ptr<Organism> sym_baby, emp::WorldPosition parent_pos) override;
  int GetNeighborHost (size_t id, emp::Ptr<emp::BitSet<64>>);

  // Prototypes for data node methods
  SyncDataMonitor<double> &GetSymDonatedDataNode();
  SyncDataMonitor<double> &GetSymStolenDataNode();
  SyncDataMonitor<double> &GetSymEarnedDataNode();
  emp::vector<emp::DataMonitor<size_t>> &GetHostTasksDataNodeVector(){
    return data_node_host_tasks;
  }
  emp::vector<emp::DataMonitor<size_t>> &GetSymTasksDataNodeVector(){
    return data_node_sym_tasks;
  }
  
  void SetupTasksNodes();

  emp::DataFile &SetUpOrgCountFile(const std::string &filename);
  emp::DataFile &SetupSymDonatedFile(const std::string &filename);
  emp::DataFile &SetupTasksFile(const std::string &filename);
  emp::DataFile &SetupHostSquareFrequencyFile(const std::string &filename);
  emp::DataFile &SetupSymSquareFrequencyFile(const std::string &filename);

  void CreateDataFiles() override;
};

#endif