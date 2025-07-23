//
// Created by pe on 2021/7/3.
//

#ifndef TWOCAMERA_COUNTTIMER_H
#define TWOCAMERA_COUNTTIMER_H

#include <chrono>

class CountTimer {
public:
    void countStart() {
        using namespace std::chrono;
        start = steady_clock::now();
    }

    void countStop() {
        auto end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        count++;
    }

    double avgInterval() const {
        return double(sum) / count;
    }

    void clear() {
        count = 0;
        sum = 0;
    }

private:
    int count = 0;
    std::chrono::time_point<std::chrono::steady_clock> start;
    int sum = 0;
};


#endif //TWOCAMERA_COUNTTIMER_H
