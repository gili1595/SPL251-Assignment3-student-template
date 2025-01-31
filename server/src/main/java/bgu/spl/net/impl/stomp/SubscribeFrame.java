package bgu.spl.net.impl.stomp;

import java.util.List;

import bgu.spl.net.api.ClientStompFrame;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

//SubscribeFrame: Manages channel subscriptions
public class SubscribeFrame extends StompFrameImp implements ClientStompFrame<String> {

    public SubscribeFrame(Command command) {
        super(command);
    }

    @Override
    public void process(int connectionId, Connections<String> connections, StompMessagingProtocol<String> protocol) {
        String destination = getHeaderValue("destination");
        destination = destination.startsWith("/") ? destination.substring(1) : destination; //added to fix '/'
        int subscriptionId = Integer.parseInt(getHeaderValue("id"));

        boolean succeedSubscribe = false;

        List<Integer> subscribers = connections.Data().getSubscribers(destination);
        
        //thread safety when looking in connections to make sure no changes in the process
        synchronized (connections.Data()) {
            //open new channel
            if (subscribers == null) {
                connections.Data().addNewChannel(destination, connectionId, subscriptionId);
                succeedSubscribe = true;
            }
        }
        
        //add subscriber to exist channel
        if (!succeedSubscribe) { 
            if (!subscribers.contains(connectionId)) {
                connections.Data().addSubscriber(destination, connectionId, subscriptionId);
            }
        }

        String receiptId = getHeaderValue("receipt");
        if (receiptId != null)
            protocol.sendReceipt(connectionId, receiptId);
    }

    @Override
    public boolean checkLegal() {
        return hasHeader("destination") && hasHeader("id") && 
            ClientStompFrame.parseIntOrNull(getHeaderValue("id")) != null && !hasBody();
    }   
}