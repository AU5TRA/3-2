package server;

import java.io.IOException;
import java.net.ServerSocket;
import java.io.File;
import utility.*;
import java.util.List;
import java.util.HashMap;
import java.util.HashSet;
import java.util.ArrayList;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.io.FileInputStream;


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
                    file = new File("src/storage/" + client_name + "/public");
                    file.mkdir();
                    file = new File("src/storage/" + client_name + "/private");
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

    public CustomList list_uploaded_files(String client_name) {
        System.out.println("Preparing list of uploaded files for " + client_name);
        String dir_name = "src/storage/" + client_name;
        File client_dir = new File(dir_name + "/public");

        ArrayList<String> files_array = new ArrayList<>();
        String[] file_list = client_dir.list();
        if (file_list != null) {
            for (String file_name : file_list) {
                file_name = "public/" + file_name;
                files_array.add(file_name);
            }
        }

        client_dir = new File(dir_name + "/private");
        file_list = client_dir.list();
        if (file_list != null) {
            for (String file_name : file_list) {
                file_name = "private/" + file_name;
                files_array.add(file_name);
            }
        }
        return new CustomList(files_array);
    }
    public CustomList list_public_files() {
        System.out.println("Preparing list of all public files from all clients.");
        ArrayList<String> public_files = new ArrayList<>();
        for (String client_name : clients) {
            String dir_name = "src/storage/" + client_name + "/public";
            File client_dir = new File(dir_name);
            String[] file_list = client_dir.list();
            if (file_list != null) {
                for (String file_name : file_list) {
                    public_files.add(client_name + "/" + file_name);
                }
            }
        }
        return new CustomList(public_files);
    }

    // ...existing code...
public String prepare_file_download(String client_name, SocketUtil socket, int fileNumber) {
    System.out.println("Preparing file download for file number " + fileNumber + " requested by " + client_name);
    CustomList uploaded_files = list_uploaded_files(client_name);
    if (fileNumber < 1 || fileNumber > uploaded_files.items.size()) {
        try { socket.write("ERROR: Invalid file number"); } catch (IOException ignored) {}
        return "ERROR";
    }
    String file_path = uploaded_files.items.get(fileNumber - 1);
    String full_path = "src/storage/" + client_name + "/" + file_path;

    System.out.println("Full file path: " + full_path);
    File file = new File(full_path);
    if (!file.exists() || !file.isFile()) {
        try { socket.write("ERROR: File not found on server."); } catch (IOException ignored) {}
        return "ERROR";
    }

    try (FileInputStream fis = new FileInputStream(file)) {
        long file_size = file.length();
        String info = "FILE_INFO:" + file_path + ":" + file_size;
        socket.write(info); // send file info first

        byte[] buffer = new byte[4096];
        long remaining = file_size;
        while (remaining > 0) {
            int read = fis.read(buffer, 0, (int) Math.min(buffer.length, remaining));
            if (read == -1) break;
            socket.write(buffer, 0, read); // stream bytes
            remaining -= read;
        }
        socket.write("done"); // final marker
        System.out.println("File " + file_path + " sent to " + client_name);
        return "OK";
    } catch (IOException e) {
        System.out.println("Error sending file: " + e.getMessage());
        try { socket.write("ERROR: Failed to download file."); } catch (IOException ignored) {}
        return "ERROR";
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