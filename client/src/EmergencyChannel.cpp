#include "../include/EmergencyChannel.h"
#include <iostream>
//EmergencyChannel: Manages channel-specific updates per user

EmergencyChannel::EmergencyChannel() : channelName_(), usernameToUpdates_() {}

EmergencyChannel::EmergencyChannel(std::string channelName) 
    : channelName_(channelName), usernameToUpdates_() {}

const std::string EmergencyChannel::getChannelName() const {
    return channelName_;
}

EmergencyUpdates &EmergencyChannel::getUserUpdates(std::string username) {
    // If updates don't exist for this user, create an entry
        if (usernameToUpdates_.find(username) == usernameToUpdates_.end()) {
            std::cout << "Creating new updates for user: " << username << std::endl;
        }
    return usernameToUpdates_[username];
}

bool EmergencyChannel::hasUpdates(const std::string& username) const {
    return usernameToUpdates_.find(username) != usernameToUpdates_.end();
}

size_t EmergencyChannel::getUserCount() const {
    return usernameToUpdates_.size();
}