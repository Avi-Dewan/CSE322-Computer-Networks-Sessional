package util;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ClientUtil {
    private String name;
    private HashMap<String, Boolean> fileNames; // True if public file, False if private
    private boolean online; // True if client is online
    private ArrayList<String> messages; // True if messages are unread

    public ClientUtil(String name) {
        this.name = name;
        this.fileNames = new HashMap<>();
        this.online = true;
        this.messages = new ArrayList<>();
    }
    public String getName() {
        return name;
    }

    public boolean isOnline() {
        return online;
    }

    public void setOnline(boolean online) {
        this.online = online;
    }

    public HashMap<String, Boolean> getFileNames() {
        return fileNames;
    }


    public List<Pair> getPublicFileNames() {
        List<Pair> publicFiles = new ArrayList<>();

        for (Map.Entry<String, Boolean> entry : fileNames.entrySet()) {
            String key = entry.getKey();
            boolean value = entry.getValue();

            if (value) {
                publicFiles.add(new Pair(key, name));
            }
        }

        return publicFiles;
    }

    public void addFileName(String fileName, boolean isPublic) {
        fileNames.put(fileName, isPublic);
    }


    public ArrayList<String> getMessages() {
        return messages;
    }

    public void addMessage(String message) {
        messages.add(message);
    }

    public void markAllMessagesAsRead() {
        messages.clear();
    }
}
