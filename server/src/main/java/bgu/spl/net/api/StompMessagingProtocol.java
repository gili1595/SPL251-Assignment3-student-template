package bgu.spl.net.api;

public interface StompMessagingProtocol<T> extends MessagingProtocol<T> {
    //this was the implemtnt:
    // void start(int connectionId, Connections<T> connections);
    // void process(T message);
    // boolean shouldTerminate();

    //each frame is its own frame
    void sendError(int connectionId, String massage, String receiptId, String body);
    void sendConnect(int connectionId);
    void sendMessage(int connectionId, String destination, String body);
    void sendReceipt(int connectionId, String receiptId);
    void terminate();
}