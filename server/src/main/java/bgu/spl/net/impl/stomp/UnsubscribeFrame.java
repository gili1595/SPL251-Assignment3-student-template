package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.ClientStompFrame;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.User;

//UnsubscribeFrame: Handles leaving channel
public class UnsubscribeFrame extends StompFrameImp implements ClientStompFrame<String> {

    public UnsubscribeFrame(Command command) {
        super(command);
    }

    @Override
    public void process(int connectionId, Connections<String> connections, StompMessagingProtocol<String> protocol) {
        int subscriptionId = Integer.parseInt(getHeaderValue("id"));   //return value of header id than parse to int 
        String receiptId = getHeaderValue("receipt");                  //return value of header through ket receipt

        User user = connections.Data().findUserByConnectionId(connectionId);
        //not sepuse to enter here
        if (user == null) { 
            protocol.sendError(connectionId, "User is not connected", receiptId, null);
            return;
        }

        String channel = user.unsubscribe(subscriptionId);
        //if the channel is null will send error frame
        if (channel == null) {
            protocol.sendError(connectionId, "User have no such subscription", receiptId, 
                                "Subscription id " + subscriptionId + ".\n User have no such subscription id.");
            return;
        }

        //updating fields
        connections.Data().removeSubscriber(connectionId, channel);
        
        if (receiptId != null)
            protocol.sendReceipt(connectionId, receiptId);
    }

    @Override
    //makeing sure the farme is ok
    public boolean checkLegal() {
        return hasHeader("id") && ClientStompFrame.parseIntOrNull(getHeaderValue("id")) != null && !hasBody();
    }
}