package bgu.spl.net.impl.stomp;

import java.util.List;

import bgu.spl.net.api.ClientStompFrame;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.User;

//SendFrame: Processes message sending to channels
public class SendFrame extends StompFrameImp implements ClientStompFrame<String> {
    
    public SendFrame(Command command) {
        super(command);
    }

    @Override
    public void process(int connectionId, Connections<String> connections, StompMessagingProtocol<String> protocol) {
        User user = connections.Data().findUserByConnectionId(connectionId);

        String destination = getHeaderValue("destination");

        System.out.println("the destination i got is: "+destination); // dbg
        
        destination = destination.startsWith("/") ? destination.substring(1) : destination; //added to fix / 
        String receiptId = getHeaderValue("receipt");

        //assume list is not changed during send
        List<Integer> subscribers = connections.Data().getSubscribers(destination); 

        //Error cases
        //1.destination is not exist
        if (subscribers == null) { 
            protocol.sendError(connectionId, "Destination is not exist", receiptId, "Destination: " + destination + ".\n This destination is not exist.");
        } 
        //2.user is not subscribed
        else if (!subscribers.contains(connectionId)) { 
            protocol.sendError(connectionId, "User is not subscribed", receiptId, "Channel: " + destination + ".\n User "+ user.getUsername() + " is not subscribed to this channel.");
        }

        //handle send
        else {
            String body = getBody();
            for (Integer conId : subscribers) {
                protocol.sendMessage(conId, destination, body);
            }

            if (receiptId != null)
                protocol.sendReceipt(connectionId, receiptId);
        }
    }

    @Override
    public boolean checkLegal() {
        return hasHeader("destination");
    }
  

  
}