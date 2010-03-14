import java.rmi.Remote;
import java.rmi.RemoteException;


public interface NodeCreator extends Remote {
	public Node createNode()throws RemoteException;
}
