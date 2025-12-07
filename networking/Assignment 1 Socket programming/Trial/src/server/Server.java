package server;
// javac -d bin src/utility/*.java src/server/*.java src/client/*.java
// java -cp bin server.Server 
// java -cp bin client.Client
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.File;
import java.util.Scanner;
import utility.*;
import java.util.List;
import java.util.HashMap;
import java.util.HashSet;



public class Server {
    private static int MAX_BUFFER_SIZE;
    private static int MIN_CHUNK_SIZE;
    private static int MAX_CHUNK_SIZE;

    private ServerSocket server_socket;
    private SocketUtil socket;
    private Set<String> clients;
    private List<CustomList> files;
    private HashMap<String, SocketUtil> online_clients_map;

    public void Server(){
        clients = new HashSet<String>();
        server_socket = new ServerSocket(6666);
        files = new List<CustomList>();
        online_clients_map = new HashMap<String, SocketUtil>();

        File file = new File("src/storage");
        if(!file.exists()){
            file.mkdir();
        }

        while(true){
            socket= new SocketUtil(server_socket.accept());
            SocketUtil sck_util= new SocketUtil(socket); // socket for the current user trying to log in
            String client_name = (String) socket.input.readObject();
            if(clients.contains(client_name)){
                System.out.println("Client already exists.");
                continue;
            }
            clients.add(client_name);
            


        }
        
    }


    public static void main(String[] args) throws IOException, ClassNotFoundException {
        
        
        // while loop, so that server can connect to another client
        // after serving the current one
        while(true) {

            // Socket socket = welcomeSocket.accept();
            // System.out.println("Remote port: " + socket.socket.getPort());
            // System.out.println("Local port: " + socket.socket.getLocalPort());

            // output buffer and input buffer
            // ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream());
            // ObjectInputStream in = new ObjectInputStream(socket.getInputStream());

            // send message to client
            // read message from client
            

            String client_name = (String) socket.input.readObject();
            System.out.println(client_name + " has connected to the server.");
        }

    }
}
