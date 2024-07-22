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
// Porting to X-Heep: Francesco Poluzzi         //
/////////////////////////////////////////////////



#ifndef CQUEUE_HPP_
#define CQUEUE_HPP_

extern "C" {
    #include <stdint.h>
}

/**
 * cqueue: Circular Queue
 *
 * A circular queue of size a power of 2.
 * Because the size is a number of two,
 * all of the operations are performed
 * without branches or complex operations.
 *
 * Trying to push more elements to the queue than it supports
 * is undefined behaviour.
 */
template<int NUM_BITS, typename T>
class cqueue {
  public:
    static const uint16_t QUEUE_SIZE = 1<<NUM_BITS;
    static const uint16_t QUEUE_MASK = QUEUE_SIZE-1;

    inline void clear() {
        front_ = 0;
        back_ = 0;
    }

    inline bool empty() {
        return front_ == back_;
    }

    inline T front() {
        return q_[(front_+1)&QUEUE_MASK];
    }

    inline T back() {
        return q_[back_];
    }

    inline void push_back(T val) {
        back_ = (back_+1)&QUEUE_MASK;
        q_[back_] = val;
    }

    inline void pop_back() {
        back_ = (back_-1)&QUEUE_MASK;
    }

    inline void pop_front() {
        front_ = (front_+1)&QUEUE_MASK;
    }

    inline void sumToAll(const T& val) {
        for (uint16_t i = back_; i != front_; i = (i-1)&QUEUE_MASK) {
            q_[i] += val;
        }
    }

  private:
    // Storage: q_(front_..back_]
    uint16_t front_ = 0, back_ = 0;
    T q_[QUEUE_SIZE];
};

/**
 * cqueue: Circular Queue
 *
 * A circular queue of size a power of 2.
 * Because the size is a number of two,
 * all of the operations are performed
 * without branches or complex operations.
 *
 * Trying to push more elements to the queue than it supports
 * is undefined behaviour.
 */
template<int NUM_BITS, typename T>
struct poscqueue {
    static const uint16_t QUEUE_SIZE = 1<<NUM_BITS;
    static const uint16_t QUEUE_MASK = QUEUE_SIZE-1;

    // Storage: poss(front..back] and vals(front..back]
    uint16_t front = 0, back = 0;
    uint16_t poss[QUEUE_SIZE];
    T vals[QUEUE_SIZE];

    inline void clear() {
        front = 0;
        back = 0;
    }

    inline bool empty() {
        return front == back;
    }

    inline uint16_t front_pos() {
        return poss[(front+1)&QUEUE_MASK];
    }

    inline T front_val() {
        return vals[(front+1)&QUEUE_MASK];
    }

    inline T back_val() {
        return vals[back];
    }

    inline void push_back(uint16_t pos, T val) {
        back = (back+1)&QUEUE_MASK;
        poss[back] = pos;
        vals[back] = val;
    }

    inline void pop_back() {
        back = (back-1)&QUEUE_MASK;
    }

    inline void pop_front() {
        front = (front+1)&QUEUE_MASK;
    }
};

#endif  // CQUEUE_HPP_

