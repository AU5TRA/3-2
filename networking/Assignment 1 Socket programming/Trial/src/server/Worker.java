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


    public Worker(SocketUtil socket, Server server)
    {
        this.socket = socket;
        this.server = server;
        this.thread = new Thread(this);
        this.thread.start();
    }

    public void run()
    {
        try {
            this.client_name = (String) socket.read();
            // instead of using several functions
            // if(server.isClientOnline(client_name)){
            //     System.out.println("Client " + client_name + " is already logged in!");
            //     socket.write("ERROR: Already logged in from another session");
            //     socket.close_connection();
            //     return; 
            // }
            // ;
            // server.addOnlineClient(client_name, socket);
            
            // if(server.isRegisteredClient(client_name)){
            //     socket.write("Welcome back, " + client_name);
            //     System.out.println(client_name + " has reconnected.");
            // }
            // else{
            //     server.registerNewClient(client_name);
            //     socket.write("Welcome, " + client_name);
            //     System.out.println("New client " + client_name + " has joined.");
            // }
            int login_reply = server.try_login_register(client_name);
            if(login_reply == -1){
                System.out.println("Client " + client_name + " is already logged in!");
                socket.write("ERROR: Already logged in from another session");
                socket.close_connection();
                return; 
            }
            else if(login_reply == 1){
                socket.write("Welcome back, " + client_name);
                System.out.println(client_name + " has reconnected.");
            }
            else if(login_reply == 0){
                socket.write("Welcome, " + client_name);
                System.out.println("New client " + client_name + " has joined.");
            }

        } catch (IOException | ClassNotFoundException e) {
            System.out.println("Error during client handshake");
            return;
        }


        while(true){
            try{
            Object obj = socket.read();

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
                else if(((Request)obj).request.equals(Request.DOWNLOAD_PUBLIC_FILE)){
                    String file_name = (String)((Request)obj).getData();
                    server.prepare_file_download(client_name, socket, file_name);
                }
                else if (((Request)obj).request.equals(Request.FILE_REQUEST)){
                    FileRequest file_request = (FileRequest) obj;
                    file_request.requestID = server.generate_request_ID();
                    System.out.println(client_name + " requested a file, request ID: " + file_request.requestID);
                    server.make_file_request(file_request);
                    server.request_to_all_users(file_request);
                }
                
                else if(((Request)obj).request.equals(Request.UPLOAD_FILE)){
                    System.out.println("Upload file request from " + client_name);
                    String metadata = (String) ((Request)obj).getData();
                    String result = server.receive_file_upload(client_name, socket, metadata);
                    System.out.println("Upload result: " + result);
                }
                else if(((Request)obj).request.equals(Request.VIEW_HISTORY)){
                    System.out.println("View history request from " + client_name);
                    CustomList history = server.get_client_history(client_name);
                    socket.write(history);
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
                    // System.out.println("Error closing connection for " + client_name);
                }
                break;
            } catch (ClassNotFoundException e) {
                System.out.println("Received unknown object from " + client_name);
            }
        }
    }
}
