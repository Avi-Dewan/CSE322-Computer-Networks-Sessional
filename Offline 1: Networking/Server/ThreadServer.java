package Server;

import util.*;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.time.Instant;

public class ThreadServer implements Runnable {
    private Thread thr;
    private NetworkUtil networkUtil;
    private ClientUtil clientUtil;
    private String clientName;
    public ServerUtil serverUtil;



    public ThreadServer(ServerUtil serverUtil, String clientName) throws IOException, ClassNotFoundException {
        this.serverUtil = serverUtil;
        this.clientName = clientName;

        this.networkUtil = serverUtil.getOnlineClients().get(clientName);
        this.clientUtil = serverUtil.getClientInfo(clientName);

        this.thr = new Thread(this);
        thr.start();
    }

    
    public void run() {
        try {

            while (true) {
                Object object = networkUtil.read();

                if(object instanceof  Integer) {

                    Integer choice = (Integer) object;

                    switch (choice) {
                        case 1:
                            sendListOfClients();
                            break;
                        case 2:
                            sendClientFileList();
                            break;
                        case 3:
                            sendPublicFilesList();
                            break;
                        case 4:
                            receiveFileRequest();
                            break;
                        case 5:
                            uploadFile();
                            break;
                        case 6:
                            sendMessages();
                            break;
                        case 7:
                            networkUtil.write("Disconnect");
                            break;
                    }

                }

            }
        } catch (IOException io) {

            //SocketException is a subclass of IOException

            System.out.println("Client disconnected: " + clientName);
            
            disconnect(clientName);

        } catch (Exception e) {
            System.out.println(e);
        } finally {
            try {
                networkUtil.closeConnection();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


    private void sendListOfClients() throws IOException {
        List<String> onlineClients = new ArrayList<>(serverUtil.getOnlineClients().keySet());
        networkUtil.write(onlineClients);
        List<String> offlineClients = serverUtil.getOfflineClients();
        networkUtil.write(offlineClients);
    }

    private void sendClientFileList() throws IOException, ClassNotFoundException {

        HashMap<String, Boolean> files = clientUtil.getFileNames();

        List<String> publicFiles = new ArrayList<>();
        List<String> privateFiles= new ArrayList<>();

        for (Map.Entry<String, Boolean> entry : files.entrySet()) {
            String key = entry.getKey();
            boolean value = entry.getValue();

            if (value) {
                publicFiles.add(key);
            } else {
                privateFiles.add(key);
            }
        }

        networkUtil.write(publicFiles);
        networkUtil.write(privateFiles);

        int sendId = (Integer)networkUtil.read();

        if(sendId == -1) return;

        sendId--;

        if(sendId > (publicFiles.size()-1)) {
            sendId = sendId - publicFiles.size();
            sendFile(clientName, privateFiles.get(sendId));
        } else {
            sendFile(clientName, publicFiles.get(sendId));
        }
    }

    private void sendPublicFilesList() throws IOException, ClassNotFoundException {

        List<Pair> publicFiles = serverUtil.getOthersPublicFiles(clientName);

        networkUtil.write(publicFiles);

        int choice = (int)networkUtil.read();

        if(choice == 0) return;

        String downloadFileName = (String) networkUtil.read();
        String downloadClientName = (String) networkUtil.read();



        if(!serverUtil.getClientsInfo().containsKey(downloadClientName)) {
            networkUtil.write("clientNotFound");
            return;
        }

        networkUtil.write("clientFound");

        ClientUtil downloadClientUtil = serverUtil.getClientsInfo().get(downloadClientName);

        if(!downloadClientUtil.getFileNames().containsKey(downloadFileName) || !downloadClientUtil.getFileNames().get(downloadFileName) ) {
            networkUtil.write("fileNotFound");
            return;
        }

        networkUtil.write("fileFound");

        sendFile(downloadClientName, downloadFileName);

    }

    private void sendFile(String clientName, String fileName) throws IOException {

        File file = new File("ClientFiles/" + clientName + "/" + fileName);

        if (!file.exists()) {
            networkUtil.write("FileNotFound");
            return;
        }

        networkUtil.write("FileFound");

        FileInputStream fileInputStream = new FileInputStream(file);

        int fileSize = (int) file.length();
        int chunkSize = serverUtil.getMAX_CHUNK_SIZE();

        // Send the file name, file size and chunk size to the client
        networkUtil.write(fileName);
        networkUtil.write(fileSize);
        networkUtil.write(chunkSize);

        byte[] buffer = new byte[chunkSize];
        int bytesRead;

        while ((bytesRead = fileInputStream.read(buffer)) != -1) {
            networkUtil.write(buffer, 0, bytesRead);
        }

        fileInputStream.close();

        // Send completion message to the client
        networkUtil.write("Complete");

        System.out.println("File sent successfully: " + fileName);
    }

    private void receiveFileRequest() throws IOException, ClassNotFoundException {

        int reqId = serverUtil.getReqId();

        networkUtil.write(reqId);

        String shortDes = (String)networkUtil.read();

        serverUtil.addRequest(reqId, new RequestUtil(reqId, shortDes, clientUtil));

        System.out.println("Server received request (id=" + reqId + ") from " + clientName);

        serverUtil.broadcastNewRequest(clientName, reqId, shortDes);
    }

    private void uploadFile() throws InterruptedException {
        try {

            String uploadWay = (String)networkUtil.read();
            String uploadType = (String)networkUtil.read();

            int reqID = -1;
            boolean isPublic = false;

            if(uploadType.equals("Public")) isPublic = true;


            if(uploadWay.equals("Response")) {
                reqID = (Integer)networkUtil.read();

                boolean validReq = serverUtil.getRequests().containsKey(reqID);

                if(!validReq) {
                   networkUtil.write("Invalid");
                   return;
                }

                networkUtil.write("Valid");

            }

            String fileFound = (String)networkUtil.read();

            if(!fileFound.equals("FileFound")) return;

            String fileName = (String) networkUtil.read();
            int fileSize = (int) networkUtil.read();

            if (serverUtil.getBuffer_size() + fileSize > serverUtil.getMAX_BUFFER_SIZE()) {
                networkUtil.write("Denied");
                return;
            }

            networkUtil.write("Allowed");


            int chunkSize = serverUtil.getRandomChunkSize();

            // Generate a unique file ID
            long unixTime = Instant.now().getEpochSecond();
            String fileID = clientName + "_" + unixTime;


            networkUtil.write(chunkSize);
            networkUtil.write(fileID);


            // Create a file for the uploaded chunks
            File file = new File("ClientFiles/" + clientName + "/" + fileName);
            FileOutputStream fileOutputStream = new FileOutputStream(file);


            byte[] buffer = new byte[chunkSize];
            int bytesRead;

            while ((bytesRead = networkUtil.read(buffer, 0, buffer.length)) != -1) {
                fileOutputStream.write(buffer, 0, bytesRead);

                // Send acknowledgment to the client
//                Thread.sleep(30001);
                networkUtil.write("Received");
            }

            fileOutputStream.close();

            String uploadStatus = (String) networkUtil.read();

            if(uploadStatus.equals("Timeout")) {
                file.delete(); // Delete the incomplete file
                System.out.println("File upload failed due to timeout: " + fileName);
                return;
            }

            // Verify the uploaded file size
            if (file.length() == fileSize) {
                networkUtil.write("Success");
                clientUtil.addFileName(fileName, isPublic);
                serverUtil.addToBuffer(fileSize);

                if(uploadWay.equals("Response")) {
                    serverUtil.sendReqResponse(reqID, clientName);
                }
                System.out.println("File uploaded successfully: " + fileName);
            } else {
                file.delete(); // Delete the incomplete file
                networkUtil.write("Failure");
                System.out.println("File upload failed: " + fileName);
            }
        } catch (IOException e) {
            System.out.println("Error occurred during file upload: " + e.getMessage());
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }

    }


    private void sendMessages() throws IOException {
        networkUtil.write(clientUtil.getMessages());
        clientUtil.markAllMessagesAsRead();
    }

    private void disconnect(String clientName) {
        serverUtil.removeOnlineClient(clientName);
        serverUtil.addOfflineClient(clientName);
        clientUtil.setOnline(false);
    }

}