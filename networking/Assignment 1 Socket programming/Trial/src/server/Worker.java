package server;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.Date;
import utility.*;

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
                if(((Request)obj).request.equals(Request.LIST_ALL_CLIENTS)){
                    System.out.println("Show all clients request from " + client_name);
                    socket.write(server.get_clients("registered"));
                }
                else if(((Request)obj).request.equals(Request.LIST_ONLINE_CLIENTS)){
                    System.out.println("Show online clients request from " + client_name);
                    socket.write(server.get_clients("online"));
                }
                else if(((Request)obj).request.equals(Request.LIST_UPLOADED_FILES)){
                    System.out.println("Show uploaded files request from " + client_name);
                    socket.write(server.list_uploaded_files(client_name));
                }
                else if(((Request)obj).request.equals(Request.LIST_PUBLIC_FILES)){
                    System.out.println("Show public files request from " + client_name);
                    socket.write(server.list_public_files());
                }
                else if(((Request)obj).request.equals(Request.DOWNLOAD_FILE)){
                    System.out.println("Download file request from " + client_name);
                    int fileNumber = (int) ((Request)obj).getData();
                    server.prepare_file_download(client_name, socket, fileNumber);
                    // socket.write(response);
                    }
                else if (((Request)obj).request.equals(Request.FILE_REQUEST)){
                    FileRequest file_request = (FileRequest) obj;
                    file_request.requestID = server.generate_request_ID();
                    System.out.println(client_name + " requested a file, request ID: " + file_request.requestID);
                    server.make_file_request(file_request);
                    server.request_to_all_users(file_request);
                }
                else if(((Request)obj).request.equals(Request.LOG_OUT)){
                    System.out.println("Log out request from " + client_name);
                    socket.write(server.close_client_connection(client_name));
                    socket.close_connection();
                    break;
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
