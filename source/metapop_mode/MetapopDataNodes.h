#ifndef METAPOP_DATA_NODES_H
#define METAPOP_DATA_NODES_H

#include "MetapopWorld.h"

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: To create and set up the data files that contain data for the
 * experiment.
 */
void MetapopWorld::CreateDataFiles() {
  std::string file_ending =
      "_SEED" + std::to_string(my_config->SEED()) + my_config->FILE_NAME() + ".data";

  CreateMeansDataFile(file_ending);
  CreateOrgCountsDataFile(file_ending);
}

/**
 * Input: The file name ending.
 *
 * Output: None.
 *
 * Purpose: To create and set up the data file which records the mean org
 * counts and task counts accross all of the populations
 */
emp::DataFile& MetapopWorld::CreateMeansDataFile(
    const std::string& file_ending) {
  auto& file = SetupFile("Means_MetapopFile" + file_ending);
  // TODO task counts & world-spec columns (cur pooled)

  auto& av_sym_count = GetSymCountDataNode();
  auto& av_host_count = GetHostCountDataNode();
  SetupTasksNodes();

  // trigger something about the data nodes?
  // makes them work correctly for the first generation
  on_update_sig.Trigger(update);

  // set up variables
  file.AddVar(update, "generation",
              "Generation (number of times populations get sampled)");
  file.AddMean(av_sym_count, "av_sym_count", "Average symbiont count");
  file.AddMean(av_host_count, "av_host_count", "Average host count");

  TaskSet& task_set = pop[GetRandomOrgID()]->GetTaskSet();
  // each position in the data_node_host_tasks (and sym) vectors holds the
  // total counts for all the subworlds; so take the mean
  int j = 0;
  for (auto data : task_set) {
    file.AddMean(data_node_host_tasks[j], "host_task_" + data.task.name,
                 "Host completions of " + data.task.name, true);
    file.AddMean(data_node_sym_tasks[j], "sym_task_" + data.task.name,
                 "Symbiont completions of " + data.task.name, true);
    j++;
  }

  file.PrintHeaderKeys();
  return file;
}

/**
 * Input: The file name ending.
 *
 * Output: None.
 *
 * Purpose: To create and set up the data file which records the organism
 * counts for all of the populations per end of each generation
 */
emp::DataFile& MetapopWorld::CreateOrgCountsDataFile(
    const std::string& file_ending) {
  auto& file = SetupFile("OrgCounts_MetapopFile" + file_ending);
  // NOTE: task counts & world-spec columns (cur pooled)

  // trigger something about the data nodes?
  // makes them work correctly for the first generation
  on_update_sig.Trigger(update);

  file.AddVar(update, "generation",
              "Generation (number of times populations get sampled)");
  for (size_t i = 0; i < GetSize(); i++) {
    std::string world_name = "world" + std::to_string(i);
    file.AddTotal(pop[i]->GetHostCountDataNode(), world_name + "_host_count",
                  "Average host count in " + world_name);
    file.AddTotal(pop[i]->GetSymCountDataNode(), world_name + "_sym_count",
                  "Average symbiont count in " + world_name);
  }

  file.PrintHeaderKeys();
  return file;
}

/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * the count of the symbionts across all the subworlds.
 *
 * Purpose: To collect data on the count of the symbionts across all the
 * subworlds to be saved to the data file that is tracking metapopulation
 * data.
 */
emp::DataMonitor<int>& MetapopWorld::GetSymCountDataNode() {
  if (!data_node_symcount) {
    data_node_symcount.New();
    OnUpdate([this](size_t) {
      data_node_symcount->Reset();
      for (size_t i = 0; i < size(); i++) {
        if (IsOccupied(i)) {
          data_node_symcount->AddDatum(
              pop[i]->GetSymCountDataNode().GetTotal());
        }
      }
    });
  }
  return *data_node_symcount;
}

/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * the count of the hosts across all the subworlds.
 *
 * Purpose: To collect data on the count of the hosts across all the subworlds
 * to be saved to the data file that is tracking metapopulation data.
 */
emp::DataMonitor<int>& MetapopWorld::GetHostCountDataNode() {
  if (!data_node_hostcount) {
    data_node_hostcount.New();
    OnUpdate([this](size_t) {
      data_node_hostcount->Reset();
      for (size_t i = 0; i < size(); i++) {
        if (IsOccupied(i)) {
          data_node_hostcount->AddDatum(
              pop[i]->GetHostCountDataNode().GetTotal());
        }
      }
    });
  }
  return *data_node_hostcount;
}

/**
 * Input: None
 *
 * Output: None
 *
 * Purpose: To collect data on the count of the tasks executed by
 * both symbionts and hosts across all the subworlds to be saved
 * to the data file that is tracking metapopulation data.
 */
void MetapopWorld::SetupTasksNodes() {
  for (size_t i = 0; i < size(); i++) {
    if (IsOccupied(i)) {
      pop[i]->SetupTasksNodes();
    }
  }

  size_t new_size = pop[GetRandomOrgID()]->GetTaskSet().NumTasks();
  if (!data_node_host_tasks.size()) {
    data_node_host_tasks.resize(new_size);
    data_node_sym_tasks.resize(new_size);
    OnUpdate([&](auto) {
      for (size_t i = 0; i < size(); i++) {
        if (!IsOccupied(i)) {
          continue;
        }
        TaskSet& task_set = pop[i]->GetTaskSet();
        int num_tasks = task_set.NumTasks();
        auto& pop_host_tasks = pop[i]->GetHostTasksDataNodeVector();
        auto& pop_sym_tasks = pop[i]->GetSymTasksDataNodeVector();
        for (int j = 0; j < num_tasks; j++) {
          data_node_host_tasks[j].AddDatum(pop_host_tasks[j].GetTotal());
          data_node_sym_tasks[j].AddDatum(pop_sym_tasks[j].GetTotal());

          pop_host_tasks[j].Reset();  // clear for next pop gen
          pop_sym_tasks[j].Reset();
        }
        task_set.ResetTaskData();
      }
    });
  }
}

#endif