import java.util.Random;
import java.net.MalformedURLException;
import java.rmi.*;

public class Main
{
	// How many nodes and how many edges to create.
	private static final int GRAPH_NODES = 1000;
	private static final int GRAPH_EDGES_LOW = 500;
	private static final int GRAPH_EDGES_MEDIUM = 2000;
	private static final int GRAPH_EDGES_HIGH = 10000;
	
	// How many searches to perform
	private static final int SEARCHES = 50;
	
	private static Node [] aNodes;
	private static Node[] remoteNodes;
	
	private static Random oRandom = new Random ();	
	
	/**
	 * Creates nodes of a graph.
	 * @param iHowMany
	 */
	public static void createNodes (int iHowMany)
	{
		aNodes = new Node [iHowMany];
		
		for (int i = 0 ; i < iHowMany ; i ++)
		{
			aNodes [i] = new NodeImpl ();
		}
	}
	
	public static void createRemoteNodes(int iHowMany)
	{
		NodeCreator nodeCreator = null;
		try {
			nodeCreator = (NodeCreator)Naming.lookup("//localhost/NodeCreator");
			remoteNodes = new Node [iHowMany];
			
			for (int i = 0 ; i < iHowMany ; i ++)
			{
				remoteNodes [i] = nodeCreator.createNode();
			}
		} catch (MalformedURLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (RemoteException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NotBoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}	
	/**
	 * Creates a fully connected graph. 
	 * @throws RemoteException 
	 */
	public void connectAllNodes () throws RemoteException
	{
		for (int iNodeFrom = 0 ; iNodeFrom < aNodes.length ; iNodeFrom++)
		{
			for (int iNodeTo = iNodeFrom + 1 ; iNodeTo < aNodes.length ; iNodeTo++)
			{
				aNodes [iNodeFrom].addNeighbor (aNodes [iNodeTo]);
				aNodes [iNodeTo].addNeighbor (aNodes [iNodeFrom]);
			}
		}
	}
	
	/**
	 * Creates a randomly connected graph.
	 * @param iHowMany
	 * @throws RemoteException 
	 */
	public static void connectSomeNodes (int iHowMany) throws RemoteException
	{
		for (int i = 0 ; i < iHowMany ; i ++)
		{
			int iNodeFrom = oRandom.nextInt (aNodes.length);
			int iNodeTo = oRandom.nextInt (aNodes.length);
			
			aNodes [iNodeFrom].addNeighbor (aNodes [iNodeTo]);
		}
	}
	
	public static void connectSomeRemoteNodes(int iHowMany)throws RemoteException
	{
		for (int i = 0 ; i < iHowMany ; i ++)
		{
			int iNodeFrom = oRandom.nextInt (remoteNodes.length);
			int iNodeTo = oRandom.nextInt (remoteNodes.length);
			
			remoteNodes [iNodeFrom].addNeighbor (remoteNodes [iNodeTo]);
		}	
	}	
	
	/**
	 * Runs a quick measurement on the graph.
	 * @param iHowMany
	 */
	public static void searchBenchmark (int iHowMany, Searcher oSearcher, Node[] aNodes)
	{	
		// Display measurement header.
		System.out.printf ("%7s %8s %13s %13s\n", "Attempt", "Distance", "Time", "TTime");
		for (int i = 0 ; i < iHowMany ; i ++)
		{
			// Select two random nodes.
			int iNodeFrom = oRandom.nextInt (aNodes.length);
			int iNodeTo = oRandom.nextInt (aNodes.length);
			
			// Calculate distance, timing the operation.
			long iTime = System.nanoTime ();
			int iDistance = 0;
			
			try {
				iDistance = oSearcher.getDistance (aNodes [iNodeFrom], aNodes [iNodeTo]);
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			iTime = System.nanoTime () - iTime;
			
			long iTTime = System.nanoTime ();
			int iTDistance = 0;
			try
			{
				iTDistance= oSearcher.getTransitiveDistance (4, aNodes [iNodeFrom], aNodes [iNodeTo]);
			}catch(RemoteException ex)
			{
				ex.printStackTrace();
			}
			
			iTTime = System.nanoTime () - iTTime;
			
			if (iDistance != iTDistance)
			{
				System.out.printf("Standard and transitive algorithms inconsistent (%d != %d)\n", iDistance, iTDistance);
			} else {
				// Print the measurement result.
				System.out.printf ("%7d %8d %13d %13d\n", i, iDistance, iTime / 1000, iTTime / 1000);
			}
		}
	}
	
	public static void main (String[] args) throws RemoteException
	{
		// Create and install a security manager if necessary.
		if (System.getSecurityManager () == null)
			System.setSecurityManager (new RMISecurityManager ());
		
		Searcher localSearcher = null;
		Searcher remoteSearcher = null;
		Node[] localGraph = null;
		Node[] remoteGraph = null;
		
		try
		{
			remoteSearcher = (Searcher) Naming.lookup ("//localhost/Searcher");
			localSearcher = new SearcherImpl();
			
			createNodes (GRAPH_NODES);
			createRemoteNodes(GRAPH_NODES);
			localGraph = aNodes;
			remoteGraph = remoteNodes;
			
			connectSomeNodes (GRAPH_EDGES_MEDIUM);		
			connectSomeRemoteNodes (GRAPH_EDGES_MEDIUM);								
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		System.out.println("---Sparse graph---");
		System.out.println("==============Local nodes, local searcher=====================");
		searchBenchmark (SEARCHES, localSearcher, localGraph);
		System.out.println("==============Local nodes, remote searcher=====================");
		searchBenchmark (SEARCHES, remoteSearcher, localGraph);
		System.out.println("==============remote nodes, local searcher=====================");
		searchBenchmark (SEARCHES, localSearcher, remoteGraph);
		System.out.println("==============remote nodes, remote searcher=====================");
		searchBenchmark (SEARCHES, remoteSearcher, remoteGraph);
		
		/*System.out.println("Normal graph");
		createNodes (GRAPH_NODES);
		connectSomeNodes (GRAPH_EDGES_MEDIUM);
		searchBenchmark (SEARCHES, oSearcher);
		
		System.out.println("Dense graph");
		createNodes (GRAPH_NODES);
		connectSomeNodes (GRAPH_EDGES_HIGH);
		searchBenchmark (SEARCHES, oSearcher);*/
		
	}
}
