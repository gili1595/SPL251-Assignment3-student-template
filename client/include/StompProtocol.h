#pragma once
#include "../include/EmergencyChannel.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <atomic>
#include <mutex>
#include "StompFrame.h"
#include "event.h"
using std::string;
using std::vector;
extern std::mutex screenMutex;

class Event;
class StompFrame;
class ConnectionHandler;

//StompProtocol: Handles STOMP message processing and state management
class StompProtocol{
private:

    std::string username_;
    std::atomic<bool> hasConnected_;
    std::atomic<bool> shouldTerminate_;

    //Maps channel name to a pair of (subscription ID, EmergencyChannel)
    std::map<std::string, std::pair<int, EmergencyChannel>> channelToSubIdChannel_;

    // Maps receipt IDs to pairs of (command, channel name)
    std::map<std::string, std::pair<std::string, std::string>> receiptIdToCommand_;

    int nextSubId_;
    int nextReceiptId_;

    //helper methods for creating STOMP frames
    StompFrame connect(std::string username, std::string password);
    StompFrame subscribe(std::string channel);
    StompFrame unsubscribe(std::string channel);
    StompFrame disconnect();
    StompFrame eventToFrame(Event event);

    //methods for handling different message types
    void hasConnect();
    void handleError(StompFrame frame);
    bool handleReceipt(StompFrame frame);
    void handleMessage(StompFrame frame);
    void handleSummary(std::string channelName, std::string user, std::string file);

    //message parsing and processing
    void incomeMessageParse(std::string channelName, std::string body);
    void send(std::string channel_name, std::vector<Event> events, std::vector<StompFrame> &framesToSend);

    static std::vector<std::string> framesToStrings(std::vector<StompFrame> frames);

    
public:

    StompProtocol();   //Constructor

    //Static helper method for splitting
    static std::vector<std::string> split(const std::string &s, char delimeter);

    // Main protocol methods
    bool shouldTerminate();
    bool isConnected();
    void terminate();

    // Processing methods for different types of input
    ConnectionHandler* loginProcess(std::string input);
    std::vector<std::string> inputProcess(std::string input);
    bool incomeProcess(std::string framestr); //return true if the frame is answer on login\logout or error message
  
};