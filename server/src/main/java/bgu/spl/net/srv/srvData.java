package bgu.spl.net.srv;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

//srvData: Handles server state (users, channels, subscriptions)
public class srvData<T> {

    private Map<Integer, User> idToUser;        // id of the user
    private Map<String, User> usernameToUser;   //username to user
    private Map<String, List<Integer>> channelToSubscribers; // subscribers by connectionId
    private AtomicInteger nextMessageId;

    public srvData() {

        this.idToUser = new ConcurrentHashMap<>();
        this.usernameToUser = new ConcurrentHashMap<>();
        this.channelToSubscribers = new ConcurrentHashMap<>();
        nextMessageId = new AtomicInteger();
    }

    //return user of username or null
    public User findUserbyUsername(String username) {
        return usernameToUser.get(username);
    }

    //return user of connectionId or null
    public User findUserByConnectionId(int connectionId) {
        return idToUser.get(connectionId);
    }

    //add user to both maps
    public void addNewUser(User newUser) {
        idToUser.put(newUser.getConnectionId(), newUser);
        usernameToUser.put(newUser.getUsername(), newUser);
    }

    //connecting an old user
    public void connectOldUser(int connectionId, User oldUser) {
        idToUser.put(connectionId, oldUser);
        oldUser.newConnect(connectionId);
    }

    //return list of subscribers of channel or null if there is no such channel 
    public List<Integer> getSubscribers(String channel) {
        channel = channel.startsWith("/") ? channel.substring(1) : channel;
        return channelToSubscribers.get(channel);
    }


    //return subscriptionId of connectionId user to channel or null
    public Integer getSubscriptionId(int connectionId, String channel) {
        return idToUser.get(connectionId).getSubscriptionId(channel);
    }

    //generate and return the next messageId 
    public int getNextMessageId() {
        return nextMessageId.getAndIncrement();
    }
    
    //open new channel
    public void addNewChannel(String channel, int connectionId, int subscriptionId) {
        //add the new channel
        channelToSubscribers.put(channel, new LinkedList<>());
        //add the subscriber to the new channel
        addSubscriber(channel, connectionId, subscriptionId);
    }


    public void addSubscriber(String channel, int connectionId, int subscriptionId){
        List<Integer> subscribers = channelToSubscribers.get(channel);//get the subscribers of the cannel
        //threa safety when adding the new connction id 
        synchronized (subscribers) {
            subscribers.add(connectionId);
        }
        idToUser.get(connectionId).addSubscription(channel, subscriptionId);
    }


    public void removeSubscriber(int connectionId, String channel) {
        Integer id = connectionId;
        List<Integer> subscribers = channelToSubscribers.get(channel);
        //thread safty when making changes in subscribers
        synchronized (subscribers) {
            subscribers.remove(id);
        }
    }

    //user discconected
    public void removeUser(int connectionId) {
        idToUser.remove(connectionId);
    }

}