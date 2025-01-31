package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.ClientStompFrame;
import bgu.spl.net.api.StompFrame;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImp implements StompMessagingProtocol<String> {

    private int connectionId;
    private Connections<String> connections;
    private volatile boolean shouldTerminate;
    
    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
        this.shouldTerminate = false;        
    }

    @Override
    public void process(String message) {

        ClientStompFrame<String> frame = StompFrame.stringToClientFrame(message);

        //if the frame is null we want to send an error frame
        if (frame == null) {
            sendError(connectionId, "Command is illegal", null, null);
            return;
        }
        //if the frame isnt legal also send error frame
        if (!frame.checkLegal()) {
            sendError(connectionId, "Headers of frame are illegal", null, null);
            return;
        }
        frame.process(connectionId, connections, this); 
    }

    @Override
    public boolean shouldTerminate() {
        return this.shouldTerminate;
    }

    @Override
    public void terminate() {
        this.shouldTerminate = true;

        connections.disconnect(connectionId);
    }

    //send CONNECTED frame
    @Override
    public void sendConnect(int connectionId) {

        StompFrame connectedFrame = new StompFrameImp(StompFrame.Command.CONNECTED);
        connectedFrame.addHeader("version", "1.2");

        connections.send(connectionId, connectedFrame.toString());
    }

    //send ERROR frame
    @Override
    public void sendError(int connectionId, String message, String receiptId, String body) {

        StompFrame errorFrame = new StompFrameImp(StompFrame.Command.ERROR);
        errorFrame.addHeader("message", message);
        if (receiptId != null) {
            errorFrame.addHeader("receipt-id", receiptId);
        }
        if (body != null)
            errorFrame.setBody(body);

        connections.send(connectionId, errorFrame.toString());
        terminate();
    }

    //send MESSAGE frame
    @Override
    public void sendMessage(int connectionId, String destination, String body) {

        StompFrame messageFrame = new StompFrameImp(StompFrame.Command.MESSAGE);

        messageFrame.addHeader("destination", "/" + destination);  //added '/' to return with it to the client
 
        Integer subId = connections.Data().getSubscriptionId(connectionId, destination);
        //should not enter here
        if (subId == null) {
            System.out.println("ERROR");
            return;
        }
        //bulding 
        messageFrame.addHeader("subscription", Integer.toString(subId));
        messageFrame.addHeader("message-id", Integer.toString(connections.Data().getNextMessageId()));
        messageFrame.setBody(body);

        connections.send(connectionId, messageFrame.toString());
    }

    //send RECEIPT frame
    @Override
    public void sendReceipt(int connectionId, String receiptId) {
        
        StompFrame receiptFrame = new StompFrameImp(StompFrame.Command.RECEIPT);
        receiptFrame.addHeader("receipt-id", receiptId);
        connections.send(connectionId, receiptFrame.toString());
    }
}