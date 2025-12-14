package utility;
import java.io.File;

public class UploadSession {
    public String fileID;
    public String uploader;
    public String filename;
    public long expectedSize;
    public long receivedSize;
    public File tempFile;
    public int chunkSize;

    public UploadSession(String fileID, String uploader, String filename, long expectedSize, File t_file, int chunkSize) {
        this.fileID = fileID;
        this.uploader = uploader;
        this.filename = filename;
        this.expectedSize = expectedSize;
        this.receivedSize = 0L;
        this.tempFile = t_file;
        this.chunkSize = chunkSize;
    }
}
