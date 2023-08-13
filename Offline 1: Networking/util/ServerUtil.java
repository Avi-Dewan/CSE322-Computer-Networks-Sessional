package util;



import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Random;

public class ServerUtil {
    public int MAX_BUFFER_SIZE;
    public int MIN_CHUNK_SIZE;
    public int MAX_CHUNK_SIZE;
    public int buffer_size;
    public HashMap<String, NetworkUtil> onlineClients;
    public ArrayList<String> offlineClients;
    public HashMap<String, ClientUtil> clientsInfo;
    private int reqId;
    private HashMap<Integer, RequestUtil> requests;

    public ServerUtil(int max_buff, int min, int max) {
        onlineClients = new HashMap<>();
        offlineClients = new ArrayList<>();
        clientsInfo = new HashMap<>();
        requests = new HashMap<>();
        this.MIN_CHUNK_SIZE = min;
        this.MAX_CHUNK_SIZE = max;
        this.MAX_BUFFER_SIZE = max_buff;
        this.buffer_size = 0;
        this.reqId = 1;
    }

    public int getRandomChunkSize() {
        Random random = new Random();
        return random.nextInt(MAX_CHUNK_SIZE - MIN_CHUNK_SIZE + 1) + MIN_CHUNK_SIZE;
    }

    public int getMAX_BUFFER_SIZE() {
        return MAX_BUFFER_SIZE;
    }

    public int getBuffer_size() {
        return buffer_size;
    }

    public int getMAX_CHUNK_SIZE() {
        return MAX_CHUNK_SIZE;
    }

    public void addToBuffer(int newBufferSize) {
        this.buffer_size += newBufferSize;
    }

    public HashMap<String, NetworkUtil> getOnlineClients() {
        return onlineClients;
    }

    public HashMap<String, ClientUtil> getClientsInfo() {
        return clientsInfo;
    }

    public ArrayList<String> getOfflineClients() {
        return offlineClients;
    }

    public void addOnlineClient(String clientName, NetworkUtil networkUtil) {
        onlineClients.put(clientName, networkUtil);
    }

    public void removeOnlineClient(String clientName) {
        onlineClients.remove(clientName);
    }

    public void addOfflineClient(String clientName) {
        offlineClients.add(clientName);
    }

    public void removeOfflineClient(String clientName) {
        offlineClients.remove(clientName);
    }

    public void addClientInfo(String clientName, ClientUtil clientUtil) {
        clientsInfo.put(clientName, clientUtil);
    }

    public ClientUtil getClientInfo(String clientName) {
        return clientsInfo.get(clientName);
    }

    public int getReqId() {
        return reqId++;
    }

    public HashMap<Integer, RequestUtil> getRequests() {
        return requests;
    }

    public void addRequest(Integer id, RequestUtil requestUtil) {
        requests.put(id , requestUtil);
    }

    public void sendReqResponse(Integer rId, String fromClient) {
        requests.get(rId).getClient().addMessage(
                "client - '" + fromClient +
                "' uploaded your requested file( request_id: " + rId + ")");
    }

    public void broadcastNewRequest(String clientName, int reqId, String shortDes) {

        String message = "REQUEST(id=" + reqId +  ") from client-'" + clientName +
        "'\nDescription: " + shortDes ;

        for(ClientUtil clientUtil: clientsInfo.values()) {
            if(!clientUtil.getName().equals(clientName))
                clientUtil.addMessage(message);
        }

    }

    public List<Pair> getOthersPublicFiles(String clientName) {
        List<Pair> pairList = new ArrayList<>();

        for(ClientUtil clientUtil: clientsInfo.values()) {
            if(!clientUtil.getName().equals(clientName)) {
                pairList.addAll(clientUtil.getPublicFileNames());
            }
        }

        return pairList;
    }
}

