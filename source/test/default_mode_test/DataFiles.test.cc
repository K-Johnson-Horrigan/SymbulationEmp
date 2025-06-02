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
int test_fun() {
  static int val = 10;
  return val += 3;
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

TEST_CASE("Test DataFile", "[default]") {
  int test_int = 5;

  emp::DataFile dfile("new_test_file.dat");

  REQUIRE(dfile.GetFilename() == "new_test_file.dat");

  emp::DataMonitor<int> data_squares;
  emp::DataMonitor<uint64_t> data_cubes;

  dfile.AddCurrent(data_squares);
  dfile.AddCurrent(data_cubes);
  dfile.AddTotal(data_cubes);
  dfile.AddMin(data_cubes);
  dfile.AddMax(data_cubes);
  dfile.AddFun<int>(test_fun);
  dfile.AddVar<int>(test_int);

  for (size_t i = 0; i < 10; i++) {
    test_int += i;
    data_squares.Add((int)(i * i));
    data_cubes.Add(i * i * i);
    dfile.Update();

    // std::cout << i << std::endl;
  }

  dfile.SetupLine("[[", ":", "]]\n");
  for (size_t i = 10; i < 20; i++) {
    data_squares.Add((int)(i * i));
    data_cubes.Add(i * i * i);
    dfile.Update();

    // std::cout << i << std::endl;
  }

  REQUIRE(compareFiles("new_test_file.dat", "source/test/default_mode_test/test_file.dat") == 2);
}