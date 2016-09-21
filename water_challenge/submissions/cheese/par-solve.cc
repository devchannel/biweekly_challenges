#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <future>
#include <array>
#include <chrono>

struct Point {
  uint16_t height;
  uint64_t dist;
};

// mountain shape with that is already filled
struct Segment {
  std::vector<Point> left;
  std::vector<Point> right;
  uint64_t volume{0};
  uint64_t width{0};
};

uint64_t combineHelper(std::vector<Point> &lower_outer,
                       const std::vector<Point> &lower_inner,
                       const std::vector<Point> &higher_inner,
                       uint64_t lower_width) {

  auto fill_height = lower_outer.back().height;

  uint64_t volume = 0;
  size_t last_dist = 0;
  uint16_t last_height = 0;
  for (auto &p : lower_inner) {
    volume += (fill_height - last_height) * (p.dist - last_dist);
    last_dist = p.dist;
    last_height = p.height;
  }

  last_dist = 0;
  last_height = 0;
  auto it = higher_inner.begin();
  for (; it->height < fill_height; ++it) {
    volume += (fill_height - last_height) * (it->dist - last_dist);
    last_dist = it->dist;
    last_height = it->height;
  }
  volume += (fill_height - last_height) * (it->dist - last_dist);

  if (it->height == fill_height) {
    ++it;
  }

  for (; it < higher_inner.end(); ++it) {
    lower_outer.push_back({it->height, it->dist + lower_width});
  }

  return volume;
}

// combine 2 segments, filling gap between them
Segment operator+(Segment&& lhs, Segment&& rhs) {
  Segment result{std::move(lhs.left), std::move(rhs.right),
                 lhs.volume + rhs.volume, lhs.width + rhs.width};

  if (result.left.empty() && result.right.empty()) {
    // volume is 0 on both sides

  } else if (result.left.empty()) {
    result.left = std::move(rhs.left);
    for (auto& p : result.left) p.dist += lhs.width;

  } else if (result.right.empty()) {
    result.right = std::move(lhs.right);
    for (auto& p : result.right) p.dist += rhs.width;

  } else {
    if(result.left.back().height > result.right.back().height) {
      result.volume += combineHelper(result.right, rhs.left, lhs.right, rhs.width);

    } else if(result.left.back().height < result.right.back().height) {
      result.volume += combineHelper(result.left, lhs.right, rhs.left, lhs.width);

    } else {
      // segment high is equal, just sum volume
      auto fill_height = result.left.back().height;

      size_t last_dist = 0;
      uint16_t last_height = 0;
      for (auto& p : lhs.right) {
        result.volume += (fill_height - last_height) * (p.dist - last_dist);
        last_dist = p.dist;
        last_height = p.height;
      }

      last_dist = 0;
      last_height = 0;
      for (auto& p : rhs.left) {
        result.volume += (fill_height - last_height) * (p.dist - last_dist);
        last_dist = p.dist;
        last_height = p.height;
      }
    }
  }

  return result;
}

template <typename It>
Segment processLinear(It beg, It end) {
  Segment result;
  result.width = static_cast<uint64_t>(std::distance(beg, end));

  uint16_t top = 0;

  // left to right pass
  for (auto it = beg; it != end; ++it) {
    if (*it > top) {
      top = *it;
      result.left.push_back(
          {top, static_cast<uint32_t>(std::distance(beg, it))});
    }
  }

  top = 0;

  // right to left pass
  bool uphill = true;
  auto left_top = result.left.rbegin();

  auto rbeg = std::reverse_iterator<It>(end);
  auto rend = std::reverse_iterator<It>(beg);

  for (auto it = rbeg; it != rend; ++it) {
    if (uphill) {

      if (*it > top) {
        top = *it;
        result.right.push_back(
            {top, static_cast<uint32_t>(std::distance(rbeg, it))});
        if (top == left_top->height) {
          uphill = false;
        }
      }

      result.volume += top - *it;

    } else {

      if (std::distance(it, rend) - 1 < left_top->dist) {
        ++left_top;
        if (left_top == result.left.rend()) {
          break;
        }
      }

      result.volume += left_top->height - *it;

    }
  }

  return result;
}

const size_t kNrThreads = 4;

template <typename It>
uint64_t processParallel(It it, It end) {
  auto chunk_size = std::distance(it, end) / kNrThreads;

  // this should be at something around 2^20
  // but I want to see how it compares while always using threads
  if (chunk_size < 1024) {
    return processLinear(it, end).volume;
  }

  std::array<std::future<Segment>, kNrThreads> tasks;
  for (auto& t : tasks) {
    auto task_end = it + chunk_size;
    t = std::async(std::launch::async,
                   [=]() { return processLinear(it, task_end); });
    it = task_end;
  }

  Segment seg{};
  for (auto& t : tasks) {
    seg = std::move(seg) + t.get();
  }

  return seg.volume;
}


// Read 'amount' comma seperated numbers from file 'fn'.
std::vector<uint16_t> getData(const char* fn, size_t amount) {
  std::vector<uint16_t> data;
  data.reserve(amount);

  std::fstream fs{fn};
  for (size_t i = 0; i < amount; ++i) {
    uint16_t x;
    fs >> x;
    if (fs.fail()) {
      throw std::exception();
    }
    data.push_back(x);
    fs.ignore(1, ',');
  }


  return data;
}

int main(int argc, char** argv) {
  namespace c = std::chrono;

  if (argc < 3) {
    return -1;
  }

  auto data = getData(argv[1], atoll(argv[2]));

  const auto start = c::system_clock::now();

  auto result = processParallel(data.begin(), data.end());

  const auto stop = c::system_clock::now();

  std::cout << result << '\n'
            << static_cast<double>(
                   c::duration_cast<c::nanoseconds>(stop - start).count()) /
                   1000000
            << '\n';
}
