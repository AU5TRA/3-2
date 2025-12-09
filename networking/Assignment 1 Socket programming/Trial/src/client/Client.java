package client;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.io.File;
import java.util.Scanner;
import java.util.ArrayList;
import java.net.SocketTimeoutException;
import java.io.FileOutputStream;
import java.io.FileInputStream;

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

        File file_download = new File("src/client/downloads");
        if(!file_download.exists()){
            file_download.mkdir();
            System.out.println("client side: Created directory downloads in src/client/");

        }

        socket = new SocketUtil(serverAddr, serverPort);
        System.out.println("Enter client name: ");
        client_name = scanner.nextLine();
        
    }

        private static void upload_file_chunked(File file, long file_size) throws IOException, ClassNotFoundException{
        int chunk_size = 4096;
        long uploaded = 0;
        
        try (FileInputStream fis = new FileInputStream(file)) {
            byte[] buffer = new byte[chunk_size];
            int read_bytes;
            
            while ((read_bytes = fis.read(buffer)) != -1) {
                socket.write(buffer, 0, read_bytes);
                uploaded += read_bytes;
                
                double progress = (double) uploaded / file_size * 100;
                int bar_length = 50;
                int filled = (int) (bar_length * uploaded / file_size);
                
                System.out.print("\rUpload progress: [");
                for (int i = 0; i < bar_length; i++) {
                    if (i < filled) System.out.print("=");
                    else System.out.print(" ");
                }
                System.out.printf("] %.1f%% (%d/%d bytes)", progress, uploaded, file_size);
            }
            
            System.out.println(); 
            
            socket.write("<EOF>");
            System.out.println("File data sent. Waiting for server confirmation...");
            
            Object confirmation = socket.read();
            if (confirmation instanceof String) {
                String msg = (String) confirmation;
                if (msg.equals("UPLOAD_SUCCESS")) {
                    System.out.println("File uploaded successfully!");
                } else {
                    System.out.println("Server response: " + msg);
                }
            }
        } catch (IOException e) {
            System.out.println("Error uploading file: " + e.getMessage());
            e.printStackTrace();
        }
    }

    public static void main(String[] args) throws IOException, ClassNotFoundException {
        String server_address = "localhost";
        int server_port = 6666;
        Client client = new Client(server_address, server_port);

        socket.write(client_name);
        String reply = (String) socket.read();
        System.out.println("Server: " + reply);


        // System.out.println("Connection established");
        // System.out.println("Remote port: " + socket.socket.getPort());
        // System.out.println("Local port: " + socket.socket.getLocalPort());
        boolean running = true;

        while(running){
            System.out.println("---------------------------");  
            System.out.println("1. Show all clients"); // done
            System.out.println("2. Show all online clients"); // done
            System.out.println("3. Show my uploaded files"); // done
            System.out.println("4. Download my file");// done
            System.out.println("5. Upload a file");
            System.out.println("6. Show public files of other clients"); // done
            System.out.println("7. Download public files of other clients"); //  done
            System.out.println("8. Make a file request"); // done
            System.out.println("9. View Unread messages");
            System.out.println("10. View upload and download history");
            System.out.println("11. Log Out"); // done
            System.out.println("Enter your choice: ");

            int choice = scanner.nextInt();

            switch(choice){
                case 1:
                    Request r = new Request(Request.LIST_ALL_CLIENTS);
                    socket.write(r);
                    System.out.println("Requesting list of all clients...");
                    Object all_clients = socket.read();
                    System.out.println("Received list of all clients from server.");
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
                case 3:
                    socket.write(new Request(Request.LIST_UPLOADED_FILES));
                    Object uploaded_files = socket.read();
                    CustomList filesList = null;
                    if (uploaded_files instanceof CustomList) {
                        filesList = (CustomList) uploaded_files;
                        filesList.showFiles(client_name, "self");
                    }
                    break;
                case 4:
                    socket.write(new Request(Request.LIST_UPLOADED_FILES));
                    uploaded_files = socket.read();
                    filesList = null;
                    if (uploaded_files instanceof CustomList) {
                        filesList = (CustomList) uploaded_files;
                        filesList.showFiles(client_name, "self");
                    }
                    if (filesList != null && filesList.items.size() > 0) {
                        System.out.println("Enter the number of the file to download: ");
                        int file_choice = scanner.nextInt();
                        socket.write(new Request(Request.DOWNLOAD_FILE, file_choice));
                        Object response = socket.read();
                        if (response instanceof String) {
                            String file_info = (String) response;
                            String[] parts = file_info.split(":");
                            if (parts.length == 3 && parts[0].equals("FILE_INFO")) {
                                String file_name = parts[1];
                                long file_size = Long.parseLong(parts[2]);
                                System.out.println("Downloading file: " + file_name + " of size " + file_size + " bytes.");
                                download_file(file_name, 4096, file_size);
                            } else {
                                System.out.println("Invalid file info received from server.");
                            }
                        } else {
                            System.out.println("Invalid response from server.");
                        }
                    }
                    else{
                        System.out.println("No files available to download.");
                    }
                break;
                                case 5:
                    System.out.println("Is this a requested file? [y/n]: ");
                    char is_requested = scanner.next().charAt(0);
                    scanner.nextLine();
                    
                    if(is_requested == 'y' || is_requested == 'Y'){
                        System.out.println("Enter the request ID: ");
                        String request_id = scanner.nextLine();
                        System.out.println("Enter the file name to upload (place the file in src/client/to_upload/): ");
                        String file_name = scanner.nextLine();
                        File file_to_upload = new File("src/client/to_upload/" + file_name);
                        
                        if(!file_to_upload.exists() || !file_to_upload.isFile()){
                            System.out.println("File not found in src/client/to_upload/");
                            break;
                        }
                        
                        long file_size = file_to_upload.length();
                        System.out.println("File size: " + file_size + " bytes");
                        
                        socket.write(new Request(Request.UPLOAD_FILE, "REQUESTED:" + request_id + ":" + file_name + ":" + file_size));
                        
                        Object server_response = socket.read();
                        if(server_response instanceof String){
                            String response = (String) server_response;
                            if(response.equals("READY_TO_RECEIVE")){
                                System.out.println("Server ready. Starting file upload...");
                                upload_file_chunked(file_to_upload, file_size);
                            }
                            else{
                                System.out.println("Server response: " + response);
                            }
                        }
                    }
                    else{
                        System.out.println("Enter file name to upload (place the file in src/client/to_upload/): ");
                        String file_name = scanner.nextLine();
                        File file_to_upload = new File("src/client/to_upload/" + file_name);
                        
                        if(!file_to_upload.exists() || !file_to_upload.isFile()){
                            System.out.println("File not found in src/client/to_upload/");
                            break;
                        }
                        
                        long file_size = file_to_upload.length();
                        System.out.println("Is this file public? [y/n]: ");
                        char is_public = scanner.next().charAt(0);
                        String visibility = (is_public == 'y' || is_public == 'Y') ? "public" : "private";
                        
                        socket.write(new Request(Request.UPLOAD_FILE, visibility + ":" + file_name + ":" + file_size));
                        
                        Object server_response = socket.read();
                        if(server_response instanceof String){
                            String response = (String) server_response;
                            if(response.equals("READY_TO_RECEIVE")){
                                System.out.println("Server ready. Starting file upload...");
                                upload_file_chunked(file_to_upload, file_size);
                            }
                            else{
                                System.out.println("Server response: " + response);
                            }
                        }
                    }
                    break;
                case 6:
                    socket.write(new Request(Request.LIST_PUBLIC_FILES));
                    Object public_files = socket.read();
                    if (public_files instanceof CustomList) {
                        filesList = (CustomList) public_files;
                        filesList.showFiles(client_name, "all");
                    }
                    break;
                case 7:
                    socket.write(new Request(Request.LIST_PUBLIC_FILES));
                    public_files = socket.read();
                    filesList = null;
                    if (public_files instanceof CustomList){
                        filesList = (CustomList) public_files;
                        filesList.showFiles(client_name, "all");
                    }
                    if(filesList != null && filesList.items.size()>0){
                        System.out.println("Enter the number of the public file to download: ");
                        int file_choice = scanner.nextInt();
                        String chosen_file_name= filesList.retrieve(file_choice-1);
                        socket.write(new Request(Request.DOWNLOAD_PUBLIC_FILE, chosen_file_name));
                        Object response = socket.read();
                        if (response instanceof String) {
                            String file_info = (String) response;
                            String[] parts = file_info.split(":");
                            if (parts.length == 3 && parts[0].equals("FILE_INFO")) {
                                String file_name = parts[1];
                                long file_size = Long.parseLong(parts[2]);
                                System.out.println("Downloading file: " + file_name + " of size " + file_size + " bytes.");
                                download_file(file_name, 4096, file_size);
                            } else {
                                System.out.println("Invalid file info received from server.");
                            }
                        } else {
                            System.out.println("Invalid response from server.");
                        }
                    }
                    else{
                        System.out.println("No files available to download.");
                    }
                    break;
                    
                case 8:
                    scanner.nextLine(); // consume newline
                    System.out.println("Enter description for the file request: ");
                    String description = scanner.nextLine();
                    FileRequest file_request = new FileRequest(client_name, description);
                    socket.write(file_request);
                    System.out.println("File request sent to server.");
                    break;
                case 11:
                    socket.write(new Request(Request.LOG_OUT));
                    running = false;
                    break;
                default:
                    System.out.println("Invalid choice. Please try again.");
        }
    }
    }


    private static void download_file(String file_name, int chunk_size, long file_size) throws IOException, ClassNotFoundException {
        String dirPath = "src/client/downloads/";
        File file = new File(dirPath + file_name);

        File parentDir = file.getParentFile();

        if (parentDir != null && !parentDir.exists()) {
            parentDir.mkdirs();  
        }

        FileOutputStream fos = new FileOutputStream(file);

        try {
            byte[] buffer = new byte[chunk_size];

            while (file_size > 0) {
                int read_bytes;
                try {
                    read_bytes = socket.read(buffer, 0, Math.min(buffer.length, (int) file_size));
                } catch (SocketTimeoutException e) {
                    System.out.println("Timeout in receiving file " + file_name);
                    fos.close();
                    return;
                }

                if (read_bytes == -1) break;

                fos.write(buffer, 0, read_bytes);
                file_size -= read_bytes;
            }

            fos.close();
        } catch (IOException e) {
            System.out.println(e);
            fos.close();
        }

        String final_msg = (String) socket.read();
        if (final_msg.equals("done")) {
            System.out.println("File " + file_name + " downloaded successfully.");
        } else {
            System.out.println("File download failed.");
        }
    }
}


















      