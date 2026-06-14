#ifndef WORLD_SETUP_C
#define WORLD_SETUP_C

#include "SymWorld.h"
#include "Host.h"
#include "Symbiont.h"


/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: To set up systematics (phylogenies).
 */
void SymWorld::SetupSystematics(){ 
  host_sys = emp::NewPtr<emp::Systematics<Organism, taxon_t::info_t, datastruct::HostTaxonData>>(GetCalcHostInfoFun());
  sym_sys = emp::NewPtr< emp::Systematics<Organism, taxon_t::info_t, datastruct::SymbiontTaxonData>>(GetCalcSymInfoFun());

  AddSystematics(host_sys);
  sym_sys->SetStorePosition(false);

  sym_sys->AddSnapshotFun([](const taxon_t::sym_taxon_t& t) {return std::to_string(t.GetInfo()); }, "info");
  host_sys->AddSnapshotFun([](const taxon_t::host_taxon_t& t) {return std::to_string(t.GetInfo()); }, "info");

  if (my_config->PHYLOGENY_TAXON_TYPE() == 2 || my_config->PHYLOGENY_TAXON_TYPE() == 3) {
    sym_sys->AddSnapshotFun([](const taxon_t::sym_taxon_t& t) {return std::to_string((t.GetData()).GetIntVal()); }, "mean_int_val");
    host_sys->AddSnapshotFun([](const taxon_t::host_taxon_t& t) {return std::to_string(t.GetData().GetIntVal()); }, "mean_int_val");
  }
  if (my_config->PHYLOGENY_TAXON_TYPE() == 3) {
    sym_sys->AddSnapshotFun([](const taxon_t::sym_taxon_t& t) {return std::to_string(t.GetData().GetHostSwitch()); }, "lineage_host_switch_count");
  }

  on_placement_sig.AddAction([this](emp::WorldPosition pos) {
    GetOrgPtr(pos.GetIndex())->SetTaxon(host_sys->GetTaxonAt(pos).Cast<taxon_t::base_taxon_t>());
    if(my_config->PHYLOGENY_TAXON_TYPE()==3)GetOrgPtr(pos.GetIndex())->GetTaxon()->GetData().RecordIntVal(GetOrgPtr(pos.GetIndex())->GetIntVal());
  });

  if (my_config->PHYLOGENY_TAXON_TYPE() == 3) {
    std::function<void(emp::Ptr<taxon_t::sym_taxon_t >, Organism&)> inherit_parental_data =
      [&](emp::Ptr<taxon_t::sym_taxon_t > taxon, Organism& org) {
      if (taxon->GetParent()) taxon->GetData().SetHostSwitch(taxon->GetParent()->GetData().GetHostSwitch());
      else taxon->GetData().SetHostSwitch(0);
      taxon->GetData().RecordIntVal(org.GetIntVal());
      };
    sym_sys->OnNew(inherit_parental_data);
  }

  if (my_config->STORE_EXTINCT()) {
    sym_sys->SetStoreOutside(true);
    host_sys->SetStoreOutside(true);
  }
}


/**
 * Input: The number of hosts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with hosts with appropriate phenotypes. 
 */
void SymWorld::SetupHosts(long unsigned int* POP_SIZE){ 
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<Host> new_org;
    new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
    if (my_config->TAG_MATCHING()) {
      emp::BitSet<TAG_LENGTH> new_tag = emp::BitSet<TAG_LENGTH>(GetRandom(), my_config->STARTING_TAGS_ONE_PROB());
      new_org->SetTag(new_tag);
    }
    InjectHost(new_org);
  }

}


/**
 * Input: The number of symbionts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with symbionts with appropriate phenotypes.
 */
void SymWorld::SetupSymbionts(long unsigned int *total_syms) {
  for (size_t j = 0; j < *total_syms; j++) {
    emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0);
    if (my_config->TAG_MATCHING()) {
      emp::BitSet<TAG_LENGTH> new_tag = emp::BitSet<TAG_LENGTH>(GetRandom(), my_config->STARTING_TAGS_ONE_PROB());
      new_sym->SetTag(new_tag); // if this sym is hosted, this tag will be overwritten upon injection
    }
    InjectSymbiont(new_sym);
  }
}

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Prepare the world for an experiment by applying the configuration settings 
 * and populating the world with hosts and symbionts.
 */
void SymWorld::Setup() {
  if(my_config->PHYLOGENY() == true){
    SetupSystematics();
  }

  double start_moi = my_config->START_MOI();
  long unsigned int POP_SIZE;
  if (my_config->POP_SIZE() == -1) {
    POP_SIZE = my_config->GRID_X() * my_config->GRID_Y();
  } else {
    POP_SIZE = my_config->POP_SIZE();
  }

  // set world structure (either mixed or a grid with some dimensions) and set synchronous generations to false
  if (my_config->GRID() == 0) {SetPopStruct_Mixed(false);}
  else SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);

  SetupHosts(&POP_SIZE);

  Resize(my_config->GRID_X(), my_config->GRID_Y());
  long unsigned int total_syms = POP_SIZE * start_moi;
  SetupSymbionts(&total_syms);
}
#endif
