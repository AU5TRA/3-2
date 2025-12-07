package server;

import java.io.IOException;
import java.net.ServerSocket;
import java.io.File;
import utility.*;
import java.util.List;
import java.util.HashMap;
import java.util.HashSet;
import java.util.ArrayList;

public class Server {
    private static int MAX_BUFFER_SIZE;
    private static int MIN_CHUNK_SIZE;
    private static int MAX_CHUNK_SIZE;

    private ServerSocket server_socket;
    private SocketUtil socket;
    private HashSet<String> clients;
    private List<String> files;  // Changed from CustomList to String
    private HashMap<String, SocketUtil> online_clients_map;

    public Server() throws IOException, ClassNotFoundException { 
        clients = new HashSet<String>();
        server_socket = new ServerSocket(6666);
        files = new ArrayList<String>();  
        online_clients_map = new HashMap<>();

        System.out.println("Server started on port 6666...");

        while(true){
            System.out.println("Waiting for connection...");
            socket = new SocketUtil(server_socket.accept());
            
            System.out.println("Connection established");
            System.out.println("Remote port: " + socket.socket.getPort());
            System.out.println("Local port: " + socket.socket.getLocalPort());
            
            String client_name = (String) socket.read();
            
            if(online_clients_map.containsKey(client_name)){
                System.out.println("Client " + client_name + " is already logged in!");
                socket.write("ERROR: Already logged in from another session");
                socket.close_connection();
                continue; 
            }
            else {
                online_clients_map.put(client_name, socket);
            }

            if(clients.contains(client_name)){
                socket.write("Welcome back, " + client_name);
                System.out.println(client_name + " has reconnected.");
            }
            else{
                clients.add(client_name);
                File file = new File("src/storage/" + client_name);
                if(!file.exists()){
                    file.mkdir();
                    System.out.println("Created directory for new client: " + client_name);
                }
                socket.write("Welcome, " + client_name);
                System.out.println("New client " + client_name + " has joined.");
            }
            new Worker(socket, client_name, this);
        }
    }
    public CustomList get_clients(String type) {
        if (type.equals("registered")) {
            System.out.println("Preparing list of all registered clients.");
            return new CustomList(new ArrayList<>(clients));
        } else if (type.equals("online")) {
            System.out.println("Preparing list of all online clients.");

            System.out.println("Online clients map: " + online_clients_map.keySet());
            return new CustomList(new ArrayList<>(online_clients_map.keySet()));
        } else {
            return null;
        }
    }

    public String close_client_connection(String client_name) {
        online_clients_map.remove(client_name);
        System.out.println("Client " + client_name + " has been removed from online clients.");
        return "Connection closed for " + client_name;
    }
    public static void main(String[] args) throws IOException, ClassNotFoundException {
        Server server = new Server();
    }
}