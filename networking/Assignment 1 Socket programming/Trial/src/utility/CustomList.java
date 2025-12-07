package utility;


import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.List;
import java.io.Serializable;

public class CustomList implements Serializable {
    public List<String> items;

    public CustomList(List<String> items) {
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

    
}