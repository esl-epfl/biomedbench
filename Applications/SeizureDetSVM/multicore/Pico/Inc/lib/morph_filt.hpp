/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////
// Author:          ESL team                    //  
// Optimizations:   Dimitrios Samakovlis        //
/////////////////////////////////////////////////



#ifndef MORPH_FILT_HPP_
#define MORPH_FILT_HPP_

#include <stdint.h>
#include <cassert>
#include "cqueue.hpp"

/**
 * Returns the minimum bit whose decimal number would be at least n.
 */
constexpr int next_bigger_bit(unsigned n) {
  return (sizeof(unsigned)*8 - __builtin_clz((n-1) | 1));
}

// Eroder ─────────────────────────────────────────────────────────────────────

/**
 * For each position of an input sequence,
 * finds the minimum over a window centered in that element.
 */
template<int WINDOW, typename T = int16_t>
class Eroder {
 public:
  static constexpr int latency() {
    return WINDOW/2;
  }

  inline T process(T input) {
    // We maintain a queue where only the last WINDOW elements can be,
    // in order to find the minimum over those elements fast.
    ++pos;
    // If the new element is smaller than another that it is in the queue,
    // we can remove the older one because the new one
    // would stay more time in the queue than the older and it is
    // always an smaller element.
    while (!cq.empty() && input < cq.back_val()) {
      cq.pop_back();
    }
    // We add the new element to the queue.
    // By the previous while, the queue is in increasing (front to back) order.
    cq.push_back(pos, input);
    // Remove the element (if it has not been already removed)
    // which is now WINDOW positions away from the current position,
    if (WINDOW <= pos - cq.front_pos()) {
      cq.pop_front();
    }
    // The smallest element in this window is in the front of the queue.
    return cq.front_val();
  }

 private:
  poscqueue<next_bigger_bit(WINDOW+1), T> cq;
  unsigned pos = 0;
};

// ArgEroder ──────────────────────────────────────────────────────────────────

//TODO
/**
 * For each position of an input sequence,
 * finds the minimum over a window centered in that element.
 */
template<int WINDOW, typename T>
class ArgEroder {
 public:
  static constexpr int latency() {
    return WINDOW/2;
  }

  ArgEroder(const T* buffer) : buffer_(buffer) {}

  inline T process(uint16_t pos) {
    // We maintain a queue where only the last WINDOW elements can be,
    // in order to find the minimum over those elements fast.
    // assert(cq_.empty() || pos >= cq_.back());
    // If the new element is smaller than another that it is in the queue,
    // we can remove the older one because the new one
    // would stay more time in the queue than the older and it is
    // always an smaller element.
    while (!cq_.empty() && buffer_[pos] < buffer_[cq_.back()]) {
      cq_.pop_back();
    }
    // We add the new element to the queue.
    // By the previous while, the queue is in increasing (front to back) order.
    cq_.push_back(pos);
    // Remove the element (if it has not been already removed)
    // which is now WINDOW positions away from the current position,
    if (WINDOW <= pos - cq_.front()) {
      cq_.pop_front();
    }
    // The smallest element in this window is in the front of the queue.
    return cq_.front();
  }

  inline void applyOffset(int16_t off) {
    cq_.sumToAll(off);
  }

 private:
  const T* buffer_;
  cqueue<next_bigger_bit(WINDOW+1), uint16_t> cq_;
};

// Dilator ────────────────────────────────────────────────────────────────────

/**
 * For each position of an input sequence,
 * finds the maximum over a window centered in that element.
 */
template<int WINDOW, typename T = int16_t>
class Dilator {
 public:
  static constexpr int latency() {
    return (WINDOW-1)/2;
  }

  inline T process(T input) {
    ++pos;
    while (!cq.empty() && input > cq.back_val()) {
      cq.pop_back();
    }
    cq.push_back(pos, input);
    if (WINDOW <= pos - cq.front_pos()) {
      cq.pop_front();
    }
    return cq.front_val();
  }

 private:
  poscqueue<next_bigger_bit(WINDOW+1), T> cq;
  unsigned pos;
};

// ArgDilator ─────────────────────────────────────────────────────────────────

//TODO
/**
 * For each position of an input sequence,
 * finds the minimum over a window centered in that element.
 */
template<int WINDOW, typename T>
class ArgDilator {
 public:
  static constexpr int latency() {
    return WINDOW/2;
  }

  ArgDilator(const T* buffer) : buffer_(buffer) {}

  inline T process(uint16_t pos) {
    // assert(cq_.empty() || pos >= cq_.back());
    while (!cq_.empty() && buffer_[pos] > buffer_[cq_.back()]) {
      cq_.pop_back();
    }
    cq_.push_back(pos);
    if (WINDOW <= pos - cq_.front()) {
      cq_.pop_front();
    }
    return cq_.front();
  }

  inline void applyOffset(int16_t off) {
    cq_.sumToAll(off);
  }

 private:
  const T* buffer_;
  cqueue<next_bigger_bit(WINDOW+1), uint16_t> cq_;
};

// Opener ─────────────────────────────────────────────────────────────────────

/**
 * An Eroder followed by a Dilator.
 */
template<int WINDOW, typename T = int16_t>
class Opener {
 public:
  static constexpr int latency() {
    return Eroder<WINDOW, T>::latency() + Dilator<WINDOW, T>::latency();
  }

  inline T process(T input) {
    return dilator.process(eroder.process(input));
  }

 private:
  Eroder<WINDOW, T> eroder;
  Dilator<WINDOW, T> dilator;
};

// Closer ─────────────────────────────────────────────────────────────────────

/**
 * A Dilator followed by an Eroder.
 */
template<int WINDOW, typename T = int16_t>
class Closer {
 public:
  static constexpr int latency() {
    return Dilator<WINDOW, T>::latency() + Eroder<WINDOW, T>::latency();
  }

  inline T process(T input) {
    return eroder.process(dilator.process(input));
  }

 private:
  Dilator<WINDOW, T> dilator;
  Eroder<WINDOW, T> eroder;
};

// Baseliner ──────────────────────────────────────────────────────────────────

/**
 * Uses an Opener followed by a Closer to delineate the baseline of a sequence.
 * The sizes need to be adjusted to the sequence morphology.
 */
template<int OPEN_SIZE, int CLOSE_SIZE, typename T = int16_t>
class Baseliner {
 public:
  static constexpr int latency() {
    return Eroder<OPEN_SIZE, T>::latency() +
        Dilator<OPEN_SIZE + CLOSE_SIZE - 1, T>::latency() +
        Eroder<CLOSE_SIZE, T>::latency();
  }

  inline T process(T input) {
    return eroder1.process(dilator.process(eroder0.process(input)));
  }

 private:
  // The two consecutives dilations are merged into a single dilation.
  Eroder<OPEN_SIZE, T> eroder0;
  Dilator<OPEN_SIZE + CLOSE_SIZE - 1, T> dilator;
  Eroder<CLOSE_SIZE, T> eroder1;
};

// BaselineFilt ───────────────────────────────────────────────────────────────

/**
 * A filter that eliminates the baseline from an input sequence.
 */
template<int OPEN_SIZE, int CLOSE_SIZE, typename T = int16_t>
class BaselineFilt {
 public:
  static constexpr int latency() {
    return Baseliner<OPEN_SIZE, CLOSE_SIZE, T>::latency();
  }

  inline T process(T input) {
    T old_input = buffer[pos];
    buffer[pos] = input;
    ++pos;
    if (pos == latency()) {
        pos = 0;
    }
    return old_input - baseliner.process(input);
  }

 private:
  Baseliner<OPEN_SIZE, CLOSE_SIZE, T> baseliner;
  T buffer[latency()] = {0};
  int pos = 0;
};

// NegBaseliner ───────────────────────────────────────────────────────────────

/**
 * Uses a Closer followed by an Opener to delineate the baseline of a sequence.
 * The sizes need to be adjusted to the sequence morphology.
 */
template<int OPEN_SIZE, int CLOSE_SIZE, typename T = int16_t>
class NegBaseliner {
 public:
  static constexpr int latency() {
    return Dilator<CLOSE_SIZE>::latency() +
        Eroder<CLOSE_SIZE + OPEN_SIZE - 1, T>::latency() +
        Dilator<OPEN_SIZE, T>::latency();
  }

  inline T process(T input) {
    return dilator1.process(eroder.process(dilator0.process(input)));
  }

 private:
  // The two consecutives erosions are merged into a single erosion.
  Dilator<CLOSE_SIZE> dilator0;
  Eroder<CLOSE_SIZE + OPEN_SIZE - 1, T> eroder;
  Dilator<OPEN_SIZE, T> dilator1;
};

// NegBaselineFilt ────────────────────────────────────────────────────────────

/**
 * A filter that eliminates the baseline from an input sequence.
 */
template<int OPEN_SIZE, int CLOSE_SIZE, typename T = int16_t>
class NegBaselineFilt {
 public:
  static constexpr int latency() {
    return Baseliner<OPEN_SIZE, CLOSE_SIZE, T>::latency();
  }

  inline T process(T input) {
    T old_input = buffer[pos];
    buffer[pos] = input;
    ++pos;
    if (pos == latency()) {
        pos = 0;
    }
    return old_input - baseliner.process(input);
  }

 private:
  NegBaseliner<OPEN_SIZE, CLOSE_SIZE, T> baseliner;
  T buffer[latency()];
  int pos = 0;
};

// HighFreqFilt ───────────────────────────────────────────────────────────────

/**
 * A filter that eliminates the high frequeny noise.
 * Targetted for ECG signals taken at 250Hz.
 */
template<typename T = int16_t>
class HighFreqFilt250Hz {
 public:
  static constexpr int latency() {
    return 2*(WINDOW/2);
  }

  inline T process(T input) {
    static constexpr T B[WINDOW*WINDOW] = {0, 0, 1, 5, 1,
                                           1, 0, 0, 1, 5,
                                           5, 1, 0, 0, 1,
                                           1, 5, 1, 0, 0,
                                           0, 1, 5, 1, 0};
    original[pos] = input;
    const T* Bi = B + WINDOW*pos;
    dilation[pos] = original[0] + Bi[0];
    erosion[pos] = original[0] - Bi[0];
    // Given the small size of this loops,
    // it is faster to run them than to try to
    // use a better algorithm.
    for (int i = 1; i < WINDOW; ++i) {
      if (original[i] + Bi[i] > dilation[pos]) {
        dilation[pos] = original[i] + Bi[i];
      }
      if (original[i] - Bi[i] < erosion[pos]) {
        erosion[pos] = original[i] - Bi[i];
      }
    }
    T closing = dilation[0];
    T opening = erosion[0];
    for (int i = 1; i < WINDOW; ++i) {
      if (dilation[i] < closing) {
        closing = dilation[i];
      }
      if (erosion[i] > opening) {
        opening = erosion[i];
      }
    }
    ++pos;
    if (pos == WINDOW) pos = 0;
    return (opening + closing)/2;
  }

 private:
  static constexpr int WINDOW = 5;
  T original[WINDOW];
  T dilation[WINDOW];
  T erosion[WINDOW];
  int pos = 0;
};

#endif  // MORPH_FILT_HPP_

