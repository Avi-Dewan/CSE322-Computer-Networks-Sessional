package Client;

import util.NetworkUtil;
import util.Pair;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.Scanner;

public class ThreadClient implements Runnable {

    private Thread thr;
    private NetworkUtil networkUtil;
    String name;

    public ThreadClient(NetworkUtil networkUtil, String name) {
        this.networkUtil = networkUtil;
        this.name = name;
        this.thr = new Thread(this);
        thr.start();
    }
    public void run() {
        try {

            Scanner input = new Scanner(System.in);

            while (true) {
                System.out.println("\n\nPress [Enter <--| ] to open MENU");
                input.nextLine();
                System.out.println("Select from below:\n" +
                        "1. Look up list of clients\n" +
                        "2. Look up your files\n" +
                        "3. Look up public files of other clients\n" +
                        "4. Make a file request\n" +
                        "5. Upload a file\n" +
                        "6. View all unread messages\n" +
                        "7. Disconnect the client");

                int option = input.nextInt();
                input.nextLine();

                System.out.println("\n");

                networkUtil.write(option);

                switch (option) {
                    case 1:
                        lookupClients();
                        break;
                    case 2:
                        lookupOwnFiles();
                        break;
                    case 3:
                        lookupOtherFiles();
                        break;
                    case 4:
                        makeFileRequest();
                        break;
                    case 5:
                        uploadFile();
                        break;
                    case 6:
                        viewMessages();
                        break;
                    case 7:
                        disconnect();
                        break;
                }
            }

        } catch (Exception e) {
            System.out.println(e);
        } finally {
            try {
                networkUtil.closeConnection();
            } catch (IOException e) {
//                System.out.println("Here");

                e.printStackTrace();
            }
        }
    }

    private void lookupClients() throws IOException, ClassNotFoundException {

        Object object = networkUtil.read();

        if(object instanceof List<?>) {
            List<?> connectedClients = (List<?>) object;

            System.out.println("Online Clients: ");

            for(Object o: connectedClients) {
                if(o.equals(name)) {
                    System.out.println(o + "(Own)");
                    continue;
                }

                System.out.println(o);
            }
        }

        object = networkUtil.read();

        if(object instanceof List<?>) {
            List<?> connectedClients = (List<?>) object;

            System.out.println("Offline Clients: ");

            for(Object o: connectedClients) {
                System.out.println(o);
            }
        }
    }

    private void lookupOwnFiles() throws IOException, ClassNotFoundException {

        int fileIdx = 1;
        Object object = networkUtil.read();

        Scanner scanner = new Scanner(System.in);

        if(object instanceof List<?>) {
            List<String> publicFiles = (List<String>) object;

            System.out.println("Your public files: ");

            for(String file: publicFiles) {
                System.out.println(fileIdx++ + ". " + file);
            }
        }

        object = networkUtil.read();

        if(object instanceof List<?>) {
            List<String> privateFiles = (List<String>) object;

            System.out.println("Your private files: ");

            for(String file: privateFiles) {
                System.out.println(fileIdx++ + ". " + file);
            }
        }

        System.out.println("\n" +
                "To download one of your file enter the index number else enter -1: ");

        fileIdx = scanner.nextInt();
        scanner.nextLine();

        networkUtil.write(fileIdx);

        if(fileIdx != -1) downloadFile();


    }
    private void lookupOtherFiles() throws IOException, ClassNotFoundException {

        Scanner scanner = new Scanner(System.in);

        List<Pair> publicFiles = (List<Pair>) networkUtil.read();

        System.out.println("Public files of other clients: \n" +
                "File Name\t\t - \t\t Client Name");

        for (Pair pair : publicFiles) {
            String file = pair.getKey();
            String client = pair.getValue();

            System.out.println(file + "\t\t - \t\t " + client);
        }

        System.out.println();

        System.out.println("Enter 1 if you want to download any of this, else enter 0");

        int choice = scanner.nextInt();
        scanner.nextLine();

        networkUtil.write(choice);

        if(choice == 1) {
            System.out.print("Enter the file name: ");
            String fileName = scanner.nextLine();

            System.out.print("Enter client name: ");
            String clientName = scanner.nextLine();

            networkUtil.write(fileName);
            networkUtil.write(clientName);

            String clientStatus = (String) networkUtil.read();

            if(!clientStatus.equals("clientFound")) {
                System.out.println("No such client exists!");
                return;
            }

            String fileStatus = (String) networkUtil.read();

            if(!fileStatus.equals("fileFound")) {
                System.out.println("No such public file exists in client: " + clientName + "!");
                return;
            }

            downloadFile();

        }

    }

    private void downloadFile() throws IOException, ClassNotFoundException {

        String response = (String) networkUtil.read();

        if (response.equals("FileNotFound")) {
            System.out.println("File not found on the server.");
            return;
        }

        String fileName = (String)networkUtil.read();
        int fileSize = (int)networkUtil.read();

        String savePath = "/home/avi47/DiskC/CSE academic code/3-2/CSE322-Computer-Networks-Sessional/Practice/Downloads";

        File file = new File(savePath + "/" + fileName);
        FileOutputStream fileOutputStream = new FileOutputStream(file);

        int chunkSize = (int) networkUtil.read();
        byte[] buffer = new byte[chunkSize];
        int bytesRead;
        int chunkCount = 1;

        while ((bytesRead = networkUtil.read(buffer, 0, buffer.length)) != -1) {

            fileOutputStream.write(buffer, 0, bytesRead);

            double percentage = (chunkCount*chunkSize*100.0/fileSize);

            if(percentage > 100) percentage = 100;

            System.out.println("Download progress -> " + String.format("%.2f", percentage) + " % ...");
            chunkCount++;
        }

        fileOutputStream.close();

        // Receive completion message from the server
        response = (String) networkUtil.read();
        if (response.equals("Complete")) {
            System.out.println("File downloaded successfully: " + fileName);
        } else {
            System.out.println("Error occurred during file download.");
        }
    }

    private void makeFileRequest() throws IOException, ClassNotFoundException {

        Scanner scanner = new Scanner(System.in);

        int reqId =(Integer) networkUtil.read();

        System.out.println("Req id: " + reqId);
        System.out.println("Enter a short description of your file: ");

        String shortDes = scanner.nextLine();

        networkUtil.write(shortDes);

    }

    private void uploadFile() {
        try {
            Scanner input = new Scanner(System.in);

            System.out.println("Select what you want to upload: \n" +
                    "1. Only upload your file\n" +
                    "2. Respond to a upload request");


            if(input.nextInt() == 1) {
                networkUtil.write("Upload");

                System.out.println("How do you want to upload your file ? \n" +
                        "1. Private \t\t 2. Public");

                if(input.nextInt() == 1) networkUtil.write("Private");
                else networkUtil.write("Public");

            } else  {
                networkUtil.write("Response");
                networkUtil.write("Public");

                System.out.println("Enter request id(Integer):");

                int reqID = input.nextInt();
                networkUtil.write(reqID);

                String isValid = (String) networkUtil.read();

                if(!isValid.equals("Valid")) {
                    System.out.println("Invalid Request id");
                    return;
                }
            }

            input.nextLine();

            System.out.print("Enter the path of the file to upload: ");

            String filePath = input.nextLine();



            File file = new File(filePath);
            if (!file.exists()) {
                networkUtil.write("FileNotFound");
                System.out.println("File not found!");
                return;
            }

            networkUtil.write("FileFound");

            String fileName = file.getName();
            int fileSize = (int) file.length();

            // Send the file name and file size to the server
            networkUtil.write(fileName);
            networkUtil.write(fileSize);

            String confirmation = (String) networkUtil.read();

            if (!confirmation.equals("Allowed")) {
                System.out.println("Server rejected the file upload!");
                return;
            }

            FileInputStream fileInputStream = new FileInputStream(file);

            int chunkSize = (int) networkUtil.read();
            String fileId = (String) networkUtil.read();

            System.out.println("Received chunkSize: " + chunkSize + " and fileId: " + fileId);

            System.out.println("Transmission starting.......");

            byte[] buffer = new byte[chunkSize];
            int bytesRead;
            int chunkCount = 1;
            boolean timeout = false;

            while ((bytesRead = fileInputStream.read(buffer)) != -1) {

                long startTime = System.currentTimeMillis();

                networkUtil.write(buffer, 0, bytesRead);

                // Wait for acknowledgment from the server
                String acknowledgment = (String) networkUtil.read();

                if (!acknowledgment.equals("Received")) {
                    System.out.println("Error occurred during file upload. Aborting...");
                    break;
                }

                long currentTime = System.currentTimeMillis();
                long elapsedTime = currentTime - startTime;
                if (elapsedTime >= 30000) {  // 30 seconds timeout
                    System.out.println("Timeout occurred. Aborting transmission...");
                    timeout = true;
                    break;
                }


                double percentage = (chunkCount*chunkSize*100.0/fileSize);

                if(percentage > 100) percentage = 100;

                System.out.println("Transmission progress -> " + String.format("%.2f", percentage) + " % ...");
                chunkCount++;

                // Check for timeout



            }

            fileInputStream.close();

            if(timeout) {
                networkUtil.write("Timeout");
                return;
            }

            // Send completion message to the server
            networkUtil.write("Complete");

            String uploadStatus = (String) networkUtil.read();
            if (uploadStatus.equals("Success")) {
                System.out.println("File upload completed successfully!");
            } else {
                System.out.println("File upload failed!");
            }
        } catch (IOException | ClassNotFoundException e) {
            System.out.println("Error occurred during file upload: " + e.getMessage());
        }
    }



    private void viewMessages() throws IOException, ClassNotFoundException {

        Object object = networkUtil.read();

        if(object instanceof List<?>) {
            List<?> messages = (List<?>) object;

            if(messages.size() == 0) {
                System.out.println("No unread messages");
                return;
            }

            System.out.println("Unread Messages : ");

            int s = 1;

            for(Object o: messages) {
                System.out.println();
                System.out.print( "Message[" + (s++) + "]: ");
                System.out.println(o);
            }
        }
    }


    private void disconnect() throws IOException, ClassNotFoundException {

        Object object = networkUtil.read();

        if(object instanceof  String) {
            String message = (String) object;
            if(message.equals("Disconnect")) {
                System.exit(0);
            }

        }
    }

}



