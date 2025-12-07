package client;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.io.File;
import java.util.Scanner;

import utility.*;

public class Client {
    private static String client_name;
    private static Scanner scanner = new Scanner(System.in);
    private static SocketUtil socket;




    public Client(String serverAddr, int serverPort) throws IOException {
        File file = new File("src/client/to_upload");
        if(!file.exists()){
            file.mkdir();
            System.out.println("client side: Created directory to_upload in src/client/");
        }

        File file_download = new File("src/client/to_download");
        if(!file_download.exists()){
            file_download.mkdir();
            System.out.println("client side: Created directory to_download in src/client/");

        }

        socket = new SocketUtil(serverAddr, serverPort);
        System.out.println("Enter client name: ");
        client_name = scanner.nextLine();
        
    }


    public static void main(String[] args) throws IOException, ClassNotFoundException {
        String server_address = "localhost";
        int server_port = 6666;
        Client client = new Client(server_address, server_port);

        socket.write(client_name);
        String reply = (String) socket.read();


        System.out.println("Connection established");
        System.out.println("Remote port: " + socket.socket.getPort());
        System.out.println("Local port: " + socket.socket.getLocalPort());


        while(true){
            System.out.println("1. Show all clients");
            System.out.println("2. Show all online clients");
            System.out.println("3. Show my uploaded files");
            System.out.println("4. Download my file");
            System.out.println("5. Upload a file");
            System.out.println("6. Show public files of other clients");
            System.out.println("7. Download public files of other clients");
            System.out.println("8. Make a file request");
            System.out.println("9. View Unread messages");
            System.out.println("10. View upload and download history");
            System.out.println("11. Log Out");
            System.out.println("Enter your choice: ");

            int choice = scanner.nextInt();

            switch(choice){
                case 1:
                    socket.write(new Request(Request.LIST_ALL_CLIENTS));
                    Object all_clients = socket.read();
                    if (all_clients instanceof CustomList) {
                        CustomList clientsList = (CustomList) all_clients;
                        clientsList.showUsers("All");
                    }
                    break;
                case 2:
                    socket.write(new Request(Request.LIST_ONLINE_CLIENTS));
                    Object online_clients = socket.read();
                    if (online_clients instanceof CustomList) {
                        CustomList clientsList = (CustomList) online_clients;
                        clientsList.showUsers("Online");
                    }
                    break;
                    
                default:
                    System.out.println("Invalid choice. Please try again.");
        }
    }
    }
}
















      