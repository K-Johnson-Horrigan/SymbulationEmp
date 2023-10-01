#ifndef EFFWORLD_SETUP_C
#define EFFWORLD_SETUP_C

#include "EfficientWorld.h"
#include "EfficientSymbiont.h"
#include "EfficientHost.h"

/**
 * Input: The number of efficient hosts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with efficient hosts with appropriate phenotypes.
 */
void EfficientWorld::SetupHosts(long unsigned int* POP_SIZE) {
  for (size_t i = 0; i < *POP_SIZE; i++) {
    emp::Ptr<EfficientHost> new_org;
    new_org.New(&GetRandom(), this, my_config, my_config->HOST_INT());
    InjectHost(new_org);
  }
}

/**
 * Input: The number of efficient symbionts.
 *
 * Output: None.
 *
 * Purpose: To populate the world with efficient symbionts with appropriate phenotypes.
 */
void EfficientWorld::SetupSymbionts(long unsigned int* total_syms) {
  for (size_t j = 0; j < *total_syms; j++) {
    emp::Ptr<EfficientSymbiont> new_sym = emp::NewPtr<EfficientSymbiont>(&GetRandom(), this, my_config, my_config->SYM_INT(), 0, 1);
    InjectSymbiont(new_sym);
  }
}

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Prepare the world for a simulation by applying the configuration settings
 * and populating the world with efficient hosts and efficient symbionts.
 */
void EfficientWorld::Populate() {
  if (my_config->EFFICIENCY_MUT_RATE() == -1) my_config->EFFICIENCY_MUT_RATE(my_config->HORIZ_MUTATION_RATE());
  SymWorld::Populate();
}
#endif
