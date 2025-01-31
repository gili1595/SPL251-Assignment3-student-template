package bgu.spl.net.srv;

import java.io.IOException;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

//ConnectionsImpl: Manages active client connections and message routing
public class ConnectionsImpl<T> implements Connections<T> {

    private srvData<T> data;
    private Map<Integer, ConnectionHandler<T>> idToHandler; //connecting between the id and the handker

    //constructor
    public ConnectionsImpl() {
        this.data = new srvData<>();
        this.idToHandler = new ConcurrentHashMap<>(); 
    }

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = idToHandler.get(connectionId);
        if (handler == null) 
            return false;
        try {
            handler.send(msg);
            return true;
        } catch (IOException e) {
            disconnect(connectionId);
            try {
                handler.close();
            } catch (IOException ex) {}
            return false;
        }
    }

    @Override
    public void connect(ConnectionHandler<T> handler, int connectionId) {
        //add to the map and start the handler with the connection id
        idToHandler.put(connectionId, handler);
        handler.start(connectionId, this);
    }

    @Override
    public void disconnect(int connectionId) {
        idToHandler.remove(connectionId);

        User userDisconnected = data.findUserByConnectionId(connectionId); 
        // mabey client didnt login yet 
        if (userDisconnected != null) { 
            Set<String> channels = userDisconnected.getChannels();
            for (String channel : channels) {
                Data().removeSubscriber(connectionId, channel);
            }

            //update through User
            userDisconnected.disconnect();
            //update in data srv
            data.removeUser(connectionId);
        }
    }

    @Override
    public srvData<T> Data() {
        return this.data;
    } 
}