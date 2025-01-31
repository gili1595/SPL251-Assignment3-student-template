package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.ClientStompFrame;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.User;

//ConnectFrame: Handles login and authentication
public class ConnectFrame extends StompFrameImp implements ClientStompFrame<String> {

    public ConnectFrame(Command command) {
        super(command);
    }

    @Override
    public void process(int connectionId, Connections<String> connections, StompMessagingProtocol<String> protocol) {

        //getting the info
        String username = getHeaderValue("login");
        String password = getHeaderValue("passcode");
        String receiptId = getHeaderValue("receipt");
        boolean succeedConnect = false;

        //thread safetyh during the info of connections data in srv data
        synchronized (connections.Data()) {
            User user = connections.Data().findUserbyUsername(username);
            //if its a new user
            if (user == null) { 
                User newUser = new User(username, password, connectionId);
                connections.Data().addNewUser(newUser);
                succeedConnect = true;
            }
            //user is not new
            else {
                //wrong password
                if (!user.getPassword().equals(password)) { 
                    protocol.sendError(connectionId, "Wrong password", receiptId, null);
                }
                else {
                    //user already logged in
                    if (user.getIsConnect()) { 
                        protocol.sendError(connectionId, "User already logged in", receiptId, null);
                    }
                    //can connect
                    else { 
                        connections.Data().connectOldUser(connectionId, user);;
                        succeedConnect = true;
                    }
                }
            }
        }
        //if th process was secessfull send connect
        if (succeedConnect) {
            protocol.sendConnect(connectionId);

            if (receiptId != null)
                protocol.sendReceipt(connectionId, receiptId);
        }
    }

    @Override
    public boolean checkLegal() {
        return hasHeader("accept-version") && getHeaderValue("accept-version").equals("1.2") && hasHeader("host") && hasHeader("login")
                && hasHeader("passcode") && !hasBody();
    }
    
}