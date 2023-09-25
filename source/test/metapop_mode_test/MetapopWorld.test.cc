#include "../../metapop_mode/MetapopWorld.h"

#include "../../catch/catch.hpp"

TEST_CASE("MetaPop Update", "[metapop]") {
  emp::Random random(61);
  SymConfigBase config;

  MetapopWorld world(random, &config);

  int metapop_generations = 1;

  for(int i = 0; i < metapop_generations; i++){
    world.Update();
  }
  REQUIRE(world.GetNumOrgs() == 0);
}