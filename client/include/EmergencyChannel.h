#pragma once
#include "../include/EmergencyUpdates.h"
#include <string>
#include <map>

//EmergencyChannel: Manages channel-specific updates per user
class EmergencyChannel final {
private:
    std::string channelName_; // The emergency channel name (like "fire_dept")
    std::map<std::string, EmergencyUpdates> usernameToUpdates_; // Maps usernames to their emergency updates

public:
    EmergencyChannel(); //default constructor
    EmergencyChannel(std::string channelName); //constructor
    //gettrs
    const std::string getChannelName() const;
    EmergencyUpdates &getUserUpdates(std::string username);
    bool hasUpdates(const std::string& username) const;
    size_t getUserCount() const;
};