import java.io.Serializable;
import java.rmi.RemoteException;
import java.rmi.server.RMIClientSocketFactory;
import java.rmi.server.RMIServerSocketFactory;
import java.rmi.server.UnicastRemoteObject;
import java.util.HashSet;
import java.util.Set;


public class NodeImplRemote extends UnicastRemoteObject implements Node, Serializable{

	public NodeImplRemote() throws RemoteException {
		super();
	}
	private Set<Node> sNodes = new HashSet<Node>();
	
	public Set<Node> getNeighbors ()
	{
		return (sNodes);
	}
	
	public Set<NodeTuple> getTransitiveNeighbors (int distance) throws RemoteException
	{
		if (distance <= 0)
			throw new IllegalArgumentException ("Argument distance must be positive");
		
		Set<NodeTuple> tNodes = new HashSet<NodeTuple> ();
		
		// Initial zero-distance node
		tNodes.add(new NodeTuple(0, this));
		
		// Closure for each level of i-distant nodes
		for (int i = 0; i < distance; i++)
		{
			Set<Node> nodes = new HashSet<Node>();
			
			for (NodeTuple tuple : tNodes)
			{
				// Use nodes which are in the current level
				if (tuple.distance == i)
					nodes.addAll(tuple.node.getNeighbors());
			}
			
			for (Node node : nodes)
			{
				tNodes.add(new NodeTuple(i + 1, node));
			}
		}
		
		return tNodes;
	}
	
	public void addNeighbor (Node oNeighbor)
	{
		sNodes.add (oNeighbor);
	}
}
