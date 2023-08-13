package Server;
import java.io.File;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import util.ClientUtil;
import util.NetworkUtil;
import util.ServerUtil;

public class  Server {

    private ServerSocket serverSocket;
    public ServerUtil serverUtil;

    Server(int MAX_BUFFER_SIZE, int MIN_CHUNK_SIZE, int MAX_CHUNK_SIZE) {

        System.out.println("Server started......");

        serverUtil = new ServerUtil(MAX_BUFFER_SIZE, MIN_CHUNK_SIZE, MAX_CHUNK_SIZE);

        File clientFiles = new File("ClientFiles"); // directory where client folders will be uploaded in server
        clientFiles.mkdir();
        File Downloads = new File("Downloads"); // directory where client folders will be uploaded in server
        Downloads.mkdir();

        try {
            serverSocket = new ServerSocket(33333);

            while (true) {
                Socket clientSocket = serverSocket.accept();
                serve(clientSocket);
            }
        } catch (Exception e) {
            System.out.println("Server stopped: " + e);
        }
    }

    public void serve(Socket clientSocket) throws IOException, ClassNotFoundException {

        NetworkUtil networkUtil = new NetworkUtil(clientSocket);
        String clientName = (String) networkUtil.read(); // reading the name of the client

        if(serverUtil.getOnlineClients().containsKey(clientName)) {
            networkUtil.write("Denied"); // Already logged in
            networkUtil.closeConnection();
            return;
        }

        networkUtil.write("Accepted");


        if(serverUtil.getOfflineClients().contains(clientName)) {
            serverUtil.removeOfflineClient(clientName); // Already folder, clientUtil for the client exists but new networkUtil is needed
            serverUtil.getClientsInfo().get(clientName).setOnline(true);
        } else  {
            File clientFile = new File("ClientFiles/" + clientName);
            clientFile.mkdir();
            serverUtil.addClientInfo(clientName, new ClientUtil(clientName));
        }



        serverUtil.addOnlineClient(clientName, networkUtil); // setting the networkUtil object(through which client is connected with server) against client name

        new ThreadServer(serverUtil, clientName); // for each client, a read thread is opened.

    }

    public static void main(String args[]) {

        int MAX_CHUNK_SIZE = 1000;
        int MIN_CHUNK_SIZE = 100;
        int MAX_BUFFER_SIZE = 1000000;

        Server server = new Server(MAX_BUFFER_SIZE, MIN_CHUNK_SIZE, MAX_CHUNK_SIZE);
    }
}
