#include <fstream>
#include <functional>
#include <iostream>
#include <stddef.h>
#include <stdlib.h>
#include <vector>

#define BEAM_MAX 33

struct UserInfo {
  int rbNeed;
  int beam;
  int id;
};
struct Interval {
  int start, end;
  std::vector<int> users;
};

std::vector<Interval> getFReeRbs(int M,
                                 const std::vector<Interval> &reservedRBs) {
  int l = 0;
  std::vector<Interval> its;
  for (const Interval &reserved_interval : reservedRBs) {
    Interval interval{.start = l, .end = reserved_interval.start};
    its.push_back(interval);
    l = reserved_interval.end;
  }
  if (l < M) {
    its.push_back(Interval{.start = l, .end = M});
  }
  return its;
}

template <class T, class Compare>
std::vector<T> kmax(int k, std::vector<T> arr, Compare cmp) {
  std::vector<T> sorted;
  for (auto &element : arr) {
    if (sorted.size() == k) {
      sorted.erase(std::min_element(sorted.begin(), sorted.end(), cmp));
    }
    sorted.push_back(element);
  }
  return sorted;
}

std::vector<std::vector<UserInfo>>
filterUsers(int J, const std::vector<UserInfo> &userInfos) {
  std::vector<std::vector<UserInfo>> infos =
      std::vector(BEAM_MAX, std::vector<UserInfo>());
  for (const UserInfo &info : userInfos) {
    infos[info.beam].push_back(info);
  }
  for (size_t i = 0; i < infos.size(); ++i) {
    infos[i] = kmax(J, infos[i],
                    [](UserInfo a, UserInfo b) { return a.rbNeed < b.rbNeed; });
  }
  return infos;
}

std::pair<int, UserInfo &> closest(int x, std::vector<UserInfo> &users) {
  int min = 1 << 30;
  int index = 0;
  for (int i = 0; i < users.size(); ++i) {
    if (std::abs(x - users[i].rbNeed) < min) {
      min = std::abs(x - users[i].rbNeed);
      index = i;
    }
  }
  return {min, users[index]};
}

int count_len(std::vector<Interval> &intervals) {
  int sum = 0;
  for (const Interval &interval : intervals) {
    sum += interval.end - interval.start;
  }
  return sum;
}

std::vector<Interval>
Solver(int N, int M, int K, int J, int L,
       std::vector<Interval> reservedRBs, // vector users is empty
       std::vector<UserInfo> userInfos) {

  std::vector<Interval> freeRBs = getFReeRbs(M, reservedRBs);
  std::vector<std::vector<UserInfo>> filteredUsersCopy =
      filterUsers(J, userInfos);

  int max_sum = 0;
  std::vector<Interval> solution;

  int len = count_len(freeRBs) / J;

  // for (int len = max_len / J; len <= M; ++len) {
  std::vector<std::vector<UserInfo>> filteredUsers(filteredUsersCopy);
  int interval_idx = 0;
  int l = freeRBs[0].start;
  int r = freeRBs[0].end;
  int sum = 0;
  std::vector<Interval> local_solution;
  std::vector<int> user_ids;
  for (int j = 1; j <= J && l < freeRBs.back().end; ++j) {
    if (l >= r) {
      ++interval_idx;
      l = freeRBs[interval_idx].start;
      r = freeRBs[interval_idx].end;
      for (const int user_id : user_ids) {
        for (std::vector<UserInfo> &vec : filteredUsers) {
          for(auto it = vec.begin(); it != vec.end(); ++it){
            if(it->id == user_id){
              vec.erase(it);
              break;
            }
          }
        }
      }
    }
    std::vector<std::pair<int, UserInfo &>> closest_users;
    for (auto &users : filteredUsers) {
      if (users.empty()) {
        continue;
      }
      closest_users.push_back(closest(len, users));
    }
    std::vector<std::pair<int, UserInfo &>> best_users =
        kmax(L, closest_users,
             [](std::pair<int, UserInfo &> a, std::pair<int, UserInfo &> b) {
               return a.first < b.first;
             });

    std::vector<int> users_ids;

    int _len = std::min(len, r - l);

    for (int i = 0; i < best_users.size(); ++i) {
      UserInfo &info = best_users[i].second;
      users_ids.push_back(info.id);
      int gain = std::min({info.rbNeed, _len});
      sum += gain;
      info.rbNeed -= gain;
      user_ids.push_back(info.id);
    }

    Interval interval{.start = l, .end = l + _len, .users = users_ids};
    local_solution.push_back(interval);
    l += _len;
  }
  if (sum > max_sum) {
    max_sum = sum;
    solution = local_solution;
  }
  //}
  // std::cout << "Sum: " << max_sum << std::endl;
  return solution;
}

int calculate_sum(std::vector<Interval> solution, std::vector<UserInfo> users) {
  int sum = 0;
  for (const auto &interval : solution) {
    for (const auto &user_id : interval.users) {
      std::vector<UserInfo>::iterator it =
          std::find_if(users.begin(), users.end(), [&user_id](UserInfo info) {
            return info.id == user_id;
          });
      int gain = std::min(interval.end - interval.start, it->rbNeed);
      it->rbNeed -= gain;
      sum += gain;
    }
  }
  return sum;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cout << "usage: naive <path_to_test_file> <path_to_output_file>"
              << std::endl;
    return -1;
  }

  char *test_file_name = argv[1];
  char *output_file_name = argv[2];

  std::ifstream test_file;
  test_file.open(test_file_name);

  std::ofstream output_file;
  output_file.open(output_file_name);

  if (!test_file.is_open()) {
    std::cerr << "can't open test file or it does not exist" << std::endl;
    return -2;
  }

  if (!output_file.is_open()) {
    std::cerr << "can't open output file or it does not exist" << std::endl;
    return -2;
  }

  int tests;
  test_file >> tests;
  for (int t = 1; t <= tests; ++t) {
    int N, M, K, J, L;
    test_file >> N >> M >> K >> J >> L;
    std::vector<Interval> reservedRbs;
    for (int i = 0; i < K; ++i) {
      int start, end;
      test_file >> start >> end;
      reservedRbs.push_back(Interval{.start = start, .end = end});
    }
    std::vector<UserInfo> users;
    for (int i = 1; i <= N; ++i) {
      int a, b;
      test_file >> a >> b;
      users.push_back(UserInfo{.rbNeed = a, .beam = b, .id = i});
    }
    std::vector<Interval> solution = Solver(N, M, K, J, L, reservedRbs, users);
    int sum = calculate_sum(solution, users);
    output_file << "Test: " << t << std::endl;
    output_file << "Sum: " << sum << std::endl;
    for (const Interval &interval : solution) {
      output_file << "Interval [" << interval.start << ", " << interval.end
                  << ")" << std::endl;
      output_file << "Users:";
      for (const auto &user : interval.users) {
        output_file << " " << user;
      }
      output_file << std::endl;
    }
  }

  return 0;
}
