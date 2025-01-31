#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <ctime>

// #include "../include/keyboardInput.h"

using namespace std;
using json = nlohmann::json;

//added this method so we can remove keyboardInput.h
void split_str(const std::string& str, char delimiter, std::vector<std::string>& args) {
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        std::string token = str.substr(start, end - start);
        while (!token.empty() && std::isspace(token[0])) {
            token = token.substr(1);
        }
        while (!token.empty() && std::isspace(token[token.length()-1])) {
            token = token.substr(0, token.length()-1);
        }
        if (!token.empty()) {
            args.push_back(token);
        }
        start = end + 1;
        end = str.find(delimiter, start);
    }
    if (start < str.length()) {
        std::string token = str.substr(start);
        while (!token.empty() && std::isspace(token[0])) {
            token = token.substr(1);
        }
        while (!token.empty() && std::isspace(token[token.length()-1])) {
            token = token.substr(0, token.length()-1);
        }
        if (!token.empty()) {
            args.push_back(token);
        }
    }
}


Event::Event(std::string channel_name, std::string city, std::string name, int date_time,
             std::string description, std::map<std::string, std::string> general_information)
    : channel_name(channel_name), city(city), name(name),
      date_time(date_time), description(description), general_information(general_information), eventOwnerUser("")
{
}

Event::~Event()
{
}

void Event::setEventOwnerUser(std::string setEventOwnerUser) {
    eventOwnerUser = setEventOwnerUser;
}

const std::string &Event::getEventOwnerUser() const {
    return eventOwnerUser;
}

const std::string &Event::get_channel_name() const
{
    return this->channel_name;
}

const std::string &Event::get_city() const
{
    return this->city;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_date_time() const
{
    return this->date_time;
}

const std::map<std::string, std::string> &Event::get_general_information() const
{
    return this->general_information;
}

const std::string &Event::get_description() const
{
    return this->description;
}

int convertDateTimeToSeconds(const std::string& dateStr) {
    struct tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%d/%m/%y %H:%M");
    return mktime(&tm);
}

Event::Event(const std::string &frame_body): channel_name(""), city(""), 
                                             name(""), date_time(0), description(""), general_information(),
                                             eventOwnerUser("")
{
    stringstream ss(frame_body);
    string line;
    string eventDescription;
    map<string, string> general_information_from_string;
    bool inGeneralInformation = false;
    while(getline(ss,line,'\n')){
        vector<string> lineArgs;
        if(line.find(':') != string::npos) {
            split_str(line, ':', lineArgs);
            string key = lineArgs.at(0);
            string val;
            if(lineArgs.size() == 2) {
                val = lineArgs.at(1);
            }
            if(key == "user") {
                eventOwnerUser = val;
            }
            if(key == "channel name") {
                channel_name = val;
            }
            if(key == "city") {
                city = val;
            }
            else if(key == "event name") {
                name = val;
            }
            else if(key == "date time") {
                date_time = convertDateTimeToSeconds(val);
            }
            else if(key == "general information") {
                inGeneralInformation = true;
                continue;
            }
            else if(key == "description") {
                while(getline(ss,line,'\n')) {
                    eventDescription += line + "\n";
                }
                description = eventDescription;
            }

            if(inGeneralInformation) {
                general_information_from_string[key.substr(1)] = val;
            }
        }
    }
    general_information = general_information_from_string;
}


names_and_events parseEventsFile(std::string json_path){

    cout<<"i am parsing now" <<endl;                // dbg
    cout<<"the file path is:" + json_path << endl;  // dbg
    
    std::ifstream f(json_path);
    json data = json::parse(f);
    
    if(!data.is_null()){cout<<"report file isnt null"<<endl;} //dbg

    std::string channel_name = data["channel_name"];
    
    cout<<channel_name<<endl; //dbg

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"]){
        std::string name = event["event_name"];

        cout<<name<<endl; //dbg

        std::string city = event["city"];

        cout<<city<<endl; //dbg

        int date_time = event["date_time"];

        cout<< date_time <<endl; //dbg

        std::string description = event["description"];

        cout<<description<<endl; //dbg

        std::map<std::string, std::string> general_information;
        for (auto &update : event["general_information"].items())
        {
            if (update.value().is_string())
                general_information[update.key()] = update.value();
            else
                general_information[update.key()] = update.value().dump();
        }

        events.push_back(Event(channel_name, city, name, date_time, description, general_information));
        cout<<"successfully parsed event" + city + name <<endl;
    }
    names_and_events events_and_names{channel_name, events};

    return events_and_names;
}