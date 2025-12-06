package client;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class Client {
    private static String client_name;
    private static Scanner scanner = new Scanner(System.in);
    private static socketUtil socket;




    public Client(String serverAddr, int serverPort) throws IOException {
        File file = new File("src/client/to_upload");
        if(!file.exists()){
            file.mkdir();
        }

        File file_download = new File("src/client/to_download");
        if(!file_download.exists()){
            file_download.mkdir();
        }

        socket = new socketUtil(serverAddr, serverPort);
        System.out.println("Enter client name: ");
        client_name = scanner.nextLine();
        
    }
    public static void main(String[] args) throws IOException, ClassNotFoundException {
        String server_address = "localhost";
        int server_port = 6666;
        Client client = new Client(server_address, server_port);






        while(true){
            System.out.println("a. List of current clients online");
            System.out.println("b. Look Up list of uploaded files");
            System.out.println("c. Download own file");
            System.out.println("d. Look Up public files of other clients");
            System.out.println("e. Download public files of other clients");
            System.out.println("f. Make a file request");
            System.out.println("g. View Unread messages");
            System.out.println("h. Upload a file");
            System.out.println("i. View upload and download history");
            
        }
        System.out.println("Connection established");
        System.out.println("Remote port: " + socket.getPort());
        System.out.println("Local port: " + socket.getLocalPort());

        // output buffer and input buffer
        ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream());
        ObjectInputStream in = new ObjectInputStream(socket.getInputStream());

        // read message from server
        String msg = (String) in.readObject();
        System.out.println(msg);

        // write message to server
        out.writeObject("Hello from client");
    }
}
