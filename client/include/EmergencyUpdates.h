#pragma once

#include <map>
#include <string>
#include <mutex>
// EmergencyUpdates tracks all updates for a specific user in a specific emergency channel
// It maintains counters for active events and events where forces have arrived, along with
// general information and a time-ordered map of events

class EmergencyUpdates final {
private:
    std::map<std::string, std::string> generalInfo_; // General emergency information
    int activeCount_; //Whether the emergency is still active
    int forcesArrivedCount_; //Whether emergency forces have arrived at scene
    std::map<int, std::pair<std::string, std::string>> timeToEvents_; //Time-ordered events (time -> (event name, description))

public:
    std::mutex mut_;

    EmergencyUpdates();
    EmergencyUpdates(const EmergencyUpdates &other);

    void addGeneralUpdate(std::string key, std::string value);
    void addEvent(int time, std::string eventName, std::string description);
    void incrementActive(); 
    void incrementForcesArrived(); 
    

    const std::map<std::string, std::string> &getGeneralUpdates() const;
    const std::map<int, std::pair<std::string,std::string>> &getEvents() const;
    int getActiveCount() const;
    int getForcesArrivedCount() const;
    
};