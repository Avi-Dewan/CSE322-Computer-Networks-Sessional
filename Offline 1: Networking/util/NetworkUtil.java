package util;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class NetworkUtil {
    private Socket socket;
    private ObjectInputStream ois;
    private ObjectOutputStream oos;



    public NetworkUtil(String s, int port) throws IOException {
        this.socket = new Socket(s, port);
        oos = new ObjectOutputStream(socket.getOutputStream());
        ois = new ObjectInputStream(socket.getInputStream());
    }

    public NetworkUtil(Socket s) throws IOException {
        this.socket = s;
        oos = new ObjectOutputStream(socket.getOutputStream());
        ois = new ObjectInputStream(socket.getInputStream());
    }

    public Object read() throws IOException, ClassNotFoundException {
        return ois.readUnshared();
    }

    public void write(Object o) throws IOException {
        oos.writeUnshared(o);
    }

    public void write(byte[] buffer, int offset, int length) throws IOException {
        oos.write(buffer, offset, length);
        oos.flush();
    }

    public int read(byte[] buffer, int offset, int length) throws IOException {
        return ois.read(buffer, offset, length);
    }

    public void closeConnection() throws IOException{
        ois.close();
        oos.close();
    }
}
