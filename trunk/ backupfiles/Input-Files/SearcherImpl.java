import java.rmi.RemoteException;
import java.util.HashSet;
import java.util.Set;

public class SearcherImpl extends java.rmi.server.UnicastRemoteObject implements Searcher
{
	public SearcherImpl() throws RemoteException {
		super();
	}

	public int getDistance (Node oFrom, Node oTo) throws RemoteException
	{
		// Implements a trivial distance measurement algorithm.
		// Starting from the source node, a set of visited nodes
		// is always extended by immediate neighbors of all visited
		// nodes, until the target node is visited or no node is left.
		
		// mVisited keeps the nodes visited in past steps.
		// mBoundary keeps the nodes visited in current step.
		Set<Node> mVisited = new HashSet<Node> ();
		Set<Node> mBoundary = new HashSet<Node> ();
		
		int iDistance = 0;
		
		// We start from the source node.
		mBoundary.add (oFrom);
		
		// Traverse the graph until finding the target node.
		while (!mBoundary.contains (oTo))
		{
			// Not having anything to visit means the target node cannot be reached.
			if (mBoundary.isEmpty ())
				return (Searcher.DISTANCE_INFINITE);
			
			Set<Node> mTraversing = new HashSet<Node> ();
			
			// Collect a set of immediate neighbors of nodes visited in current step.
			for (Node oNode : mBoundary)
			{
				mTraversing.addAll (oNode.getNeighbors ());
			}
			
			// Nodes visited in current step become nodes visited in past steps.
			mVisited.addAll (mBoundary);
			// Out of immediate neighbors, consider only those not yet visited.
			mTraversing.removeAll (mVisited);
			// Make these nodes the new nodes to be visited in current step.
			mBoundary = mTraversing;
			
			iDistance ++;
		}
		
		return (iDistance);
	}
	
	public int getTransitiveDistance (int distance, Node oFrom, Node oTo) throws RemoteException
	{
		// Implements a transitive distance measurement algorithm.
		// Starting from the source node, a set of visited nodes
		// is always extended by transitive neighbors of all visited
		// nodes, until the target node is visited or no node is left.
		
		// mVisited keeps the nodes visited in past steps.
		// mBoundary keeps the nodes visited in current step.
		Set<Node> mVisited = new HashSet<Node> ();
		Set<NodeTuple> mBoundary = new HashSet<NodeTuple> ();
		
		// We start from the source node.
		mBoundary.add (new NodeTuple(0, oFrom));
		
		// Traverse the graph until finding the target node.
		while (true)
		{
			// Not having anything to visit means the target node cannot be reached.
			if (mBoundary.isEmpty ())
				return (Searcher.DISTANCE_INFINITE);
			
			Set<NodeTuple> mTraversing = new HashSet<NodeTuple> ();
			
			// Collect a set of transitive neighbors of nodes visited in current step.
			for (NodeTuple oTuple : mBoundary)
			{
				Set<NodeTuple> mPartial = oTuple.node.getTransitiveNeighbors (distance);
				
				for (NodeTuple partial : mPartial)
				{
					if (mVisited.contains (oTuple.node))
						continue;
					
					// Check whether the node is already traversed
					int iDistance = oTuple.distance + partial.distance;
					NodeTuple existing = null;
					
					for (NodeTuple traversing : mTraversing)
					{
						if (traversing.node == partial.node)
						{
							existing = traversing;
							break;
						}
					}
					
					if (existing != null)
					{
						if (existing.distance < iDistance)
							continue;
						
						existing.distance = iDistance;
						continue;
					}
					
					mTraversing.add(new NodeTuple(iDistance, partial.node));
				}
				
				// Nodes visited in current step become nodes visited in past steps.
				mVisited.add (oTuple.node);
			}
			
			for (NodeTuple oTuple : mTraversing)
			{
				if (oTuple.node == oTo)
					return oTuple.distance;
			}
			
			mBoundary = mTraversing;
		}
	}
}
