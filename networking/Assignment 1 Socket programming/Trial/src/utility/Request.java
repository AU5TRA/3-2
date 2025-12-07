package utility;
import java.io.Serializable;

public class Request implements Serializable{
    public static final String LIST_ALL_CLIENTS = "LIST_ALL_CLIENTS";
    public static final String LIST_ONLINE_CLIENTS = "LIST_ONLINE_CLIENTS";
    public static final String LIST_UPLOADED_FILES = "LIST_UPLOADED_FILES";
    public static final String DOWNLOAD_FILE = "DOWNLOAD_FILE";
    public static final String UPLOAD_FILE = "UPLOAD_FILE";
    public static final String LIST_PUBLIC_FILES = "LIST_PUBLIC_FILES";
    public static final String DOWNLOAD_PUBLIC_FILE = "DOWNLOAD_PUBLIC_FILE";
    public static final String MAKE_FILE_REQUEST = "MAKE_FILE_REQUEST";
    public static final String VIEW_UNREAD_MESSAGES = "VIEW_UNREAD_MESSAGES";
    public static final String VIEW_HISTORY = "VIEW_HISTORY";
    public static final String LOG_OUT = "LOG_OUT";

    public String request;
    public Request(String req) {
        this.request = req;
    }
}