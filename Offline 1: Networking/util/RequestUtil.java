package util;

public class RequestUtil {
    private int reqId;
    private String description;
    private ClientUtil client;

    public RequestUtil(int reqId, String description, ClientUtil client) {
        this.reqId = reqId;
        this.description = description;
        this.client = client;
    }

    public int getReqId() {
        return reqId;
    }

    public String getDescription() {
        return description;
    }

    public ClientUtil getClient() {
        return client;
    }
}
