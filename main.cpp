#include <vector>

struct UserInfo {
  int rbNeed;
  int beam;
  int id;
};
struct Interval {
  int start, end;
  std::vector<int> users;
};

std::vector<Interval>
Solver(int N, int M, int K, int J, int L,
       std::vector<Interval> reservedRBs, // vector users is empty
       std::vector<UserInfo> userInfos);

int main() { return 0; }