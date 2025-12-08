package utility;

public class FileRequest extends Request {
    public String requester;
    public String requestID;
    public String description;

    public FileRequest(String requester, String description) {
        super(Request.FILE_REQUEST);
        this.requester = requester;
        this.description = description;
    }
}