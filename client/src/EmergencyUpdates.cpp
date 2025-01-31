#include "../include/EmergencyUpdates.h"
#include <iostream>

//EmergencyUpdates: Tracks event data and statistics
EmergencyUpdates::EmergencyUpdates() 
    : generalInfo_(), activeCount_(0), forcesArrivedCount_(0), timeToEvents_(), mut_() {}

EmergencyUpdates::EmergencyUpdates(const EmergencyUpdates &other)
    : generalInfo_(other.generalInfo_), activeCount_(other.activeCount_), 
      forcesArrivedCount_(other.forcesArrivedCount_), timeToEvents_(other.timeToEvents_), mut_() {}

void EmergencyUpdates::addGeneralUpdate(std::string key, std::string value) {
    std::cout << "Adding general info: " << key << "=" << value << std::endl;
    generalInfo_[key] = value;
}

void EmergencyUpdates::addEvent(int time, std::string eventName, std::string description) {
    std::cout << "Adding event: time=" << time << ", name=" << eventName << std::endl;//dbg
    timeToEvents_[time] = std::make_pair(eventName, description);
}

void EmergencyUpdates::incrementActive() {
    activeCount_++;
    std::cout << "Incremented active count to: " << activeCount_ << std::endl;
}

void EmergencyUpdates::incrementForcesArrived() {
    forcesArrivedCount_++;
    std::cout << "Incremented forces arrived count to: " << forcesArrivedCount_ << std::endl;
}

const std::map<std::string, std::string>& EmergencyUpdates::getGeneralUpdates() const {
    return generalInfo_;
}

const std::map<int, std::pair<std::string,std::string>>& EmergencyUpdates::getEvents() const {
    return timeToEvents_;
}

int EmergencyUpdates::getActiveCount() const {
    return activeCount_;
}

int EmergencyUpdates::getForcesArrivedCount() const {
    return forcesArrivedCount_;
}
