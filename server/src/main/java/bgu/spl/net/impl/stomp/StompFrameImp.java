package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.api.StompFrame;

public class StompFrameImp implements StompFrame {
    
    //frames have a command the headers and a body
    private Command command;
    private Map<String , String> headers;
    private String body;

    public StompFrameImp(Command command) {
        this.command = command;
        this.headers = new HashMap<>();
        this.body = "";
    }
    
    @Override
    public Command getCommand() {
        return this.command;
    }

    @Override
    public boolean hasHeader(String key) {
        return headers.containsKey(key);
    }

    //return value of header's key or null if key doesnt exits
    @Override
    public String getHeaderValue(String key) {
        return headers.get(key);
    }

    @Override
    public void addHeader(String key, String value) {
        headers.put(key, value);
    }

    @Override
    //making sure body isnt empy
    public boolean hasBody() {
        return !this.body.equals("");
    }

    @Override
    public String getBody() {
        return this.body;
    }

    @Override
    public void setBody(String body) {
        this.body = body;
    }

    @Override
    //prints the frame
    public String toString() {
        String output = "";
        output += command.toString() + "\n";
        for (String key : headers.keySet()) {
            output = output + key + ":" + headers.get(key) + "\n";
        }
        output += "\n";
        if (hasBody())
            output += getBody() + "\n";
            
        return output;
    }
}