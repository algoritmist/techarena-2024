#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
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

void removeEmptyAndSort(std::vector<std::vector<UserInfo>> &infos) {
  for (int i = 0; i < infos.size(); ++i) {
    if (infos[i].empty()) {
      while (!infos.empty() && infos.back().empty()) {
        infos.pop_back();
      }
      if (i < infos.size()) {
        swap(infos[i], infos.back());
      }
    }
  }
  sort(infos.begin(), infos.end(),
       [](std::vector<UserInfo> &a, std::vector<UserInfo> &b) {
         return a.back().rbNeed > b.back().rbNeed;
       });
}

template <class T, class Compare>
std::vector<T> kmax(int k, std::vector<T> arr, Compare cmp) {
  if (k < 10) {
    std::vector<T> sorted;
    for (auto &element : arr) {
      if (sorted.size() < k) {
        sorted.push_back(element);
      } else {
        auto it = std::min_element(sorted.begin(), sorted.end(), cmp);
        if (cmp(*it, element)) {
          sorted.erase(it);
          sorted.push_back(element);
        }
      }
    }
    return sorted;
  } else {
    std::sort(arr.rbegin(), arr.rend(), cmp);
    while (arr.size() > k) {
      arr.pop_back();
    }
    return arr;
  }
}

std::vector<std::vector<UserInfo>>
filterUsers(int J, const std::vector<UserInfo> &userInfos) {
  std::vector<std::vector<UserInfo>> infos =
      std::vector(BEAM_MAX, std::vector<UserInfo>());
  for (const UserInfo &info : userInfos) {
    infos[info.beam].push_back(info);
  }
  for (size_t i = 0; i < infos.size(); ++i) {
    sort(infos[i].begin(), infos[i].end(),
         [](UserInfo a, UserInfo b) { return a.rbNeed > b.rbNeed; });
    while (infos[i].size() > J) {
      infos[i].pop_back();
    }
    reverse(infos[i].begin(), infos[i].end());
  }
  removeEmptyAndSort(infos);
  return infos;
}

int count_len(std::vector<Interval> &intervals) {
  int sum = 0;
  for (const Interval &interval : intervals) {
    sum += interval.end - interval.start;
  }
  return sum;
}

bool intervals_intersect(const Interval &a, const Interval &b) {
  return !(a.end < b.start || b.end < a.start);
}

void update_user_vec(std::vector<UserInfo> &infos) {
  if (infos.empty()) {
    return;
  }
  if (infos.back().rbNeed <= 0) {
    infos.pop_back();
  } else {
    int sz = infos.size();
    if (sz >= 2) {
      if (infos.back().rbNeed < infos[sz - 2].rbNeed) {
        infos.pop_back();
      }
    }
  }
}

int mod(int x, int y) {
  while (x >= y) {
    x -= y;
  }
  return x;
}

/*int update_pos(const std::vector<std::vector<UserInfo>> &infos,
               int users_in_interval, int pos) {
  int sz = infos.size();
  int sum_cur = 0;
  for (int i = pos, cnt = 0; cnt < users_in_interval; ++cnt, ++i) {
    int idx = mod(i, sz);
    sum_cur += infos[idx].back().rbNeed;
  }
  int new_pos = mod(pos + users_in_interval, sz);
  int sum_next = 0;
  for (int i = new_pos, cnt = 0; cnt < users_in_interval; ++cnt, ++i) {
    int idx = mod(i, sz);
    sum_next += infos[idx].back().rbNeed;
  }
  return (sum_cur > sum_next) ? pos : new_pos;
}*/

std::vector<Interval>
Solver(int N, int M, int K, int J, int L,
       std::vector<Interval> reservedRBs, // vector users is empty
       std::vector<UserInfo> userInfos) {

  std::vector<Interval> freeRBs = getFReeRbs(M, reservedRBs);

  int max_sum = 0;
  std::vector<Interval> solution;

  int max_len = count_len(freeRBs);

  std::vector<std::vector<UserInfo>> filteredUsersCopy =
      filterUsers(J, userInfos);

  for (int len = 1; len <= max_len; ++len) {
    std::vector<std::vector<UserInfo>> filteredUsers(filteredUsersCopy);
    int interval_idx = 0;
    int l = freeRBs[0].start;
    int r = freeRBs[0].end;
    int sum = 0;
    std::vector<Interval> local_solution;
    std::vector<Interval> user_interval(N + 1,
                                        Interval{.start = -1, .end = -1});
    // int pos = 0;
    for (int j = 1; j <= J && l < freeRBs.back().end; ++j) {
      // std::cout << "iterval " << j << std::endl;
      if (l >= r) {
        ++interval_idx;
        l = freeRBs[interval_idx].start;
        r = freeRBs[interval_idx].end;
      }

      for (auto &user_vec : filteredUsers) {
        if (user_vec.empty()) {
          continue;
        }
        Interval interval{.start = l, .end = r};
        auto user = user_vec.back();
        if (user_interval[user.id].start != -1 &&
            !intervals_intersect(user_interval[user.id], interval)) {
          user_vec.pop_back();
        }
      }

      removeEmptyAndSort(filteredUsers);

      int sz = filteredUsers.size();

      if (sz == 0) {
        break;
      }

      // pos = mod(pos, sz);

      int users_in_interval = std::min(sz, L);
      // pos = update_pos(filteredUsers, users_in_interval, pos);

      std::vector<int> user_ids;

      int _len = std::min(len, r - l);
      int max_gain = 0;

      for (int i = 0; i < users_in_interval; ++i) {
        auto &user = filteredUsers[i].back();
        int gain = std::min(_len, user.rbNeed);
        max_gain = std::max(max_gain, gain);
        sum += gain;
        user.rbNeed -= gain;
        update_user_vec(filteredUsers[i]);
        user_ids.push_back(user.id);
      }

      int new_l = l + std::min(_len, max_gain);
      if(max_gain == 0){
        break;
      }

      for (auto &id : user_ids) {
        user_interval[id] = Interval{.start = l, .end = new_l};
      }
      Interval interval{.start = l, .end = new_l, .users = user_ids};
      local_solution.push_back(interval);
      l = new_l;
    }

    if (sum > max_sum) {
      max_sum = sum;
      solution = local_solution;
    }
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

#ifdef MY_TEST_MODE
int main(int argc, char **argv) {

  const char *test_file_name;
  const char *output_file_name;
  if (argc != 3) {
    std::cout << "usage: naive <path_to_test_file> <path_to_output_file>"
              << std::endl;
    std::cout << "args not provided: using default directory..." << std::endl;
    test_file_name = "../samples/open.txt";
    output_file_name = "../samples/output.txt";
  } else {
    test_file_name = argv[1];
    output_file_name = argv[2];
  }

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
#endif