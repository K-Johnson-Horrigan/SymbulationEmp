#ifndef SGP_DATA_NODES_H
#define SGP_DATA_NODES_H

#include "SGPWorld.h"
#include <cstddef>
#include <cstdint>
#include <limits>

void SGPWorld::CreateDataFiles() {
  std::string file_ending =
      "_SEED" + std::to_string(my_config->SEED()) + ".data";
  // track the number of each organism type
  SetUpOrgCountFile(my_config->FILE_PATH() + "OrganismCounts" +
    my_config->FILE_NAME() + file_ending)
    .SetTimingRepeat(my_config->DATA_INT());

  // SGP mode doesn't need int val files, and they have significant performance
  // overhead. Only the transmission file needs to be created for this mode.  
  SetUpTransmissionFile(my_config->FILE_PATH() + "TransmissionRates" +
                        my_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(my_config->DATA_INT());
  SetupSymDonatedFile(my_config->FILE_PATH() + "SymDonated" +
                      my_config->FILE_NAME() + file_ending)
      .SetTimingRepeat(my_config->DATA_INT());
  if (my_config->TASK_TYPE() == 1) {
    SetupTasksFile(my_config->FILE_PATH() + "Tasks" + my_config->FILE_NAME() +
                   file_ending)
        .SetTimingRepeat(my_config->DATA_INT());
  } else if (my_config->TASK_TYPE() == 0) {
    SetupHostSquareFrequencyFile(my_config->FILE_PATH() + "Host_Square" +
                                 my_config->FILE_NAME() + file_ending)
        .SetTimingRepeat(my_config->DATA_INT());
    SetupSymSquareFrequencyFile(my_config->FILE_PATH() + "Sym_Square" +
                                my_config->FILE_NAME() + file_ending)
        .SetTimingRepeat(my_config->DATA_INT());
  }
}


/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: The address of the DataFile that has been created.
 *
 * Purpose: To set up the file that will be used to track 
 * organism counts in the world.
 *  This includes: (1) the host count, (2) the hosted symbiont
 * count, and (2b) the free living symbiont count, if free living
 * symbionts are permitted.
 */
emp::DataFile &SGPWorld::SetUpOrgCountFile(const std::string &filename) {
  auto& file = SetupFile(filename);
  auto& host_count = GetHostCountDataNode();
  auto& endo_sym_count = GetCountHostedSymsDataNode();
  
  file.AddVar(update, "update", "Update");
  file.AddTotal(host_count, "count", "Total number of hosts");
  file.AddTotal(endo_sym_count, "hosted_syms", "Total number of syms in a host");
  
  if (my_config->FREE_LIVING_SYMS()) {
    auto& free_sym_count = GetCountFreeSymsDataNode();
    file.AddTotal(free_sym_count, "free_syms", "Total number of free syms");
  }

  file.PrintHeaderKeys();
  
  return file;
}

emp::DataFile &SGPWorld::SetupTasksFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  SetupTasksNodes();
  int i = 0;
  for (auto data : task_set) {
    file.AddTotal(data_node_host_tasks[i], "host_task_" + data.task.name,
                  "Host completions of " + data.task.name, true);
    file.AddTotal(data_node_sym_tasks[i], "sym_task_" + data.task.name,
                  "Symbiont completions of " + data.task.name, true);
    i++;
  }
  file.PrintHeaderKeys();

  return file;
}

emp::DataFile &
SGPWorld::SetupHostSquareFrequencyFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  file.Add(
      [&](std::ostream &os) {
        for (auto [val, count] : data_node_host_squares) {
          os << val << ": " << count << "; ";
        }
        data_node_host_squares.clear();
      },
      "host_square_frequencies", "Host number of repeats for each square");
  file.PrintHeaderKeys();
  return file;
}

emp::DataFile &
SGPWorld::SetupSymSquareFrequencyFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  file.Add(
      [&](std::ostream &os) {
        for (auto [val, count] : data_node_sym_squares) {
          os << val << ": " << count << "; ";
        }
        data_node_sym_squares.clear();
      },
      "sym_square_frequencies", "Sym number of repeats for each square");
  file.PrintHeaderKeys();

  return file;
}

emp::DataFile &SGPWorld::SetupSymDonatedFile(const std::string &filename) {
  auto &file = SetupFile(filename);
  file.AddVar(update, "update", "Update");
  GetSymEarnedDataNode();
  file.AddTotal(data_node_sym_earned->UnsynchronizedGetMonitor(),
                "sym_points_earned", "Points earned by symbionts", true);
  
  // deleted sym donated/stolen columns for now

  // stress parasites
  GetSymAttackedDataNode();
  file.AddFun<size_t>(
      [&]() {
        return data_node_sym_attacked->UnsynchronizedGetMonitor().GetCount();
      },
      "sym_attack_calls", "Number of attack calls");
  file.AddTotal(data_node_sym_attacked->UnsynchronizedGetMonitor(),
                "sym_points_attacked", "Survival resources decremented by symbionts", true);

  GetSymProtectedDataNode();
  file.AddFun<size_t>(
      [&]() {
        return data_node_sym_protected->UnsynchronizedGetMonitor().GetCount();
      },
      "sym_protect_calls", "Number of protect calls");
  file.AddTotal(data_node_sym_protected->UnsynchronizedGetMonitor(),
                "sym_points_protected", "Survival resources incremented by symbionts", true);
  
  GetSymStandbyDataNode();
  file.AddFun<size_t>(
      [&]() {
        return data_node_sym_stoodby->UnsynchronizedGetMonitor().GetCount();
      },
      "sym_standby_calls", "Number of standby calls");
  file.AddTotal(data_node_sym_stoodby->UnsynchronizedGetMonitor(),
                "sym_points_stoodby", "Survival resources contributed by standby calls", true);
  
  file.AddMean(GetHostSurvivalResDataNode(), "mean_survival_res", "Average number of survival resources per host");
  
  file.AddMean(GetPositiveHostSurvivalResDataNode(), "mean_positive_survival_res", "Average positive number of survival resources per host");
  file.AddTotal(GetCountPositiveHostSurvivalResDataNode(), "count_positive_survival_res", "Number of hosts with positive survival resource values");
  
  
  auto& sym_attack_protect_node = GetSymAttackProtectProportionDataNode();


  file.AddMean(sym_attack_protect_node, "mean_attackprotect", "Average sym attack protect total");
  file.AddHistBin(sym_attack_protect_node, 0, "Hist_-100", "Count for histogram bin -100 to <-90");
  file.AddHistBin(sym_attack_protect_node, 1, "Hist_-90", "Count for histogram bin -90 to <-80");
  file.AddHistBin(sym_attack_protect_node, 2, "Hist_-80", "Count for histogram bin -80 to <-70");
  file.AddHistBin(sym_attack_protect_node, 3, "Hist_-70", "Count for histogram bin -70 to <-60");
  file.AddHistBin(sym_attack_protect_node, 4, "Hist_-60", "Count for histogram bin -60 to <-50");
  file.AddHistBin(sym_attack_protect_node, 5, "Hist_-50", "Count for histogram bin -50 to <-40");
  file.AddHistBin(sym_attack_protect_node, 6, "Hist_-40", "Count for histogram bin -40 to <-30");
  file.AddHistBin(sym_attack_protect_node, 7, "Hist_-30", "Count for histogram bin -30 to <-20");
  file.AddHistBin(sym_attack_protect_node, 8, "Hist_-20", "Count for histogram bin -20 to <-10");
  file.AddHistBin(sym_attack_protect_node, 9, "Hist_-10", "Count for histogram bin -10 to <0.0");
  file.AddHistBin(sym_attack_protect_node, 10, "Hist_0", "Count for histogram bin 0 to <10");
  file.AddHistBin(sym_attack_protect_node, 11, "Hist_10", "Count for histogram bin 10 to <20");
  file.AddHistBin(sym_attack_protect_node, 12, "Hist_20", "Count for histogram bin 20 to <30");
  file.AddHistBin(sym_attack_protect_node, 13, "Hist_30", "Count for histogram bin 30 to <40");
  file.AddHistBin(sym_attack_protect_node, 14, "Hist_40", "Count for histogram bin 40 to <50");
  file.AddHistBin(sym_attack_protect_node, 15, "Hist_50", "Count for histogram bin 50 to <60");
  file.AddHistBin(sym_attack_protect_node, 16, "Hist_60", "Count for histogram bin 60 to <70");
  file.AddHistBin(sym_attack_protect_node, 17, "Hist_70", "Count for histogram bin 70 to <80");
  file.AddHistBin(sym_attack_protect_node, 18, "Hist_80", "Count for histogram bin 80 to <90");
  file.AddHistBin(sym_attack_protect_node, 19, "Hist_90", "Count for histogram bin 90 to 100");
    
  file.PrintHeaderKeys();
  return file;
}

void SGPWorld::SetupTasksNodes() {
  if (!data_node_host_tasks.size()) {
    data_node_host_tasks.resize(task_set.NumTasks());
    data_node_sym_tasks.resize(task_set.NumTasks());
    OnUpdate([&](auto) {
      int i = 0;
      for (auto data : task_set) {
        data_node_host_tasks[i].AddDatum(data.n_succeeds_host);
        data_node_sym_tasks[i].AddDatum(data.n_succeeds_sym);
        i++;
      }
      task_set.ResetTaskData();
    });
  }
}

emp::DataMonitor<double> &SGPWorld::GetHostSurvivalResDataNode() {
  if (!data_node_host_survival_res) {
    data_node_host_survival_res.New();
    OnUpdate([this](size_t){
      data_node_host_survival_res->Reset();
      for (size_t i = 0; i< pop.size(); i++) {
        if (IsOccupied(i)) {
          data_node_host_survival_res->AddDatum(pop[i]->GetSurvivalResources());
        }
      }
    });
  }
  return *data_node_host_survival_res;
}

emp::DataMonitor<double> &SGPWorld::GetPositiveHostSurvivalResDataNode() {
  if (!data_node_positive_host_survival_res) {
    data_node_positive_host_survival_res.New();
    OnUpdate([this](size_t){
      data_node_positive_host_survival_res->Reset();
      data_node_count_positive_host_survival_res->Reset();
      data_node_sym_attack_protect_proportion->Reset();
      for (size_t i = 0; i< pop.size(); i++) {
        if (IsOccupied(i)) {
          int survival_res = pop[i]->GetSurvivalResources();
          if(survival_res > 0) {
            data_node_positive_host_survival_res->AddDatum(survival_res);
            data_node_count_positive_host_survival_res->AddDatum(1);
          }
          if (pop[i]->HasSym()) {
            for (size_t j = 0; j < pop[i]->GetSymbionts().size(); j++) {
              data_node_sym_attack_protect_proportion->AddDatum(pop[i]->GetSymbionts().at(j)->GetAttackProtect());
            }
          }
        }
      }
    });
  }
  return *data_node_positive_host_survival_res;
}

emp::DataMonitor<double> &SGPWorld::GetCountPositiveHostSurvivalResDataNode() {
  if (!data_node_count_positive_host_survival_res) {
    data_node_count_positive_host_survival_res.New();
  }
  return *data_node_count_positive_host_survival_res;
}

SyncDataMonitor<double> &SGPWorld::GetSymEarnedDataNode() {
  if (!data_node_sym_earned) {
    data_node_sym_earned.New();
  }
  return *data_node_sym_earned;
}

SyncDataMonitor<double> &SGPWorld::GetSymDonatedDataNode() {
  if (!data_node_sym_donated) {
    data_node_sym_donated.New();
  }
  return *data_node_sym_donated;
}

SyncDataMonitor<double> &SGPWorld::GetSymStolenDataNode() {
  if (!data_node_sym_stolen) {
    data_node_sym_stolen.New();
  }
  return *data_node_sym_stolen;
}

SyncDataMonitor<double> &SGPWorld::GetSymAttackedDataNode() {
  if (!data_node_sym_attacked) {
    data_node_sym_attacked.New();
  }
  return *data_node_sym_attacked;
}

SyncDataMonitor<double> &SGPWorld::GetSymProtectedDataNode() {
  if (!data_node_sym_protected) {
    data_node_sym_protected.New();
  }
  return *data_node_sym_protected;
}

SyncDataMonitor<double> &SGPWorld::GetSymStandbyDataNode() {
  if (!data_node_sym_stoodby) {
    data_node_sym_stoodby.New();
  }
  return *data_node_sym_stoodby;
}

emp::DataMonitor<double, emp::data::Histogram>& SGPWorld::GetSymAttackProtectProportionDataNode() {
  if (!data_node_sym_attack_protect_proportion) {
    data_node_sym_attack_protect_proportion.New();
  }
  // copying the int val structure from default--only 20 actual bins
  data_node_sym_attack_protect_proportion->SetupBins(-100, 101, 21);
  return *data_node_sym_attack_protect_proportion;
}

#endif
