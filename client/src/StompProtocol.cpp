#include "../include/StompProtocol.h"
#include "../include/StompFrame.h"
#include "../include/ConnectionHandler.h"
#include "../include/EmergencyChannel.h"
#include "../include/EmergencyUpdates.h"
#include "../include/event.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <sstream>
#include <fstream>
#include <map>
//StompProtocol: Handles STOMP message processing and state management

using namespace std;

//constructor
StompProtocol::StompProtocol() :
        username_(), hasConnected_(false), shouldTerminate_(false), channelToSubIdChannel_(), 
        receiptIdToCommand_(), nextSubId_(0), nextReceiptId_(0) {}

vector<string> StompProtocol::split(const string& s, char delimiter) {
   vector<string> output;
   string token;
   for (auto c : s) {
        if(c == delimiter)
        {
            output.push_back(token);
            token = ""; //reset token
        }
        else token += c;
    }
    output.push_back(token); 
    return output;
}

bool StompProtocol::shouldTerminate() {
    return shouldTerminate_;
}

bool StompProtocol::isConnected() {
    return hasConnected_;
}

ConnectionHandler* StompProtocol::loginProcess(string input) {

    vector<string> words = StompProtocol::split(input, ' ');
    string command = words[0];

    //not the right command
    if (command != "login") {
        {
            lock_guard<mutex> lock(screenMutex);
            cout << "You must log in first. Try again" << endl;
        }
        return nullptr;
    }
    //login command is 4 words 1.login 2.host:port 3.username 4.password
    //if not it illegal
    if (words.size() != 4) {
        {
            lock_guard<mutex> lock(screenMutex);
            cout << "Illegal arguments for login. Try again" << endl;
        }
        return nullptr;
    }

    string hostport = words[1];
    string username = words[2];
    string password = words[3];

    //dividing to host and port
    vector<string> hp = StompProtocol::split(hostport, ':');
    string host = hp[0];
    if (hp.size() != 2) {
        {
            lock_guard<mutex> lock(screenMutex);
            cout << "Illegal arguments for login. Try again" << endl;
        }
        return nullptr;
    }
    short port = stoi(hp[1]);

    // try connect 
    ConnectionHandler* connectionHandler = new ConnectionHandler(host, port);
    if (!(*connectionHandler).connect()) { // problem in connection
        {
            lock_guard<mutex> lock(screenMutex);
            cerr << "Could not connect to server"  << endl;
            cerr << "Cannot connect to " << host << ":" << port << endl;
        }
        delete connectionHandler;
        return nullptr;
    }
    
    // try login
    // send CONNECT frame
    username_ = username;
    StompFrame connectFrame = connect(username, password); //creates connect frame
    string toSend = connectFrame.toString();
    if (!(*connectionHandler).sendFrameAscii(toSend, '\0')) { //send the connect frame, if not succseeded
        {
            lock_guard<mutex> lock(screenMutex);
            cout << "Disconnected. Exiting...\n" << endl;
        }
        delete connectionHandler;
        return nullptr;
    }
    return connectionHandler;
}

//CONNECT frame
StompFrame StompProtocol::connect(string username, string password) {
    StompFrame frame = StompFrame("CONNECT");
    frame.addHeader("accept-version", "1.2");
    frame.addHeader("host", "stomp.cs.bgu.ac.il");
    frame.addHeader("login", username);
    frame.addHeader("passcode", password);
    return frame;
}

//all the other inputs that are not login
vector<string> StompProtocol::inputProcess(std::string input) {

    vector<StompFrame> framesToSend;

    //split input with " "
    vector<string> words = StompProtocol::split(input, ' ');

    //check the command
   const string command = words[0];
    if (command == "login") { 
        {
            lock_guard<mutex> lock(screenMutex);
            cout << "The client is already logged in, log out before trying again." << endl;
        }
        return vector<string>();
    }

    //input is join
    else if (command == "join") {
        if (channelToSubIdChannel_.count(words[1]) > 0)
        {
            {
                lock_guard<mutex> lock(screenMutex);
                cout << "You are already joined to this channel." << endl;
            }
            return vector<string>();
        }
        //if its not "join police"
        if (words.size() != 2) {
            {
                lock_guard<mutex> lock(screenMutex);
                cout << "Illegal arguments for join" << endl;
            }
            return vector<string>();
        }
        //send SUBSCRIBE frame
        framesToSend.push_back(subscribe(words[1]));
    }

    //input is exit
    else if (command == "exit") {
        if (channelToSubIdChannel_.count(words[1]) == 0) {
            {
                lock_guard<mutex> lock(screenMutex);
                cout << "You did not join to this game." << endl;
            }
            return vector<string>();
        }
        //if its not "exit police"
        if (words.size() != 2) {
            {
                lock_guard<mutex> lock(screenMutex);
                cout << "Illegal arguments for exit" << endl;
            }
            return vector<string>();
        }
        //send UNSUBSCRIBE frame
        framesToSend.push_back(unsubscribe(words[1])); 
    }

    //input is report
    else if (command == "report") {
        try {
            cout<<words[1]<<endl;
            string json = words[1];
            names_and_events nne = parseEventsFile(json);

            if (nne.events.size() == 0){cout<<"missed parsing"<<endl;} // dbg

            send(nne.channel_name, nne.events, framesToSend);
        } catch (exception e) {
            {
                lock_guard<mutex> lock(screenMutex);
                cout << "Illegal arguments for report" << endl;
            }
            return vector<string>();
        }
    }

    //input is summary
    else if (command == "summary") {
        if (words.size() != 4) {
            {
                lock_guard<mutex> lock(screenMutex);
                cout << "Illegal arguments for summary. Try again" << endl;
            }
            return vector<string>();
        }
        string channel = words[1];
        string user = words[2];
        string file = words[3];

        handleSummary(channel, user, file);
        return vector<string>();
    }

    //input logout
    else if (command == "logout") { 
        //send DISCONNECT frame
        framesToSend.push_back(disconnect());
    }
    
    //if input is non of the above
    else {
        {
            lock_guard<mutex> lock(screenMutex);
            cout << "Illegal command. Try again" << std::endl;
        }
        return vector<string>();
    }
    return StompProtocol::framesToStrings(framesToSend);
}


// SUBSCRIBE frame
StompFrame StompProtocol::subscribe(std::string channel) {
    string command = "SUBSCRIBE";
    StompFrame frame = StompFrame(command);
    frame.addHeader("destination","/" + channel);
    
    channelToSubIdChannel_[channel] = make_pair(nextSubId_, EmergencyChannel(channel));

    std::cout << "Subscribed to channel: " << channel << " with ID: " << nextSubId_ << std::endl;//dbg

    frame.addHeader("id", to_string(nextSubId_));
    nextSubId_++;

    string receipt = to_string(nextReceiptId_);
    receiptIdToCommand_[receipt] = make_pair(command, channel);
    frame.addHeader("receipt", receipt);
    nextReceiptId_++;

    return frame;
}

// UNSUBSCRIBE frame
StompFrame StompProtocol::unsubscribe(std::string channel) {
    string command = "UNSUBSCRIBE";
    StompFrame frame = StompFrame(command);

    int subId = get<0>(channelToSubIdChannel_[channel]);
    frame.addHeader("id", to_string(subId));

    channelToSubIdChannel_.erase(channel);

    string receipt = to_string(nextReceiptId_);
    receiptIdToCommand_[receipt] = make_pair(command, channel);
    frame.addHeader("receipt", receipt);
    nextReceiptId_++;

    return frame;
}

// DISCONNECT frame
StompFrame StompProtocol::disconnect() {
    string command = "DISCONNECT";
    StompFrame frame = StompFrame(command);

    string receipt = to_string(nextReceiptId_);
    receiptIdToCommand_[receipt] = make_pair(command, "");
    frame.addHeader("receipt", receipt);
    nextReceiptId_++;

    return frame;
}

//from report
//going through the events in the report
void StompProtocol::send(string channel, vector<Event> events, vector<StompFrame> &framesToSend) {
    cout<<"reached send()"<<endl;
    for (Event event : events) {
        StompFrame frame = eventToFrame(event);
        framesToSend.push_back(frame);
    }
}

StompFrame StompProtocol::eventToFrame(Event event) {
    // SEND frame
    StompFrame frame = StompFrame("SEND");
    frame.addHeader("destination", "/"+event.get_channel_name());

    std::string body;
    body = "user: "+ username_ + "\n";

    body += "city: " + event.get_city() + "\n";

    body += "event name: " + event.get_name()+ "\n";

    body += "date time: " + std::to_string(event.get_date_time())+ "\n";

    body += "general information:\n";

    for (const auto& info : event.get_general_information()) {
        body += info.first + ":" + info.second + "\n";
    }

    body += "description:\n";

    body += event.get_description();

    frame.setBody(body);

    cout << frame.toString()<< endl; //dbg

    return frame;
}

vector<string> StompProtocol::framesToStrings(vector<StompFrame> frames) {
    vector<string> output;
    for (StompFrame frame : frames) {
        output.push_back(frame.toString());
    }
    return output;
}

//generate summary
void StompProtocol::handleSummary(string channel, string user, string file) {
    if (channelToSubIdChannel_.count(channel) == 0) {
        {
            lock_guard<mutex> lock(screenMutex);
            cerr << "You are not subscribe to channel " << channel << endl;
        }
        return;
    }

    cout<<"summary generation started"<<endl; // dbg

    EmergencyChannel &cn = get<1>(channelToSubIdChannel_[channel]);

    if (!cn.hasUpdates(user)) { // check if we have saved updates
        std::cout << "Error: No updates found for user " << user << " in channel " << channel << std::endl;
        return;
    }

    EmergencyUpdates &updates = cn.getUserUpdates(user);

    //dbg
    std::cout << "Retrieved updates for channel: " << channel << ", user: " << user << std::endl;


    ofstream outfile(file);
    if (!outfile) {
        {
            lock_guard<mutex> lock(screenMutex);
            cerr << "Error opening file " << file << endl;
        }
        return;
    }

    lock_guard<mutex> lock(updates.mut_);

    outfile << "Channel " << channel << endl;
    outfile << "Stats:" << endl;
    
    // Count total reports
    const auto& events = updates.getEvents();
    int totalReports = events.size();
    
    std::cout << "Number of events found: " << events.size() << std::endl;//dbg


    outfile << "Total: " << totalReports << endl;
    outfile << "active: " << updates.getActiveCount() << endl;
    outfile << "forces arrival at scene: " << updates.getForcesArrivedCount() << endl << endl;

    outfile << "Event Reports:" << endl;
    int reportNum = 1;
    for (const auto& event : events) {
        outfile << "Report_" << reportNum++ << ":" << endl;
        const auto& genInfo = updates.getGeneralUpdates();

        std::cout << "Number of general info entries: " << genInfo.size() << std::endl;//dbg

        outfile << "city: " << genInfo.at("city") << endl;
        
        time_t rawtime = event.first;
        struct tm * timeinfo = localtime(&rawtime);
        char buffer[80];
        strftime(buffer, 80, "%d/%m/%y %H:%M", timeinfo);
        outfile << "date time: " << buffer << endl;
        
        outfile << "event name: " << event.second.first << endl;
        
        string desc = event.second.second;
        if (desc.length() > 27) {
            desc = desc.substr(0, 27) + "...";
        }
        outfile << "summary: " << desc << endl << endl;
    }

    outfile.close();
}

bool StompProtocol::incomeProcess(string framestr) {
    StompFrame frame = StompFrame::stringToFrame(framestr);

    string command = frame.getCommand();
    if (command == "CONNECTED") {
        hasConnect();
        return true;
    }
    else if (command == "ERROR") {
        handleError(frame);
        return true;
    }
    else if (command == "RECEIPT") {
        return handleReceipt(frame);
    }
    else if (command == "MESSAGE") {
        handleMessage(frame);
        return false;
    }
    return false;
}

void StompProtocol::hasConnect() {
    hasConnected_ = true;
    {
        lock_guard<mutex> lock(screenMutex);
        cout << "Login successful" << endl;
    }
}

void StompProtocol::handleError(StompFrame frame) {
    {
        lock_guard<mutex> lock(screenMutex);
        cout << "ERROR" << endl;
        cout << frame.getHeaderValue("message") << endl;
        if (frame.hasBody()) {
            cout << "The problem is:" << endl;
            cout << frame.getBody() << endl;
        }
        if (frame.hasHeader("receipt-id")) {
            string receiptId = frame.getHeaderValue("receipt-id");
            cout << "The ERROR is related to frame " << frame.getHeaderValue("receipt-id") << endl;
            string command = get<0>(receiptIdToCommand_[receiptId]);
            if (get<1>(receiptIdToCommand_[receiptId]) != "") {
                cout << "ERROR in : Tried to perform " << command << " with " << get<1>(receiptIdToCommand_[receiptId]) << endl;
            }
            else {
                cout << "ERROR in : Tried to perform " << command << endl;
            }
        }
    }
    terminate();
}

bool StompProtocol::handleReceipt(StompFrame frame) {
    string receiptId = frame.getHeaderValue("receipt-id");
    string command = get<0>(receiptIdToCommand_[receiptId]);
    bool output = false;
    if (command == "SUBSCRIBE") {
        lock_guard<mutex> lock(screenMutex);
        cout << "Joined channel " << get<1>(receiptIdToCommand_[receiptId]) << endl;
    }
    else if (command == "UNSUBSCRIBE") {
        lock_guard<mutex> lock(screenMutex);
        cout << "Exited channel " << get<1>(receiptIdToCommand_[receiptId]) << endl;
    }
    else if (command == "DISCONNECT") {
        {
            lock_guard<mutex> lock(screenMutex);
            cout << "Logout successful" << endl;
        }
        terminate();
        output = true;
    }
    receiptIdToCommand_.erase(receiptId);
    return output;
}

void StompProtocol::handleMessage(StompFrame frame) {
    string destination = frame.getHeaderValue("destination");

    // Remove the leading '/' if it exists
    if (destination[0] == '/') {
        destination = destination.substr(1);
    }

    std::cout << "Received message for channel: " << destination << std::endl;//dbg

    // Check if we're subscribed to this channel
    if (channelToSubIdChannel_.find(destination) == channelToSubIdChannel_.end()) {
        std::cout << "Warning: Received message for channel we're not subscribed to: " << destination << std::endl;
        return;
    }

    string body = frame.getBody();
    incomeMessageParse(destination, body);
}

void StompProtocol::incomeMessageParse(string destination, string body) {
vector<string> lines = split(body, '\n');
    
    // Parse header information
    string username = split(lines[0], ':')[1].substr(1);
    string city = split(lines[1], ':')[1].substr(1);
    string eventName = split(lines[2], ':')[1].substr(1);
    string strTime = split(lines[3], ':')[1].substr(1);
    int time = stoi(strTime);
    
    cout << "Debug: Processing event for " << username << " in " << city << endl;
    
    auto& channelData = channelToSubIdChannel_[destination];
    EmergencyChannel& channel = get<1>(channelData);
    EmergencyUpdates& updates = channel.getUserUpdates(username);
    
    lock_guard<mutex> lock(updates.mut_);
    updates.addGeneralUpdate("city", city);
    
    bool inGenInfo = false;
    bool inDesc = false;
    string description;
    
    for (size_t i = 4; i < lines.size(); i++) {
        string line = lines[i];
        
        // Trim any whitespace from the line
        while (!line.empty() && (line[0] == ' ' || line[0] == '\t')) {
            line = line.substr(1);
        }
        while (!line.empty() && (line[line.size()-1] == ' ' || line[line.size()-1] == '\t')) {
            line = line.substr(0, line.size()-1);
        }
        
        if (line == "general information:") {
            cout << "Debug: Found general information section" << endl;
            inGenInfo = true;
            inDesc = false;
            continue;
        }
        else if (line == "description:") {
            cout << "Debug: Found description section" << endl;
            inGenInfo = false;
            inDesc = true;
            continue;
        }
        
        if (inGenInfo && !line.empty()) {
            vector<string> kv = split(line, ':');
            if (kv.size() == 2) {
                string key = kv[0];
                string value = kv[1];
                
                // Trim any whitespace from key and value
                while (!key.empty() && (key[0] == ' ' || key[0] == '\t')) {
                    key = key.substr(1);
                }
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
                    value = value.substr(1);
                }
                
                cout << "Debug: Processing general info - Key: '" << key << "', Value: '" << value << "'" << endl;
                
                if (key == "active" && value == "true") {
                    cout << "Debug: Found active=true, incrementing counter" << endl;
                    updates.incrementActive();
                }
                else if (key == "forces_arrival_at_scene" && value == "true") {
                    cout << "Debug: Found forces_arrival=true, incrementing counter" << endl;
                    updates.incrementForcesArrived();
                }
            }
        }
        else if (inDesc) {
            description += line;
            if (i < lines.size() - 1) {
                description += "\n";
            }
        }
    }
    
    updates.addEvent(time, eventName, description);
    
    cout << "Debug: Finished processing event. Current counts - "
         << "Active: " << updates.getActiveCount() 
         << ", Forces arrived: " << updates.getForcesArrivedCount() << endl;
}

void StompProtocol::terminate() {
    shouldTerminate_ = true;
}
