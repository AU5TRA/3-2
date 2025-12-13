package utility;


import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.ArrayList;
import java.io.Serializable;

public class CustomList implements Serializable {
    public ArrayList<String> items;

    public CustomList(ArrayList<String> items) {
        this.items = items;
    }

    public void showUsers(String user_type) {
        System.out.println(user_type + " Users:");
        int i = 1;
        for (String item : items) {
            System.out.println(i+ ". " + item);
            i++;
        }
        System.out.println();
    }
    public String retrieve(int index){
        return items.get(index);
    }
    public void showMessages(String file_type) {
        
        if(items.size() == 0){
            System.out.println("No unread messages.");
            return;
        }
        System.out.println("Unread Messages:");
        int i = 1;
        for (String item : items) {
            System.out.println(i+ ". " + item);
            i++;
        }
        System.out.println();
    }
    public void showFiles(String client_name, String type) {
        
        if(items.size() == 0){
            if(type.equals("self")){
                System.out.println("No uploaded files found for " + client_name);
            }
            else if(type.equals("all")){
                System.out.println("No public files found");
            }
            return;
        }
        if(type.equals("self")){
            System.out.println("Uploaded Files for " + client_name + ":");
        }
        else if(type.equals("all")){
            System.out.println("Available public files:");
        }
        int i = 1;
        for (String item : items) {
            System.out.println(i+ ". " + item);
            i++;
        }
        System.out.println();
    }

    public void showHistory(String client_name) {
        
        if(items.size() == 0){
            System.out.println("No history found for " + client_name);
            return;
        }
        System.out.println("History for " + client_name + ":");
        int i = 1;
        for (String item : items) {
            System.out.println(i+ ". " + item);
            i++;
        }
        System.out.println();
    }

    public void showMessages() {
        
        if(items.size() == 0){
            System.out.println("No unread messages.");
            return;
        }
        System.out.println("Unread Messages:");
        int i = 1;
        for (String item : items) {
            System.out.println(i+ ". " + item);
            i++;
        }
        System.out.println();
    }
}