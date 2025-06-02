#include "../../default_mode/DataNodes.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"

// copied (with return type modifications) from Empirical's DataFile test
int compareFiles(const std::string& p1, const std::string& p2) {
  // From mtrw's answer to https://stackoverflow.com/questions/6163611/compare-two-files
  std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
  std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

  if (f1.fail() || f2.fail()) {
    return 0; //file problem
  }

  if (f1.tellg() != f2.tellg()) { //size mismatch
    return 1;
  }

  //seek back to beginning and use std::equal to compare contents
  f1.seekg(0, std::ifstream::beg);
  f2.seekg(0, std::ifstream::beg);
  if (std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
    std::istreambuf_iterator<char>(),
    std::istreambuf_iterator<char>(f2.rdbuf()))) return 2;
  return 3;
}

TEST_CASE("WriteOrgDumpFile", "[default]") {
  emp::Random random(17);
  SymConfigBase config;
  SymWorld world(random, &config);
  size_t world_size = 4;
  world.Resize(world_size);

  REQUIRE(world.GetNumOrgs() == 0);
  std::string test_file_name = "source/test/default_mode_test/test_data_files/temp/new_org_dump_file.dat";
  world.WriteOrgDumpFile(test_file_name);
  
  REQUIRE(compareFiles(test_file_name, "source/test/default_mode_test/test_data_files/org_dump_file.txt") == 2);
}