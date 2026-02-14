#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

enum TrafficLightPhase {
    red,
    green
};

template <class T>
class MessageQueue
{
public:
    void send(T &&phase);
    T receive();
private:
    std::deque<TrafficLightPhase> _queue;
    std::mutex _mutex;
    std::condition_variable _condition;
};

class TrafficLight : public TrafficObject
{
public:
    // constructor / destructor
    TrafficLight();
    ~TrafficLight() override = default;
    // getters / setters
    TrafficLightPhase getCurrentPhase();
    void setCurrentPhase(TrafficLightPhase phase);
    // typical behaviour methods
    void waitForGreen();
    void simulate() override;
private:
    TrafficLightPhase _currentPhase{red};
    // typical behaviour methods
    void cycleThroughPhases();

    MessageQueue<TrafficLightPhase> _queue;

    std::condition_variable _condition;
    std::mutex _mutex;
};

#endif