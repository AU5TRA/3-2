package utility;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class socketUtil {
    private Socket socket;
    private ObjectInputStream input;
    private ObjectOutputStream output;

    public socketUtil(String serverAddr, int port) throws IOException {
        this.socket = new Socket(serverAddr, port);
        input = new ObjectInputStream(socket.getInputStream());
        output = new ObjectOutputStream(socket.getOutputStream());
    }

    public socketUtil(Socket socket) throws IOException {
        this.socket = socket;
        input = new ObjectInputStream(socket.getInputStream());
        output = new ObjectOutputStream(socket.getOutputStream());
    }

    public void close_connection() throws IOException {
        input.close();
        output.close();
    }

    public void setTimeout(int timeout) throws IOException {
        socket.setSoTimeout(timeout);
    }

    public int read(byte[] buffer, int offset, int len) throws IOException {
        return output.read(buffer, offset, len);
    }

    public void write(byte[] buffer, int offset, int len) throws IOException {
        output.write(buffer, offet, len);
        flush();
    }

    public int available() throws IOException {
        return socket.getInputStream().available();
    }

    public void flush() throws IOException {
        output.flush();
    }
}



















//        buf - the buffer into which the data is read
//        off - the start offset in the destination array buf
//        len - the maximum number of bytes read
//        https://docs.oracle.com/en/java/javase/17/docs/api/java.base/java/io/ObjectInputStream.html#read(byte%5B%5D,int,int)