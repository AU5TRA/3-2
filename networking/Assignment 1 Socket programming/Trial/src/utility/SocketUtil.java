package utility;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class SocketUtil {
    public Socket socket;
    public ObjectInputStream input;
    public ObjectOutputStream output;

    public SocketUtil(String serverAddr, int port) throws IOException {
        this.socket = new Socket(serverAddr, port);
        this.output = new ObjectOutputStream(this.socket.getOutputStream());
        this.output.flush();  
        this.input = new ObjectInputStream(this.socket.getInputStream());
        
    }

    public SocketUtil(Socket sckt) throws IOException {
        this.socket = sckt;
        this.output = new ObjectOutputStream(sckt.getOutputStream());
        this.output.flush();
        this.input = new ObjectInputStream(sckt.getInputStream());

    }

    public void close_connection() throws IOException {
        if (input != null) input.close();
        if (output != null) output.close();
        if (socket != null) socket.close();
    }

    public void setTimeout(int timeout) throws IOException {
        socket.setSoTimeout(timeout);
    }

    public Object read() throws IOException, ClassNotFoundException {
        return this.input.readUnshared();
    }

    public int read(byte[] buffer, int offset, int len) throws IOException {
        return this.input.read(buffer, offset, len);
    }
    public void write(Object obj) throws IOException {
        output.writeUnshared(obj);
        flush();
    }

    public void write(byte[] buffer, int offset, int len) throws IOException {
        this.output.write(buffer, offset, len);
        flush();
    }

    public int available() throws IOException {
        return socket.getInputStream().available();
    }

    public void flush() throws IOException {
        this.output.flush();
    }
}



















//        buf - the buffer into which the data is read
//        off - the start offset in the destination array buf
//        len - the maximum number of bytes read
//        https://docs.oracle.com/en/java/javase/17/docs/api/java.base/java/io/ObjectInputStream.html#read(byte%5B%5D,int,int)