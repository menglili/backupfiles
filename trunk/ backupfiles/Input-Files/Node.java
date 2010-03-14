import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Set;
import java.util.List;

public interface Node extends Remote
{
	Set<Node> getNeighbors ()throws RemoteException;
	Set<NodeTuple> getTransitiveNeighbors (int distance)throws RemoteException;
	void addNeighbor (Node oNeighbor)throws RemoteException;
}
