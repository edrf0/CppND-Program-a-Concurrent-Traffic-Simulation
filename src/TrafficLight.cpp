#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] {
        return !_queue.empty();
    });
    T message = std::move(_queue.back());
    _queue.pop_back();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    {
        std::lock_guard<std::mutex> lock{_mutex};
        _queue.push_back(std::move(msg));
    }
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = red;
}

TrafficLight::~TrafficLight() {
    _isSimulationRunning = false;
    _queue.send(std::move(green));
    for (auto& thread : threads) if (thread.joinable()) thread.join();
    threads.clear();
}

void TrafficLight::waitForGreen()
{
    while (_isSimulationRunning) {
        auto phase = _queue.receive();
        if (phase == green) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase phase) {
    _currentPhase = phase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
    // std::thread t(&TrafficLight::cycleThroughPhases, this);
    // t.detach();
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::cout << "TrafficLight thread STARTED for ID: " << _id << std::endl;

    std::mt19937_64 engine(std::random_device{}());
    std::uniform_real_distribution<double> distribution(4.0, 6.0);

    double cycleDuration = distribution(engine) * 1000;
    double elapsedTime{};
    std::chrono::system_clock::time_point endTime{};
    auto startTime = std::chrono::system_clock::now();
    while (_isSimulationRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        endTime = std::chrono::system_clock::now();
        elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        if (elapsedTime >= cycleDuration) {
            setCurrentPhase(_currentPhase == red ? green:red);
            _queue.send(std::move(_currentPhase));
            cycleDuration = distribution(engine) * 1000;
            startTime = std::chrono::system_clock::now();
        }
    }
    std::cout << "TrafficLight thread FINISHED for ID: " << _id << std::endl;
}
