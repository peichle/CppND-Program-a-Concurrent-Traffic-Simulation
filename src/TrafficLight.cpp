#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> ulck(_mtx);
    _cond.wait(ulck, [this]{return !_message.empty();});

    T msg = std::move(_message.back());
    _message.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lck(_mtx);
    _message.clear();
    _message.emplace_back(std::move(msg));
    _cond.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight(){}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    TrafficLightPhase current = TrafficLightPhase::red;
    while (current != TrafficLightPhase::green)
    {
        current = _TrafficLightQueue.receive();
    }
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    time_t curr_t, total_t;

    while(true){        
        // Duration calculation according to: https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
        std::random_device rd; // obtain a random number from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_int_distribution<> distr(4, 6); // define the range

        int duration = distr(gen);
        curr_t = time(NULL);
        total_t = curr_t + duration;

        while (curr_t != total_t)
        {
            curr_t = time(NULL);
            // Sleep 1ms
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        if (_currentPhase == TrafficLightPhase::green)
        {
            _currentPhase = TrafficLightPhase::red;
        }
        else
        {
            _currentPhase = TrafficLightPhase::green;
        }

        std::cout << "Send message" << std::endl;
        _TrafficLightQueue.send(std::move(_currentPhase));

    }

}

