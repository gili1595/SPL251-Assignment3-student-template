package bgu.spl.net.api;

import bgu.spl.net.impl.stomp.ConnectFrame;
import bgu.spl.net.impl.stomp.DisconnectFrame;
import bgu.spl.net.impl.stomp.SendFrame;
import bgu.spl.net.impl.stomp.SubscribeFrame;
import bgu.spl.net.impl.stomp.UnsubscribeFrame;

public interface StompFrame {
    enum Command {
        CONNECTED,
        MESSAGE,
        RECEIPT,
        ERROR,
        CONNECT,
        SEND,
        SUBSCRIBE,
        UNSUBSCRIBE,
        DISCONNECT
    }

    Command getCommand();
    boolean hasHeader(String key);
    String getHeaderValue(String key);
    void addHeader(String key, String value);
    boolean hasBody();
    String getBody();
    void setBody(String body);
    String toString();

    //parse the messege and creates a new frame according to the command
    static ClientStompFrame<String> stringToClientFrame(String msg) {
        String[] lines = msg.split("\n");
        Command command = Command.valueOf(lines[0]);
        ClientStompFrame<String> output;
        switch (command) {
            case CONNECT:
                output = new ConnectFrame(command);
                break;
            case SEND:
                output = new SendFrame(command);
                break;
            case SUBSCRIBE:
                output = new SubscribeFrame(command);
                break;
            case UNSUBSCRIBE:
                output = new UnsubscribeFrame(command);
                break;
            case DISCONNECT:
                output = new DisconnectFrame(command);
                break;
            default:
                return null;
        }
        //parsing to get the key and value to create the header
        int i = 1;
        while (i < lines.length && !lines[i].equals("")) {
            String[] keyValue = lines[i].split(":");
            String key = keyValue[0];
            String Value = keyValue[1];
            output.addHeader(key, Value);
            i++;
        }
        i++;
        //buidling the body
        String body = "";
        while (i < lines.length) {
            body += lines[i];
            if (i < lines.length - 1)
                body += "\n";
            i++;
        }
        if (!body.equals("")) {
            output.setBody(body);
        }
        return output;
    }
}