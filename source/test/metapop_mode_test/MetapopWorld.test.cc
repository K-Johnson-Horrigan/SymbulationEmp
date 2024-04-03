#include "../../metapop_mode/MetapopWorld.h"
#include "../../metapop_mode/MetapopDataNodes.h"
#include "../../metapop_mode/MetapopWorldSetup.cc"


TEST_CASE("Populate", "[metapop]") {
  emp::Random random(61);
  
  
  SymConfigBase config;
  MetapopWorld world(random, &config);
  int x = 10;
  int y = 100;
  size_t world_size = x * y;
  config.GRID_X(x);
  config.GRID_Y(y);
  config.POP_SIZE(world_size);
  config.RANDOM_ANCESTOR(0);
  config.TASK_TYPE(3);

  size_t num_worlds = 3;
  config.NUM_POPULATIONS(num_worlds);
  
  REQUIRE(world.GetNumOrgs() == 0);

  world.Populate();
  
  REQUIRE(world.GetNumOrgs() == num_worlds);

  for(size_t i = 0; i < num_worlds; i++){
    REQUIRE(world.GetOrg(i).GetNumOrgs() == world_size);
  }
}

TEST_CASE("MetaPop Update", "[metapop]") {
  emp::Random random(61);
  SymConfigBase config;
  MetapopWorld world(random, &config);
  
  double proportion = 0.5;
  int x = 10;
  int y = 100;
  int world_size = x * y;
  size_t num_worlds = 3;
  config.GRID_X(x);
  config.GRID_Y(y);
  config.SAMPLE_PROPORTION(proportion);
  config.POP_SIZE(-1);
  config.RANDOM_ANCESTOR(0);
  config.TASK_TYPE(1);
  config.UPDATES(1);
  config.NUM_POPULATIONS(num_worlds);
  
  world.Populate();
  REQUIRE(world.GetNumOrgs() == num_worlds);

  int expected_sample_size = world_size * proportion;

  // sampling behavior should be the same between selection schemes
  WHEN("Random selection is used"){
    config.SELECTION_SCHEME(0);
    config.RANDOM_SELECTION_SCHEME(2);
    THEN("Hosts are sampled"){
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == world_size);
      }
      world.Update();
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == expected_sample_size);
      }

    }
  }

  WHEN("Truncation selection is used"){
    config.SELECTION_SCHEME(1);
    THEN("Hosts are sampled"){
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == world_size);
      }
      world.Update();
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == expected_sample_size);
      }
    }
  }

   WHEN("Tournament selection is used"){
    config.SELECTION_SCHEME(2);
    THEN("Hosts are sampled"){
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == world_size);
      }
      world.Update();
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == expected_sample_size);
      }
    }
  }
}

TEST_CASE("Selection schemes", "[metapop]"){
  emp::Random random(61);
  SymConfigBase config;
  MetapopWorld world(random, &config);
  
  double proportion = 1.0;
  int x = 10;
  int y = 100;
  int world_size = x * y;
  size_t num_worlds = 1;
  int updates = 201;
  config.DATA_INT(120);
  config.UPDATES(updates);
  config.GRID_X(x);
  config.GRID_Y(y);
  config.SAMPLE_PROPORTION(proportion);
  config.POP_SIZE(-1);
  config.RANDOM_ANCESTOR(0);
  config.START_MOI(0);
  config.TASK_TYPE(1);
  config.NUM_POPULATIONS(num_worlds);
  
  // no reproduction
  config.MUTATION_RATE(0);
  config.HOST_REPRO_RES(1000000);
  config.SYM_HORIZ_TRANS_RES(1000000);
  config.SYM_VERT_TRANS_RES(1000000);
 

  int expected_sample_size = world_size * proportion;

  // fitness function should be NAND only as individual, sum of everything else 
  // as population 

  // NOT only genome: 1
  config.START_GENOME_TYPE(1);
  world.Populate();
  REQUIRE(world.GetNumOrgs() == num_worlds);
  
  // NAND only genome: 2
  num_worlds = 9;
  config.START_GENOME_TYPE(2);
  config.NUM_POPULATIONS(num_worlds);
  world.Populate();

  num_worlds = 10;
  REQUIRE(world.GetNumOrgs() == num_worlds);
   world.SetupTasksNodes();
   
  // RUN A FEW UPDATES TO ACCRUE TASKS
  emp::vector<size_t> schedule = emp::GetPermutation(world.GetRandom(), num_worlds);
  for (size_t i : schedule) {
    if (world.IsOccupied(i)) {
      world.GetOrg(i).ResetIndPopTaskCounts();
      world.GetOrg(i).RunExperiment(false);
    }
  }
  world.emp::World<SGPWorld>::Update(); 
  
  // SET UP BASELINE TASK COUNTS FOR THIS SYSTEM
  double pop_tasks_pre = 0;
  double ind_tasks_pre = 0;
  
  double expected_highest_pop = world.GetOrg(0).GetPopTaskCount();
  double expected_lowest_ind = world.GetOrg(0).GetIndTaskCount();
  double expected_lower_pop = world.GetOrg(1).GetPopTaskCount();

  for (size_t i = 0; i < num_worlds; i++){
    emp_assert(world.IsOccupied(i));
    pop_tasks_pre += world.GetOrg(i).GetPopTaskCount();
    ind_tasks_pre += world.GetOrg(i).GetIndTaskCount();
  }

  WHEN("We run the worlds (not sampling them)"){
    THEN("They accrue tasks"){
      REQUIRE(pop_tasks_pre > 0); // NOTs (and others)--1 world start with this
      REQUIRE(ind_tasks_pre > 0); // NANDs (only)--9 worlds start with these
      REQUIRE(ind_tasks_pre > pop_tasks_pre);
      REQUIRE(expected_lowest_ind == 0); // the NAND world didn't do any individual tasks

      for (size_t i = 1; i < num_worlds; i++){
        emp_assert(world.IsOccupied(i));
        REQUIRE(world.GetOrg(i).GetPopTaskCount() + 100 < expected_highest_pop);
        REQUIRE(world.GetOrg(i).GetIndTaskCount() - 100 > expected_lowest_ind);
      }
    }
  }
  
  WHEN("We use truncation selection"){
    world.SampleTruncate();

    THEN("We sample the correct number of organisms"){
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == expected_sample_size);
      }
    }

    emp::vector<size_t> schedule = emp::GetPermutation(world.GetRandom(), num_worlds);
    for (size_t i : schedule) {
      if (world.IsOccupied(i)) {
        world.GetOrg(i).ResetIndPopTaskCounts();
        world.GetOrg(i).RunExperiment(false);
      }
    }

    THEN("Under these conditions organisms don't reproduce"){
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == expected_sample_size);
      }
    }

    THEN("The mean number of population level tasks in the next generation will increase"){
      double pop_tasks_post = 0;
      double ind_tasks_post = 0;

      for (size_t i = 0; i < num_worlds; i++){
        emp_assert(world.IsOccupied(i));
        pop_tasks_post += world.GetOrg(i).GetPopTaskCount();
        ind_tasks_post += world.GetOrg(i).GetIndTaskCount();
      }
      
      REQUIRE(pop_tasks_post > 0); // NOTs 
      REQUIRE(ind_tasks_post == 0); // NANDs 
      REQUIRE(pop_tasks_post > pop_tasks_pre + 100);
      REQUIRE(pop_tasks_post > ind_tasks_post);

      for (size_t i = 1; i < num_worlds; i++){
        emp_assert(world.IsOccupied(i));
        REQUIRE(world.GetOrg(i).GetPopTaskCount() - 100 > expected_lower_pop);
      }
        
    }
  }

  WHEN("We use random selection"){
    config.RANDOM_SELECTION_SCHEME(2);
    world.SampleRandom();

    THEN("We sample the correct number of organisms"){
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == expected_sample_size);
      }
    }

    emp::vector<size_t> schedule = emp::GetPermutation(world.GetRandom(), num_worlds);
    for (size_t i : schedule) {
      if (world.IsOccupied(i)) {
        world.GetOrg(i).ResetIndPopTaskCounts();
        world.GetOrg(i).RunExperiment(false);
      }
    }

    THEN("Under these conditions organisms don't reproduce"){
      for(size_t i = 0; i < num_worlds; i++){
        int host_count = world.GetOrg(i).GetNumOrgs();
        REQUIRE(host_count == expected_sample_size);
      }
    }

    THEN("The mean number of population level tasks in the next generation will increase"){
      double pop_tasks_post = 0;
      double ind_tasks_post = 0;

      for (size_t i = 0; i < num_worlds; i++){
        emp_assert(world.IsOccupied(i));
        pop_tasks_post += world.GetOrg(i).GetPopTaskCount();
        ind_tasks_post += world.GetOrg(i).GetIndTaskCount();
      }
      REQUIRE(pop_tasks_post < ind_tasks_post); 
      REQUIRE(pop_tasks_post < pop_tasks_pre); // if there are NOTs, there are very few of them accross the 10 worlds
      REQUIRE(ind_tasks_post > 0); // NANDs 
      REQUIRE(ind_tasks_post > ind_tasks_pre); // since we should now have 10 NOT worlds, we expect more ind tasks

      for (size_t i = 1; i < num_worlds; i++){
        emp_assert(world.IsOccupied(i));
        REQUIRE(world.GetOrg(i).GetPopTaskCount() < 100);
      }
        
    }
  }
}


TEST_CASE("SampleSelf", "[metapop]") {
  emp::Random random(61);
  SymConfigBase config;
  MetapopWorld world(random, &config);
  
  double proportion = 0.5;
  int x = 10;
  int y = 100;
  size_t world_size = x * y;
  size_t num_worlds = 1;
  config.GRID_X(x);
  config.GRID_Y(y);
  config.SAMPLE_PROPORTION(proportion);
  config.RANDOM_ANCESTOR(0);
  config.TASK_TYPE(1);
  config.UPDATES(1);
  config.NUM_POPULATIONS(num_worlds);

  WHEN("We require a world to sample itself and it has more hosts than requested"){
    config.POP_SIZE(-1);
    world.Populate();
    REQUIRE(world.GetNumOrgs() == num_worlds);

    size_t sgp_world_pos = 0;
    SGPWorld& sgp_world = world.GetOrg(sgp_world_pos);
    REQUIRE(sgp_world.GetNumOrgs() == world_size);

    world.SampleSelf(sgp_world_pos);

    THEN("It samples itself until it gets the predetermined number of hosts"){
      REQUIRE(sgp_world.GetNumOrgs() == proportion * world_size);
    }
  }

  WHEN("We require a world to sample itself and it has fewer hosts than requested"){
    config.POP_SIZE(proportion * world_size - 100);
    world.Populate();
    REQUIRE(world.GetNumOrgs() == num_worlds);

    size_t sgp_world_pos = 0;
    SGPWorld& sgp_world = world.GetOrg(sgp_world_pos);
    REQUIRE(sgp_world.GetNumOrgs() == proportion * world_size - 100);

    world.SampleSelf(sgp_world_pos);

    THEN("It samples itself until it gets the predetermined number of hosts"){
      REQUIRE(sgp_world.GetNumOrgs() == proportion * world_size);
    }
  }
}

TEST_CASE("SampleSource", "[metapop]") {
  emp::Random random(61);
  SymConfigBase config;
  MetapopWorld world(random, &config);
  
  double proportion = 0.5;
  int x = 10;
  int y = 100;
  size_t world_size = x * y;
  size_t num_worlds = 2;
  config.GRID_X(x);
  config.GRID_Y(y);
  config.SAMPLE_PROPORTION(proportion);
  config.RANDOM_ANCESTOR(0);
  config.TASK_TYPE(1);
  config.UPDATES(1);
  config.NUM_POPULATIONS(num_worlds);

  WHEN("We require a world to sample a different and the source world has more hosts than requested"){
    config.POP_SIZE(-1);
    world.Populate();
    REQUIRE(world.GetNumOrgs() == num_worlds);

    size_t source_world_pos = 0;
    size_t dest_world_pos = 1;
    SGPWorld& source_world = world.GetOrg(source_world_pos);
    SGPWorld& dest_world = world.GetOrg(dest_world_pos);
    dest_world.Clear();
    REQUIRE(source_world.GetNumOrgs() == world_size);
    REQUIRE(dest_world.GetNumOrgs() == 0);

    world.SampleSource(dest_world_pos, source_world_pos);

    THEN("The destination world samples the source world"){
      REQUIRE(dest_world.GetNumOrgs() == proportion * world_size);
    }
  }

  WHEN("We require a world to sample a different and the source world has fewer hosts than requested"){
    config.POP_SIZE(proportion * world_size - 100);
    world.Populate();
    REQUIRE(world.GetNumOrgs() == num_worlds);

    size_t source_world_pos = 0;
    size_t dest_world_pos = 1;
    SGPWorld& source_world = world.GetOrg(source_world_pos);
    SGPWorld& dest_world = world.GetOrg(dest_world_pos);
    dest_world.Clear();
    REQUIRE(source_world.GetNumOrgs() == proportion * world_size - 100);
    REQUIRE(dest_world.GetNumOrgs() == 0);

    world.SampleSource(dest_world_pos, source_world_pos);

    THEN("The destination world samples the source world"){
      REQUIRE(dest_world.GetNumOrgs() == proportion * world_size);
    }
  }
}