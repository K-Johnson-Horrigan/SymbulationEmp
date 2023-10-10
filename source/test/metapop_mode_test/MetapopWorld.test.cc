#include "../../metapop_mode/MetapopWorld.h"
#include "../../metapop_mode/MetapopWorldSetup.cc"
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

TEST_CASE("Populate", "[metapop]") {
  emp::Random random(61);
  SymConfigBase config;

  MetapopWorld world(random, &config);

  size_t num_worlds = 1;
  REQUIRE(world.GetNumOrgs() == 0);

  world.Populate(num_worlds);

  REQUIRE(world.GetNumOrgs() == num_worlds);
}