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
emp::DataFile& MetapopWorld::CreateDataFiles() {
  std::string file_ending =
      "_SEED" + std::to_string(my_config->SEED()) + ".data";
  auto& file = SetupFile("MetapopFile" + file_ending);
  // What I want to track for now:
  // some stats for each of the...lets say 5 worlds that exist:
  // av num orgs
  // num of not, nor, xor
  // TODO task counts & world-spec columns (cur pooled)

  auto& av_sym_count = GetSymCountDataNode();
  auto& av_host_count = GetHostCountDataNode();

  // trigger something about the data nodes?
  // makes them work correctly for the first generatione
  on_update_sig.Trigger(update);

  // set up variables
  file.AddVar(update, "update", "Update");
  file.AddMean(av_sym_count, "av_sym_count", "Average symbiont count");
  file.AddMean(av_host_count, "av_host_count", "Average host count");

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

#endif