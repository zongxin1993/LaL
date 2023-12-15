#include "CGraph.h"

using namespace CGraph;
class MyEmptyNode : public GNode {
 public:
  CStatus run() override {
    return CStatus();
  }
};
void tutorial_dag(long count) {
  // 简单dag场景，对应第三个例子，2thread，dag，100w次
  GPipelinePtr pipeline = GPipelineFactory::create();
  CStatus status;
  GElementPtr a,b1,b2,c1,c2,d;
  UThreadPoolConfig config;
  config.default_thread_size_ = 2;
  config.secondary_thread_size_ = 0;
  config.max_task_steal_range_ = 1;
  config.max_thread_size_ = 2;
  config.primary_thread_empty_interval_ = 1;
  config.primary_thread_busy_epoch_ = 3000;
  config.monitor_enable_ = false;    // 关闭扩缩容机制
  config.primary_thread_policy_ = CGRAPH_THREAD_SCHED_RR;
  config.primary_thread_priority_ = 10;
  pipeline->setUniqueThreadPoolConfig(config);
  pipeline->setAutoCheck(false);
  pipeline->registerGElement<MyEmptyNode>(&a);
  pipeline->registerGElement<MyEmptyNode>(&b1, {a});
  pipeline->registerGElement<MyEmptyNode>(&b2, {b1});
  pipeline->registerGElement<MyEmptyNode>(&c1, {a});
  pipeline->registerGElement<MyEmptyNode>(&c2, {c1});
  pipeline->registerGElement<MyEmptyNode>(&d, {b2, c2});
  pipeline->setGEngineType(GEngineType::DYNAMIC);
  status += pipeline->init();
  /** 其中流程进行计时 **/
  auto start_ts_ = std::chrono::high_resolution_clock::now();
  for (long t = 0; t < count; t++) {
    pipeline->run();
  }
  std::chrono::duration<double, std::milli> span = std::chrono::high_resolution_clock::now() - start_ts_;
  printf("----> [CGraph] time cost is : [%0.2lf] ms \n",span.count());
  /*******************/
  status += pipeline->destroy();
  GPipelineFactory::remove(pipeline);
}

int main(int argc, char* argv[]) {

  long testCount = 1000000;
  int round = 1;

  if (argc == 3) {
    testCount = atol(argv[1]);
    round = atoi(argv[2]);
  }

  for (int i = 0; i < round; i++) {
    tutorial_dag(testCount);
  }
  return 0;
}