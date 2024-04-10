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
      "_SEED" + std::to_string(my_config->SEED()) + "_SS" + std::to_string(my_config->SELECTION_SCHEME()) + my_config->FILE_NAME() + ".data";

  SetupTasksNodes();
  CreateOrgCountsDataFile(file_ending);
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

  // get some task set for names etc
  TaskSet& task_set = pop[GetRandomOrgID()]->GetTaskSet();

  file.AddVar(update, "generation",
              "Generation (number of times populations get sampled)");
  for (size_t i = 0; i < GetSize(); i++) {
    std::string world_name = "world" + std::to_string(i);
    file.AddTotal(pop[i]->GetHostCountDataNode(), world_name + "_host_count",
                  "Average host count in " + world_name);
    file.AddTotal(pop[i]->GetSymCountDataNode(), world_name + "_sym_count",
                  "Average symbiont count in " + world_name);

    emp::vector<int>* host_task_counts = pop[i]->GetHostTaskCounts();
    emp::vector<int>* sym_task_counts = pop[i]->GetHostTaskCounts();
    int t = 0;
    for (auto data : task_set) {
      file.AddVar(host_task_counts->at(t), world_name + "_"+ data.task.name +"_host_count",
                  "Overall host completions of " + data.task.name + " in " + world_name);
      file.AddVar(sym_task_counts->at(t), world_name + "_"+ data.task.name +"_sym_count",
                  "Overall symbiont completions of " + data.task.name + " in " + world_name);
      t++;
    }
  }

  file.PrintHeaderKeys();
  return file;
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
}

#endif