package server;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.Date;
import utility.Request;
import utility.SocketUtil;

public class Worker extends Thread {
    // Socket socket;
    private Thread thread;
    private SocketUtil socket;
    private String client_name;
    private Server server;


    public Worker(SocketUtil socket, String name, Server server)
    {
        this.socket = socket;
        this.client_name = name;
        this.server = server;
        this.thread = new Thread(this);
        this.thread.start();
    }

    public void run()
    {
        while(true){
            try{
            Object obj = socket.read();
            System.out.println("Received object from " + client_name);
            System.out.println(obj.getClass().getName());

            if (obj instanceof Request){
                System.out.println("Processing request from " + client_name);
                if(((Request)obj).request.equals(((Request)obj).LIST_ALL_CLIENTS)){
                    System.out.println("Show all clients request from " + client_name);
                    socket.write(server.get_clients("registered"));
                }
                else if(((Request)obj).request.equals(((Request)obj).LIST_ONLINE_CLIENTS)){
                    System.out.println("Show online clients request from " + client_name);
                    socket.write(server.get_clients("online"));
                }
            }
            } catch (IOException e) {
                System.out.println("Connection with " + client_name + " lost.");
                try {
                    socket.write(server.close_client_connection(client_name));
                    socket.close_connection();
                } catch (IOException ex) {
                    System.out.println("Error closing connection for " + client_name);
                }
                break;
            } catch (ClassNotFoundException e) {
                System.out.println("Received unknown object from " + client_name);
            }
        }
    }
}
