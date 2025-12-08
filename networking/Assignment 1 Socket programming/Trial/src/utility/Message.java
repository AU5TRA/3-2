package utility;

public class Message {
    public String sender;
    public String receiver;
    public String msg;
    public boolean isRequest;
    public boolean seen_status;

    public Message(String sender, String receiver, boolean is_request, String msg) {
        this.sender = sender;
        this.receiver = receiver;
        this.msg = msg;
        this.isRequest = is_request;
        this.seen_status = false;
    }
}