package server;
// javac -d bin src/utility/*.java src/server/*.java src/client/*.java
// java -cp bin server.Server
// java -cp bin client.Client
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
import java.io.FileOutputStream;
import java.lang.Math.*;
import java.util.concurrent.*;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import java.util.Set;

public class Server {
    public int MIN_CHUNK_SIZE, MAX_CHUNK_SIZE;
    public long MAX_BUFFER_SIZE;
    public AtomicLong CUR_BUFFER_SIZE;

    private ServerSocket server_socket;
    private SocketUtil socket;
    private Set<String> clients;
    private List<String> files;
    private ConcurrentMap<String, SocketUtil> online_clients_map;
    private CopyOnWriteArrayList<FileRequest> file_request_list;
    private ConcurrentMap<String, List<Message>> messages;
    private ConcurrentMap<String, UploadSession> uploadSessions;
    private AtomicInteger requestCounter = new AtomicInteger(0);
    private AtomicInteger uploadCounter = new AtomicInteger(0);

    public Server() throws IOException, ClassNotFoundException { 
        this.CUR_BUFFER_SIZE = new AtomicLong(0L);
        this.MAX_BUFFER_SIZE = 250 * 1024; // default 250 KB
        this.MIN_CHUNK_SIZE = 1024; // 1 KB
        this.MAX_CHUNK_SIZE = 8 * 1024; // 8 KB
        this.uploadSessions = new ConcurrentHashMap<>();



        clients = Collections.synchronizedSet(new HashSet<String>());
        server_socket = new ServerSocket(6666);
        files = new ArrayList<String>();  
        online_clients_map = new ConcurrentHashMap<>();
        messages = new ConcurrentHashMap<>();
        file_request_list = new CopyOnWriteArrayList<>();


        System.out.println("Server started on port 6666...");

        while(true){
            System.out.println("Waiting for connection...");
            socket = new SocketUtil(server_socket.accept());
            
            System.out.println("Connection established");
            // System.out.println("Remote port: " + socket.socket.getPort());
            // System.out.println("Local port: " + socket.socket.getLocalPort());
            
            new Worker(socket, this);
        }
    }
    public synchronized CustomList get_clients(String type) {
        if (type.equals("registered")) {
            // System.out.println("Preparing list of all registered clients.");
            return new CustomList(new ArrayList<>(clients));
        } else if (type.equals("online")) {
            // System.out.println("Preparing list of all online clients.");
            return new CustomList(new ArrayList<>(online_clients_map.keySet()));
        } else {
            return null;
        }
    }

    public synchronized CustomList list_uploaded_files(String client_name) {
        // System.out.println("Preparing list of uploaded files for " + client_name);
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

    public synchronized CustomList list_public_files() {
        System.out.println("Preparing list of all public files from all clients.");
        ArrayList<String> public_files = new ArrayList<>();
        for (String client_name : new ArrayList<>(clients)) {
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

    public String prepare_file_download(String client_name, SocketUtil socket, int fileNumber) {
        System.out.println("Preparing file download for file number " + fileNumber + " requested by " + client_name);
        CustomList uploaded_files = list_uploaded_files(client_name);
        if (fileNumber < 1 || fileNumber > uploaded_files.items.size()) {
            try { socket.write(Color.RED+ "ERROR" + Color.RESET + ": Invalid file number"); } catch (IOException ignored) {}
            return "ERROR";
        }
        String file_path = uploaded_files.items.get(fileNumber - 1);
        String full_path = "src/storage/" + client_name + "/" + file_path;

        System.out.println("Full file path: " + full_path);
        File file = new File(full_path);
        if (!file.exists() || !file.isFile()) {
            try { socket.write(Color.RED+ "ERROR" + Color.RESET + ": File not found on server."); } catch (IOException ignored) {}
            return "ERROR";
        }

        try (FileInputStream fis = new FileInputStream(file)) {
            long file_size = file.length();
            String info = "FILE_INFO:" + file_path + ":" + file_size;
            socket.write(info);

            byte[] buffer = new byte[4096];
            long remaining = file_size;
            while (remaining > 0) {
                int read = fis.read(buffer, 0, (int)Math.min(buffer.length, remaining));
                if (read == -1) break;
                socket.write(buffer, 0, read);
                remaining -= read;
            }
            socket.write("done"); 
            System.out.println("File " + file_path + " sent to " + client_name);
            return "OK";
        } catch (IOException e) {
            System.out.println("Error sending file: " + e.getMessage());
            try { socket.write(Color.RED+ "ERROR" + Color.RESET + ": Failed to download file."); } catch (IOException ignored) {}
            return "ERROR";
        }
    }

    public String prepare_file_download(String client_name, SocketUtil socket, String file_path){
        System.out.println("Server side " + file_path);
        String[] parts = file_path.split("/", 2);
        String extracted_path = parts.length > 1 ? parts[1] : file_path;
        // file_path = parts[0] + "/public/" + extracted_path;
        file_path = "/public/" + extracted_path;

        String full_path = "src/storage/"  + parts[0] + file_path;
        System.out.println("File extracted from: "  + full_path);
        System.out.println("File exported to: " + file_path);
        File file = new File(full_path);
        if (!file.exists() || !file.isFile()) {
            try { socket.write(Color.RED+ "ERROR" + Color.RESET + ": File not found on server."); } catch (IOException ignored) {}
            return "ERROR";
        }

        try (FileInputStream fis = new FileInputStream(file)) {
            long file_size = file.length();
            String info = "FILE_INFO:" + file_path + ":" + file_size;
            socket.write(info);

            byte[] buffer = new byte[4096];
            long remaining = file_size;
            while (remaining > 0) {
                int read = fis.read(buffer, 0, (int) Math.min(buffer.length, remaining));
                if (read == -1) break;
                socket.write(buffer, 0, read);
                remaining -= read;
            }
            socket.write("done"); 
            System.out.println("File " + file_path + " sent to " + client_name);
            return "OK";
        } catch (IOException e) {
            System.out.println("Error sending file: " + e.getMessage());
            try { socket.write(Color.RED+ "ERROR" + Color.RESET + ": Failed to download file."); } catch (IOException ignored) {}
            return "ERROR";
        }
    }

    public String generate_request_ID() {
        return String.valueOf(requestCounter.incrementAndGet());
    }

    public void make_file_request(FileRequest file_request) {
        file_request_list.add(file_request); // used copyOnWrite
    }

    public void request_to_all_users(FileRequest file_request) {
        String description = file_request.requester + " has made a request (Request ID: " + file_request.requestID + ") with the description:\n" + file_request.description;
        System.out.println("Spreading file request: " + description);
        Message m = new Message(file_request.requester, "all", true, description);
        for (String client_name : new ArrayList<>(clients)) { 
            if (client_name.equals(file_request.requester)) // don't send to requester
                continue;
            // messages.get(client_name).add(m);
            messages.computeIfAbsent(client_name, k -> Collections.synchronizedList(new ArrayList<>())).add(m);
        }
    }

    public synchronized boolean isClientOnline(String client_name) {
        return online_clients_map.containsKey(client_name);
    }
    
    public synchronized void addOnlineClient(String client_name, SocketUtil socket) {
        online_clients_map.put(client_name, socket);
    }
    
    public synchronized boolean isRegisteredClient(String client_name) {
        return clients.contains(client_name);
    }
    
    public synchronized void registerNewClient(String client_name) throws IOException {
        clients.add(client_name);
        messages.put(client_name, new ArrayList<>());
        File file = new File("src/storage/" + client_name);
        if(!file.exists()){
            file.mkdir();
            file = new File("src/storage/" + client_name + "/public");
            file.mkdir();
            file = new File("src/storage/" + client_name + "/private");
            file.mkdir();
            System.out.println("Created directory for new client: " + client_name);
        }
    }

    private synchronized void discardIncompleteUploadsForClient(String client_name) {
        List<String> toRemove = new ArrayList<>();
        long freed_space = 0L;
        for (UploadSession s : uploadSessions.values()) {
            if (s.uploader.equals(client_name)) {
                synchronized(s){
                    try {
                        if (s.tempFile != null && s.tempFile.exists()) 
                            s.tempFile.delete();
                    } catch (Exception ignored) {}
                    freed_space += s.expectedSize;
                    // CUR_BUFFER_SIZE -= s.expectedSize;
                    toRemove.add(s.fileID);
                    System.out.println("Discarded incomplete upload " + s.fileID + " from " + client_name);
                }
            }
        }
        for (String fid : toRemove) uploadSessions.remove(fid);
        CUR_BUFFER_SIZE.addAndGet(-freed_space); // atomic long
    }

    public String close_client_connection(String client_name) {
        discardIncompleteUploadsForClient(client_name);
        online_clients_map.remove(client_name);
        System.out.println("Client " + client_name + " has been removed from online clients.");
        return "Connection closed for " + client_name;
    }
    
    public String receive_file_upload(String client_name, SocketUtil sck, String metadata) {
        System.out.println("Receiving file upload (negotiation) from " + client_name + " metadata: " + metadata);
        String[] parts = metadata.split(":");
        String filename;
        String upload_dir;
        long file_size;

        if (parts[0].equals("REQUESTED")) {
            // REQUESTED:request_id:file_name:file_size
            filename = parts[2];
            file_size = Long.parseLong(parts[3]);
            upload_dir = "src/storage/" + client_name + "/public";
        } else {
            // public/private:file_name:file_size
            filename = parts[1];
            file_size = Long.parseLong(parts[2]);
            upload_dir = "src/storage/" + client_name + "/" + parts[0];
        }

        File dir = new File(upload_dir);
        if (!dir.exists()) dir.mkdirs();
        
        // for threads
        // synchronized (this) {
        long prev_size;
        while(true){
            prev_size = CUR_BUFFER_SIZE.get();
            if (prev_size + file_size > MAX_BUFFER_SIZE) {
                try { sck.write("REJECT:BUFFER_FULL"); } catch (IOException ignored) {}
                System.out.println("Rejected upload from " + client_name + " because buffer full");
                return "ERROR: BUFFER_FULL";
            }
            // CUR_BUFFER_SIZE += file_size;
            if(CUR_BUFFER_SIZE.compareAndSet(prev_size, prev_size + file_size)){
                break;
            }
        }
        // }

        String fileID = "File_" + System.currentTimeMillis() + "_" + (uploadCounter.incrementAndGet());
        int random_chunk_size = (int)(Math.random() * (MAX_CHUNK_SIZE - MIN_CHUNK_SIZE + 1) + MIN_CHUNK_SIZE);

        // int random_chunk_size = Math.min((int) Math.min(MAX_CHUNK_SIZE, file_size), Math.max(MIN_CHUNK_SIZE, (int)(Math.random() * (MAX_CHUNK_SIZE - MIN_CHUNK_SIZE + 1) + MIN_CHUNK_SIZE)));

        File tempFile = new File(upload_dir + "/.upload_" + fileID + ".tmp");
        UploadSession session = new UploadSession();
        session.fileID = fileID;
        session.uploader = client_name;
        session.filename = filename;
        session.expectedSize = file_size;
        session.receivedSize = 0;
        session.tempFile = tempFile;
        session.chunkSize = random_chunk_size;
        uploadSessions.put(fileID, session);
        /////////////////
        try {
            // tell client to start with chunk size and fileID
            sck.write("READY:" + random_chunk_size + ":" + fileID);
            System.out.println("Chunk Size:" + random_chunk_size + " fileID:" + fileID + " Client:" + client_name);

            FileOutputStream fos = new FileOutputStream(tempFile);
            byte[] buffer = new byte[random_chunk_size];
            int seq = 0;
            // synchronized(session){
            //     while (session.receivedSize < session.expectedSize) {
            //         seq++;
            //         int toRead = (int) Math.min(buffer.length, session.expectedSize - session.receivedSize);
            //         int got = sck.read(buffer, 0, toRead);
            //         if (got <= 0) {
            //             throw new IOException("Client disconnected while uploading");
            //         }
            //         fos.write(buffer, 0, got);
            //         session.receivedSize += got;

            //         // send ack for got chunk
            //         try { sck.write("ACK:" + fileID + ":" + seq + ":" + got); } catch (IOException ignored) {}
            //     }
            // }
            while(true){
                // synchronizing only what is necessary
                synchronized(session){
                    if (session.receivedSize >= session.expectedSize) {
                        break;
                    }
                }
                seq++;
                int to_read;
                synchronized(session){
                    to_read = (int) Math.min(buffer.length, session.expectedSize - session.receivedSize);
                }
                int got = sck.read(buffer, 0, to_read);
                if (got <= 0) {
                    throw new IOException("Client disconnected while uploading");
                }
                fos.write(buffer, 0, got);
                synchronized(session){
                    session.receivedSize += got;    
                }
                try{
                    sck.write("ACK:" + fileID + ":" + seq + ":" + got);
                } catch (IOException ignored) { }
            }
            fos.close();

            Object final_msg_obj = sck.read();
            String final_msg = "";
            if(final_msg_obj instanceof String)
                final_msg = (String) final_msg_obj;
            // String finalMsg = finalMsgObj instanceof String ? (String) finalMsgObj : "";


            if (session.receivedSize == session.expectedSize && final_msg.equals("COMPLETED:" + fileID)) {
                // client says ok
                File finalFile = new File(upload_dir + "/" + filename);
                if (finalFile.exists()) 
                    finalFile.delete();
                // if there is a file of that name, we erase it first    
                tempFile.renameTo(finalFile);

                // synchronized (this) {
                //     CUR_BUFFER_SIZE -= session.expectedSize; // empty buffer
                // }
                // auto thread safe
                CUR_BUFFER_SIZE.addAndGet(-session.expectedSize);
                
                uploadSessions.remove(fileID);
                sck.write("UPLOAD_SUCCESS");

                System.out.println("Upload success " + filename + " from " + client_name);
                return "OK";
            } else {
                fos.close();
                if (tempFile.exists()) 
                    tempFile.delete(); // delete temp file to clear memory
                // synchronized (this) {
                //     CUR_BUFFER_SIZE -= session.expectedSize;
                // }
                CUR_BUFFER_SIZE.addAndGet(-session.expectedSize);
                uploadSessions.remove(fileID);
                sck.write(Color.RED + "UPLOAD_FAILED" + Color.RESET);
                System.out.println("Upload failed (size mismatch or bad completion) for " + fileID);
                return "ERROR: UPLOAD_FAILED";
            }
        } catch (IOException | ClassNotFoundException e) {
            System.out.println("Error during upload: " + e.getMessage());
            try {
                if (session.tempFile != null && session.tempFile.exists()) session.tempFile.delete();
            } catch (Exception ignored) {}
            // synchronized (this) {
            //     CUR_BUFFER_SIZE -= session.expectedSize;
            // }
            CUR_BUFFER_SIZE.addAndGet(-session.expectedSize);
            uploadSessions.remove(fileID);
            try { sck.write("UPLOAD_FAILED"); } catch (IOException ignored) {}
            return "ERROR: " + e.getMessage();
        }
    }
    public synchronized int try_login_register(String client_name){
        if(online_clients_map.containsKey(client_name)){
            return -1; // already online
        }
        online_clients_map.put(client_name, socket);
        boolean registered = clients.contains(client_name);
        if(!registered){
            clients.add(client_name);
            messages.put(client_name, new ArrayList<>());
            File file = new File("src/storage/" + client_name);
            if(!file.exists()){
                file.mkdir();
                file = new File("src/storage/" + client_name + "/public");
                file.mkdir();
                file = new File("src/storage/" + client_name + "/private");     
                file.mkdir();
                System.out.println("Created directory for new client: " + client_name);
            }
            return 0; // registered
        }
        return 1;
    }

    public static void main(String[] args) throws IOException, ClassNotFoundException {
        Server server = new Server();
    }
}