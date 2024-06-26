#include <iostream>
#include <random>
#include "TrafficLight.h"


// For print statement in FP4.a
std::ostream& operator<<(std::ostream& os, TrafficLightPhase phase) {
    switch (phase) {
        case TrafficLightPhase::red:
            os << "red";
            break;
        case TrafficLightPhase::green:
            os << "green";
            break;
        default:
            os << "unknown";
            break;
    }
    return os;
}

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(_mutex);  // FP.5a
    _condition.wait(lck, [this] {return !_queue.empty();});  // FP.5a

    T msg = std::move(_queue.back());  // FP.5a
    _queue.pop_back();  // FP.5a

    return msg;  // FP.5a
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);  // FP4.a
    // std::cout << "Adding message " << msg; //<< " to the queue.\n";  // FP4.a
    std::cout << "Adding message " << msg << " to the queue.\n";
    _queue.push_back(std::move(msg));  // FP4.a
    _condition.notify_one();  // FP4.a
}


/* Implementation of class "TrafficLight" */

/*
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}
*/

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.

    while (true){
        TrafficLightPhase color = _messageQueue.receive();
        if (color == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    /*
    FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called.
    To do this, use the thread queue in the base class.
    */
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));  // FP.2b

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    // --- Start FP.2a ---

    // Initialize variables
    long duration = 0;
    auto t0 = std::chrono::system_clock::now();
    auto t1 = std::chrono::system_clock::now();

    // Set duration for first cycle
    // The cycle duration should be a random value between 4 and 6 seconds
    std::uniform_int_distribution<int> dist(4000, 6000);
    std::random_device rd;
    std::mt19937 gen(rd());
    int cycleDuration = dist(gen);

    while (true){
        if(duration >= cycleDuration){
            // std::cout << "duration = " << duration << ", cycleDuration = " << cycleDuration << "\n";
            t0 = std::chrono::system_clock::now();
            // toggled the current phase of the traffic light between red and green
            if (_currentPhase == TrafficLightPhase::red){
                _currentPhase = TrafficLightPhase::green;
            }
            else{
                _currentPhase = TrafficLightPhase::red;
            }
            // send an update method to the message queue using move semantics
            _messageQueue.send(std::move(_currentPhase));

            // Set duration of next cycle:
            std::random_device rd;
            std::mt19937 gen(rd());
            cycleDuration = dist(gen);
        }
        // the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto t1 = std::chrono::system_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>( t1 - t0).count();
    }
    // -- Finish FP2.a ---

}
