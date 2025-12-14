package utility;

public class FileRequest extends Request {
    public String requester;
    public String requestID;
    public String description;
    public String recipient; 

    public FileRequest(String requester, String description, String recipient) {
        super(Request.FILE_REQUEST);
        this.requester = requester;
        this.description = description;
        this.recipient = recipient;
    }
}