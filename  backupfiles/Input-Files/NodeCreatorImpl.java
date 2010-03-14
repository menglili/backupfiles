import java.rmi.Naming;
import java.rmi.RemoteException;
import java.rmi.server.RMIClientSocketFactory;
import java.rmi.server.RMIServerSocketFactory;
import java.rmi.server.UnicastRemoteObject;
import java.util.Random;


public class NodeCreatorImpl extends UnicastRemoteObject implements NodeCreator{
	
	public NodeCreatorImpl() throws RemoteException {
		super();
	}

	public Node createNode() throws RemoteException {
		return new NodeImplRemote();
	}
}
