#include <taskflow/taskflow.hpp>
#include "CGraph.h"

using namespace CGraph;

void demo3(long count) {
  // 简单dag图
  tf::Taskflow taskflow;
  auto [A, B1, B2, C1, C2, D] = taskflow.emplace(
      []() { return CStatus(); },
      []() { return CStatus(); },
      []() { return CStatus(); },
      []() { return CStatus(); },
      []() { return CStatus(); },
      []() { return CStatus(); }
  );
  A.precede(B1, C1);
  B1.precede(B2);
  C1.precede(C2);
  D.succeed(B2, C2);
  // execute the workflow
  tf::Executor executor(2);
  auto start_ts_ = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < count; i++) {
    executor.run(taskflow).wait();
  }
  std::chrono::duration<double, std::milli> span = std::chrono::high_resolution_clock::now() - start_ts_;
  printf("----> [taskflow] time cost is : [%0.2lf] ms \n",span.count());
}

int main(int argc, char* argv[]){
  long testCount = 1000000;
  int round = 1;

  if (argc == 3) {
    testCount = atol(argv[1]);
    round = atoi(argv[2]);
  }

  for (int i = 0; i < round; i++) {
    demo3(testCount);
  }
  return 0;
}
