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
}
